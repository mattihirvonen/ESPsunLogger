
//
// ESP32 ADC is enough linear at range 150 mV ... 2500 mV to measure
// small solar panel's current (we are here interrested only 10% accuracy).
// There is about 80 mV offset error in ESP32's ADC measurements.
//
// https://suncalc.org
// https://lucidar.me/en/esp32/linearity-of-the-esp32-adc/
// https://hackaday.io/project/205380-adc-performance-arduino-vs-esp32-vs-ads1115
// https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
// https://randomnerdtutorials.com/esp-idf-esp32-gpio-analog-adc/
// https://github.com/256dpi/arduino-mqtt
//
// Use here floating points (non efficient and only sign+23 bits mantissa)
//
// Select current measurement shunt resistance value:
// - resistance value "Rshunt" is <= (3.0V / Iref)
// - where "Iref" is measured solar panel's short circuit current at max solar intensity

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include "esp32lib.hpp"
#include "pinMap.h"               // LED, BUTTON, AIN0, AIN1, ...
#include "measure.h"              // adcValue_t
#include "mqttClient.h"           // setup_mqtt(), loop_mqtt()


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <LittleFS.h>             // Or FFat.h or/and SD.h
#include <threadSafeFS.h>         // Include thread-safe wrapper since LittleFS, FFat and SD file systems are not thread safe
#include "serversConfig.h"        // Function prototype for setup_telnet()

// Crete thread-safe wrapper arround LittleFS (or FFat or SD)
using  File = threadSafeFS::File; // Use thread-safe wrapper for all file operations form now on in your code
threadSafeFS::FS TSFS (LittleFS);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define UNUSED  __attribute__((unused))

#define WIFI_ACCESSPOINT  0     // Zero: connect to WiFi router
#define MQTT_SERVER       0     // Local MQTT broker (future feature)
#define MQTT_CLIENT       1     // Connect to MQTT broker?

//
// Calibration info:
// - Panel 1:  ESP32 Devkit1, ADC_DIFF=2200, Rshunt=82  (2026-07-28)
//
// 2022-07-29 sun from clear sky 942 W/m2 (99%), Rshunt 82 ohm (2.4 h kohdalla lokissa)
// - time=12:45, Udiff=1785, Upanel=2170 mV, ADC_REF=2200, intensity=81
// - fix ADC_REF: 2200 * 81 / 99 = 1800
//
// - time=13:20, Udiff=1795, Upanel=2175 mV, ADC_REF=2200, intensity=81
// - fix ADC_REF: 2200 * 81 / 99 = 1800

#if defined(ESP32S3)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)
#define ADC_PANEL     AIN0
#define ADC_DIODE     AIN1
#else
#define ADC_PANEL     AIN6    // GPIO pin: Analog ADC1_CH6 - ESP32 DEVKIT V1
#define ADC_DIODE     AIN7    // GPIO pin: Analog ADC1_CH7 - ESP32 DEVKIT V1
#endif

#define ADC_CHANNELS     2    // 2: shunt and diode // 1: only shunt (fix voltage diode)
#define SPmax          950    // Sun's peak power [W/m2] at latitude 60 deg. north (summer time)
#define ADC_REF       1800    // Calibration value: "adcValue.diff" at "SPmax"

//-----------------------------------------------------------------------------------------

// WiFi credentials - Set #if to zero when use local defines in this source
#if 1
#include "WiFiConf.h"   // Hide my secrets here !!!
#else
const char* ssid        = "YOUR_ROUTER_WiFi_SSID";
const char* password    = "YOUR_ROUTER_WiFi_PASSWORD";
const char* ssid_AP     = "ACCESSPOINT_WiFi_SSID";
const char* password_AP = "ACCESSPOINT_WiFi_PASSWORD";
#endif

//-----------------------------------------------------------------------------------------

WiFiClient   wifiClient;


void connect( int wifi_accesspoint )
{
  if ( ! wifi_accesspoint )
  {
    Serial.print("\nChecking   WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED, LED_OFF);
      Serial.print(".");
      delay(1000);
    }
    digitalWrite(LED, LED_ON);
    Serial.print("\nConnected  WiFi");
  }
}

//-----------------------------------------------------------------------------------------

float       Iref   = 0.0270;   // Solar panel's measured "short circuit" current [A] at SPmax
float       Rshunt = 82.0;     // Current shunt resistance [ohm]: Select value <= (2.5V / Iref)
//
int         ADCref = ADC_REF;  // Calibration value: Measured "adcValue.diff" [mV] at SPmax (2100)
int         Ntaps  = 20;       // Filter coefficient
//
adcValue_t  adcValue;          // Work space variable (filtered ADC data)


// Dummy IIR style filtering
int floatingAverage( int32_t *sum, int x, int N )
{
    int avg = *sum / N;

    *sum -= avg;
    *sum += x;

    return *sum / N;
}

/*
// Template function to linearize ADC measurement result
int adcLinearize( int mV )
{
    // Raw: ADC linear range 200 mV ... 2500 mV:
    // - 2586/2.2V - 115/0.2V -> 1235/V
    // - 2961/2.5V - 115/0.2V -> 1238/V
    // - 2961/2.5V - 240/0.3V -> 1237/V

    // Schottky diode voltage drop (abt 300 mV at 1 mA)
    return (mV > 250) ? mV : 0;
}
*/

// Return value: 1.0 per each 100% of sun intensity hour
float cumulative_sum( int32_t sum )
{
    float value = sum;

    return value / (100.0 * 3600.0);
}


void taskMeasure( void UNUSED *pvParameters )
{
    #define TASK_PERIOD 50  // in tick(s) [ms]

    static TickType_t  xLastWakeTime;
    static int32_t     sum_panel = 0, sum_diode = 0, sum_diff = 0;
           int          mV_panel,      mV_diode,      mV_diff;
    //     int          adcRaw;

    if ( ! xLastWakeTime ) {
           xLastWakeTime = xTaskGetTickCount();  // Initializetion: Get current uptime
    }

    while ( 1 ) // Loop for ever
    {
        #define DIODE_mV  265   // BAT85 typical: 250 mV / 0.3 mA - 300 mV / 1 mA

        // Wait for the next cycle.
        BaseType_t UNUSED  xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_PERIOD );

        // ADC result offset and gain fixes required with raw uncalibrated ADC data
    //  adcRaw   = analogRead( ADC_PANEL );
        #if defined(ESP32S3)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)
        mV_panel = 0;
        mV_diode = 0;
        #else // ESP32S3
        mV_panel = analogReadMilliVolts( ADC_PANEL );      // Factory calibrated !!!
        #if  ADC_CHANNELS > 1
        mV_diode = analogReadMilliVolts( ADC_DIODE );      // Factory calibrated !!!
        #else
        mV_diode = DIODE_mV;                               // Single channel ADC measurement
        #endif
        #endif // ESP32S3

        // Filter measurement results
        adcValue.panel = floatingAverage( &sum_panel, mV_panel, Ntaps );
        adcValue.diode = floatingAverage( &sum_diode, mV_diode, Ntaps );
        adcValue.diff  = floatingAverage( &sum_diff,  adcValue.panel - adcValue.diode, Ntaps );
    }
}

//--------------------------------------------------------------------------------------------

void setup( void )
{
    Serial.begin( 115200 );
    delay( 1500 );
    Serial.println("\n\nStart...");

    pinMode(BUTTON, INPUT_PULLUP);  // Enable internal pull-up resistor
    pinMode(LED, OUTPUT);           // Set user LED GPIO pin as output
    digitalWrite(LED, LED_OFF);

    // Start LittleFS (or FFat or SD)
    LittleFS.begin (true);

    // Start WiFi connection
    #if   WIFI_ACCESSPOINT
    setup_wifi_AP( ssid_AP, password_AP );
    #else
    // Connect to Wi-Fi router
    setup_wifi( ssid, password );
    digitalWrite(LED, LED_ON);
    #endif // WIFI_ACCESSPOINT

    setup_telnetServer();
    setup_ntpClient( WIFI_ACCESSPOINT );
    setup_ftpServer();
    setup_mqtt( WIFI_ACCESSPOINT, MQTT_CLIENT );

    #if 1
    // There is broblem with public servers like broker.hivemq.com
    // Testing MQTT with/without strict timing task's CPU load
    xTaskCreate(
      taskMeasure,    // function name
      "Measure",      // task name (for debugging)
      1024,           // stack size in words (not bytes)
      NULL,           // parameters to pass
      2,              // priority (1 = lowest)
      NULL            // task handle (optional)
    );
    #endif

    pinMode(ADC_DIODE, INPUT);
    pinMode(ADC_PANEL, INPUT);
}


void loop( void )
{
    int32_t    now = millis();

    blink_led( now, WIFI_ACCESSPOINT );
    loop_mqtt( now, WIFI_ACCESSPOINT, MQTT_CLIENT, ADC_PANEL );
}
