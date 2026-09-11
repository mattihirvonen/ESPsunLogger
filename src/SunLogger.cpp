
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

#ifndef UNUSED
#define UNUSED  __attribute__((unused))
#endif

#define WIFI_ACCESSPOINT  0     // Zero: connect to WiFi router
#define MQTT_SERVER       0     // Local MQTT broker (future feature)
#define MQTT_CLIENT       1     // Connect to MQTT broker?

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

WiFiClient  wifiClient;


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

#define TASK_STACK_SIZE  2048   // Words (not bytes), 1024 is not enough with "Wire" library
#define TASK_PRIORITY    2      // Task priority (1 = lowest)

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
      taskMeasure,      // function name
      "Measure",        // task name (for debugging)
      TASK_STACK_SIZE,  // stack size in words (not bytes)
      NULL,             // parameters to pass
      TASK_PRIORITY,    // priority (1 = lowest)
      NULL              // task handle (optional)
    );
    #endif
}


void loop( void )
{
    int32_t    now = millis();

    blink_led( now, WIFI_ACCESSPOINT );
    loop_mqtt( now, WIFI_ACCESSPOINT, MQTT_CLIENT );
}
