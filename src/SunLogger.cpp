
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
#include <PubSubClient.h>         // MQTT
#include "esp32lib.hpp"

#define UNUSED  __attribute__((unused))

#define ADC_RSHUNT  32      // GPIO pin: Analog ADC1_CH4 - ESP32 DEVKIT V1
#define ADC_DIODE   34      // GPIO pin: Analog ADC1_CH6 - ESP32 DEVKIT V1
#define SPmax       950     // Sun's peak power [W/m2] at latitude 60 deg. north (summer time)

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
const char*  mqtt_server = "192.168.1.184";  // "broker.hivemq.com";

WiFiClient   espClient;
PubSubClient mqttClient( espClient );

void mqtt_callback(char* topic, byte* message, unsigned int length)
{
  Serial.print("Message received - topic: ");
  Serial.println(topic);
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.println("Message received - data:  " + msg);
}

//-----------------------------------------------------------------------------------------

typedef struct
{
    int     diode;
    int     Rshunt;
    int     diff;
}  adcValue_t;


float       Iref   = 0.0270;   // Solar panel's measured "short circuit" current [A] at SPmax
float       Rshunt = 80.0;     // Current shunt resistance [ohm]: Select value <= (2.5V / Iref)
//
int         ADCref = 2300;     // Measured ADC value at SPmax (2000)
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
    static int32_t     sum_shunt = 0, sum_diode = 0, sum_diff = 0;
           int          mV_shunt,      mV_diode,      mV_diff;
    //     int          adcRaw;

    if ( ! xLastWakeTime ) {
           xLastWakeTime = xTaskGetTickCount();  // Initializetion: Get current uptime
    }

    while ( 1 ) // Loop for ever
    {
        #define DIODE_mV  250   // BAT85 typical: 250 mV / 0.3 mA - 300 mV / 1 mA

        // Wait for the next cycle.
        BaseType_t UNUSED  xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_PERIOD );

        // ADC result offset and gain fixes required with raw data
    //  adcRaw   = analogRead( ADC_RSHUNT );               // Uncalibrated value
        mV_shunt = analogReadMilliVolts( ADC_RSHUNT );     // Factory calibrated !!!
        #if 0
        mV_diode = analogReadMilliVolts( ADC_DIODE  );     // Factory calibrated !!!
        #else
        mV_diode = DIODE_mV;                               // Single channel ADC measurement
        #endif
        // Filter measurement results
        adcValue.Rshunt = floatingAverage( &sum_shunt, mV_shunt, Ntaps );
        adcValue.diode  = floatingAverage( &sum_diode, mV_diode, Ntaps );

        if ( adcValue.Rshunt < adcValue.diode ) {
             adcValue.Rshunt = adcValue.diode;
        }
        adcValue.diff = floatingAverage( &sum_diff,  adcValue.Rshunt - adcValue.diode, Ntaps );
    }
}


void setup( void )
{
    Serial.begin( 115200 );

    // Connect to Wi-Fi router
    setup_wifi( ssid, password );

    // Connect to MQTT broker
    mqttClient.setServer( mqtt_server, 1883 );
    mqttClient.setCallback( mqtt_callback );

    xTaskCreate(
      taskMeasure,    // function name
      "Measure",      // task name (for debugging)
      1024,            // stack size in words (not bytes)
      NULL,           // parameters to pass
      2,              // priority (1 = lowest)
      NULL            // task handle (optional)
  );

  pinMode(ADC_DIODE,  INPUT);
  pinMode(ADC_RSHUNT, INPUT);
}


void loop( void )
{
    #define PERIOD  1000L  // [ms]

    static int      counter  = 0;
    static int32_t  sum      = 0;
    static int32_t  previous = 0;
           int32_t  now      = millis();
           char     line[256];

    if ( (int32_t)(now - previous) < PERIOD ) {
        return;
    }
    previous += PERIOD;
    counter  += 1;        // "seconds"

    int adcData        =  adcValue.diff;                // Filtered ADC [mV] value
    int solarIntensity = (100 * adcData) / ADCref;      // Solar's intensity [%]

    sum += solarIntensity;    // Note: Overflows after few years
    snprintf( line, sizeof(line), "%5d: adc %4d - solar intensity %3d - cumulative %2ld\r\n", counter, adcData, solarIntensity, sum / counter );
    Serial.printf("%s", line);

    if (!mqttClient.connected()) {
        mqtt_reconnect( mqttClient );
        mqttClient.subscribe("solar/#");
    }
    else {
        mqttClient.loop();

        // Publish a message every 1 seconds
        static unsigned long lastMsg = millis();
        if (millis() - lastMsg >= 1000) {
            lastMsg = millis();

            String topic      = "solar/data";
            float  cumulative = cumulative_sum( sum );

            int mV = analogReadMilliVolts( ADC_RSHUNT );  // Debug testing

            // GnuPlot compatible data row:
//          snprintf( line, sizeof(line), "%6d  %4d  %3d  %.3f\n", counter, adcData, solarIntensity, cumulative );
            snprintf( line, sizeof(line), "%6d  %4d  %3d  %.3f  %4d  %4d  %4d\n",
                      counter, adcData, solarIntensity, cumulative, adcValue.Rshunt, adcValue.diode, mV );

            mqttClient.publish( topic.c_str(), line, strlen(line)+1 );

            Serial.print  ("Message published: ");
            Serial.println(line);
        }
    }
}
