
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

#include <stdint.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <WiFi.h>
#include <PubSubClient.h>     // MQTT
#include "esp32lib.hpp"

#define UNUSED  __attribute__((unused))

#define ADC_IOPIN  34      // Analog ADC1_CH6
#define SPmax      950     // Sun's peak power [W/m2] at latitude 60 deg. north (summer time)

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
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.println("Message: " + msg);
}

//-----------------------------------------------------------------------------------------

float   Iref   = 0.0215;   // Solar panel's measured "short circuit" current [A] at SPmax
float   Rshunt = 100.0;    // Current shunt resistance [ohm]: Select value <= (2.5V / Iref)
//
int     ADCref = 2650;     // Measured ADC value at SPmax (and  also at Iref)
int     Ntaps  = 30;       // Filter coefficient

int     adcValue;          // Work space variable (filtered ADC data)


// Dummy IIR style filtering
int floatingAverage( int32_t *sum, int x, int N )
{
    int avg = *sum / N;
    
    *sum -= avg;
    *sum += x;
    
    return *sum / N;
}


void taskMeasure( void UNUSED *pvParameters )
{
    #define TASK_PERIOD  10   // in tick(s) [ms]
    #define OFFSET_FIX   100  // 80 mV equals about 100 ADC units
    
    static TickType_t  xLastWakeTime;
    static int32_t     sum = 0;

    if ( ! xLastWakeTime ) {
           xLastWakeTime = xTaskGetTickCount();  // Initializetion: Get current uptime
    }

    while ( 1 ) // Loop for ever
    {
        int adcResult;
        
        // Wait for the next cycle.
        BaseType_t UNUSED  xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_PERIOD );

        // Note ADC result offset fix
        adcResult = analogRead( ADC_IOPIN ) + OFFSET_FIX;
        adcValue  = floatingAverage( &sum, adcResult, Ntaps );
        
     // Serial.printf("Measure: adcResult=%d - adcValue=%d\n", adcResult, adcValue);
    }
}


void setup( void )
{
    Serial.begin( 115200 );

    // Connect to Wi-Fi router
    setup_wifi( ssid, password );
    
    xTaskCreate(
      taskMeasure,    // function name
      "Measure",      // task name (for debugging)
      1024,            // stack size in words (not bytes)
      NULL,           // parameters to pass
      2,              // priority (1 = lowest)
      NULL            // task handle (optional)
  );
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
    
    int solarIntensity = (100 * adcValue) / ADCref;    // Solar's intensity [%]
    
    sum += solarIntensity;                             // Note: Overflows after few years
    counter++;
    snprintf( line, sizeof(line), "%5d: adc %4d - solar intensity %3d - cumulative %2ld\r\n", counter, adcValue, solarIntensity, sum / counter );

    Serial.printf("%s", line);
}
