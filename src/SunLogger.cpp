
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

// #include <PubSubClient.h>         // MQTT
   #include <MQTT.h>                 // MQTT

#include "esp32lib.hpp"

#define UNUSED  __attribute__((unused))

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

#define ADC_CHANNELS   2      // 2: shunt and diode // 1: only shunt (fix voltage diode)
#define ADC_PANEL     34      // GPIO pin: Analog ADC1_CH6 - ESP32 DEVKIT V1
#define ADC_DIODE     35      // GPIO pin: Analog ADC1_CH7 - ESP32 DEVKIT V1
#define SPmax        950      // Sun's peak power [W/m2] at latitude 60 deg. north (summer time)
#define ADC_REF     1800      // Calibration value: "adcValue.diff" at "SPmax"

#define MQTT_CLIENT_ID   "aurinkopaneeli"
#define MQTT_USERNAME    "public"
#define MQTT_PASSWORD    "public"
#define MQTT_TOPIC       "solar/tikku"         // Select topic to not conflict with public brokers!

// #define MQTT_BROKER  "192.168.1.184"
// #define MQTT_BROKER  "broker.hivemq.com"         // Test topic conflict with wild card using
   #define MQTT_BROKER  "test.mosquitto.org"        // OK, require empty USERNAME and PASSWORD
// #define MQTT_BROKER  "public.cloud.shiftr.io"    // OK, require non empty USERNAME and PASSWORD

//-----------------------------------------------------------------------------------------

// WiFi credentials - Set #if to zero when use local defines in this source
#if 1
#include "WiFiConf.h"   // Hide my secrets here !!!
#else
const char* ssid     = "YOUR_ROUTER_WiFi_SSID";
const char* password = "YOUR_ROUTER_WiFi_PASSWORD";
#endif

//-----------------------------------------------------------------------------------------

// Replace with your MQTT broker details
const char*  mqtt_server = MQTT_BROKER;

WiFiClient   espClient;
MQTTClient   mqttClient;

/*
void mqtt_callback(char* topic, byte* message, unsigned int UNUSED length)
{
  // NOTE:
  // Expect here "message" is printable ASCII text (not binary data) !!!

  Serial.print("Message received - topic: ");
  Serial.println(topic);
  Serial.print("Message received - data:  ");
  Serial.println((char*)message);
}
*/

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}


void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
//while (!mqttClient.connect(MQTT_CLIENT_ID)) {
//while (!mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {   // "public.cloud.shiftr.io"
  while (!mqttClient.connect(MQTT_CLIENT_ID, "", "")) {                         // "test.mosquitto.org"
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  mqttClient.subscribe(MQTT_TOPIC);
//client.unsubscribe(TOPIC);
}

//-----------------------------------------------------------------------------------------

typedef struct
{
    int     panel;
    int     diode;
    int     diff;
}  adcValue_t;


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
        mV_panel = analogReadMilliVolts( ADC_PANEL );      // Factory calibrated !!!
        #if  ADC_CHANNELS > 1
        mV_diode = analogReadMilliVolts( ADC_DIODE );      // Factory calibrated !!!
        #else
        mV_diode = DIODE_mV;                               // Single channel ADC measurement
        #endif
        // Filter measurement results
        adcValue.panel = floatingAverage( &sum_panel, mV_panel, Ntaps );
        adcValue.diode = floatingAverage( &sum_diode, mV_diode, Ntaps );
        adcValue.diff  = floatingAverage( &sum_diff,  adcValue.panel - adcValue.diode, Ntaps );
    }
}


void setup( void )
{
    Serial.begin( 115200 );
    delay( 1500 );
    Serial.println("\n\nStart...");

    // Connect to Wi-Fi router
    setup_wifi( ssid, password );

    // Connect to MQTT broker
	/*
    mqttClient.setServer( mqtt_server, 1883 );
    mqttClient.setCallback( mqtt_callback );
	*/
	// Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
    // by Arduino. You need to set the IP address directly.
 // mqttClient.begin("public.cloud.shiftr.io", espClient);
    mqttClient.begin(MQTT_BROKER, espClient);
    mqttClient.onMessage(messageReceived);

    connect();

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
    #define PERIOD  1000L  // [ms]

    static int      counter  = 0;
    static int32_t  sum      = 0;
    static int32_t  previous = 0;
           int32_t  now      = millis();
           char     line[256];

/*
    if (!mqttClient.connected()) {
        mqtt_reconnect( mqttClient );
//      mqttClient.subscribe( MQTT_TOPIC );
    }
    mqttClient.loop();
*/

    mqttClient.loop();
 // delay(10);  // <- fixes some issues with WiFi stability

    if (!mqttClient.connected()) {
        connect();
    }

    #if 0
    return;    // Test MQTT with minimal CPU load in "loop" function
    #else

    if ( (int32_t)(now - previous) < PERIOD ) {
        return;
    }
    previous += PERIOD;
    counter  += 1;            // "seconds"

    int adcData_diff    = 0;  // Filtered ADC [mV] value of shunt resistor
    int solarIntensity  = 0;  // Solar's intensity [%]

    if ( (adcValue.diff > 0) && (adcValue.panel > 200) ) {
        adcData_diff    = adcValue.diff;
        solarIntensity  = (100 * adcData_diff) / ADCref;
    }
    sum += solarIntensity;    // Overflow after few years

    #if 0  // Debug feature
    snprintf( line, sizeof(line), "%5d: adc %4d - solar intensity %3d - cumulative %2ld\r\n", counter, adcData_diff, solarIntensity, sum / counter );
    Serial.printf("%s", line
    #endif

    // Publish MQTT message every 1 seconds (PERIOD)
    String topic      = MQTT_TOPIC;
    float  cumulative = cumulative_sum( sum );

    int mV = analogReadMilliVolts( ADC_PANEL );  // Debug testing

    // Produce Octave and GnuPlot compatible data row:
//  snprintf( line, sizeof(line), "%6d  %4d  %3d  %.3f\r\n", counter, adcData_diff, solarIntensity, cumulative );
    snprintf( line, sizeof(line), "%3d  %.3f  %6d  %4d  %4d  %4d  %4d\r\n",
              solarIntensity, cumulative, counter, adcData_diff, adcValue.panel, adcValue.diode, adcValue.panel - mV );

//  mqttClient.publish( topic.c_str(), line, strlen(line) + 2 );
    mqttClient.publish( topic.c_str(), line );

    Serial.print("Message published: ");
    Serial.print(line);

    #endif  // Test MQTT with minimal CPU load in "loop" function
}
