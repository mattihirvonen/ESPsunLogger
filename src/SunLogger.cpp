
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
#include <MQTT.h>
#include "esp32lib.hpp"
#include "pinMap.h"               // LED, BUTTON, AIN0, AIN1, ...

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Choose your server's name - this is how Telnet server would introduce itself to the clients
#define  HOSTNAME  "esp32server"

#include <LittleFS.h>             // Or FFat.h or/and SD.h
#include <threadSafeFS.h>         // Include thread-safe wrapper since LittleFS, FFat and SD file systems are not thread safe
#include <ntpClient.h>            // NTP client is needed only for time commands
#include "telnetConfig.h"         // Local config enable/disable telnet server's built in command set
#include <telnetServer.h>
#include <ftpServer.h>

// Crete thread-safe wrapper arround LittleFS (or FFat or SD)
using  File = threadSafeFS::File; // Use thread-safe wrapper for all file operations form now on in your code
threadSafeFS::FS TSFS (LittleFS);

telnetServer_t *telnetServer = NULL;
ftpServer_t    *ftpServer    = NULL;

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


#define ADC_CHANNELS   2      // 2: shunt and diode // 1: only shunt (fix voltage diode)
#define SPmax        950      // Sun's peak power [W/m2] at latitude 60 deg. north (summer time)
#define ADC_REF     1800      // Calibration value: "adcValue.diff" at "SPmax"

#define MQTT_CLIENT_ID  "aurinkopaneeli"
#define MQTT_USERNAME   "public"              // public.cloud.shiftr.io
#define MQTT_PASSWORD   "public"              // public.cloud.shiftr.io
#define MQTT_TOPIC      "solar/tikku"         // Select topic to not conflict with public brokers!
#define MQTT_SUBSCRIBE   0

   #if       WIFI_ACCESSPOINT
   #define   MQTT_BROKER  "127.0.0.1"                 // Localhost
   #else
   #define   MQTT_BROKER  "192.168.1.184"             // OK
// #define   MQTT_BROKER  "test.mosquitto.org"        // OK, require empty USERNAME and PASSWORD
// #define   MQTT_BROKER  "public.cloud.shiftr.io"    // OK, require non empty USERNAME and PASSWORD
// #define   MQTT_BROKER  "broker.hivemq.com"         // Test topic conflict with wild card using
   #endif // WIFI_ACCESSPOINT

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
MQTTClient   mqttClient;


void messageReceived(String &topic, String &payload) {
  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.

  #if 0
  Serial.println("incoming: " + topic + " - " + payload);
  #else
  // Note: Expect "payload" is printable ASCII text (not binary data)
  Serial.print("Message received - topic: ");
  Serial.println(topic.c_str());
  Serial.print("Message received - data:  ");
  Serial.println(payload.c_str());
  #endif
}


void connect( int wifi_accesspoint, int mqtt_client )
{
  if ( ! wifi_accesspoint )
  {
    Serial.print("\nChecking   WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED, LED_OFF);
      Serial.print(".");
      delay(1000);
    }
    Serial.print("\nConnected  WiFi");
  }

  if ( mqtt_client )
  {
    Serial.print("\nConnecting MQTT...");
  //while (!mqttClient.connect(MQTT_CLIENT_ID)) {
  //while (!mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {   // "public.cloud.shiftr.io"
    while (!mqttClient.connect(MQTT_CLIENT_ID, "", "")) {                         // "test.mosquitto.org"
      Serial.print(".");
      delay(1000);
    }
    Serial.println("\nConnected  MQTT");

    #if MQTT_SUBSCRIBE
    mqttClient.subscribe(MQTT_TOPIC);
  //mqttClient.unsubscribe(MQTT_TOPIC);
    #endif // MQTT_SUBSCRIBE
  }
  digitalWrite(LED, LED_ON);
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

// Provide callback function that would handle user-defined commands
String telnetCommandHandlerCallback (int argc, char *argv [], telnetServer_t::telnetConnection_t *tcn)
{
    #undef  LED_BUILTIN
    #define LED_BUILTIN  LED

    // Must be reentrant !!!

    #define argv0is(X) (argc > 0 && !strcmp (argv[0], X))  
    #define argv1is(X) (argc > 1 && !strcmp (argv[1], X))
    #define argv2is(X) (argc > 2 && !strcmp (argv[2], X))   

    // Short-running functions should return the text the Telnet server will send to the client as a response to the command
    if (argv0is ("turn") && argv1is ("led") && argv2is ("on")) {
            digitalWrite (LED_BUILTIN, LED_ON);
            return "Led is on";
    } else if (argv0is ("turn") && argv1is ("led") && argv2is ("off")) {
            digitalWrite (LED_BUILTIN, LED_OFF);
            return "Led is off";
    }

    // Long-running functions should provide a mechanism to break the loop 
    else if (argv0is ("led") && argv1is ("state")) {
            for (int i = 0; i < 1000; i++) {
                char buf [6];
                sprintf (buf, "%s", digitalRead (LED_BUILTIN) ? "on\r\n" : "off\r\n");
                if (tcn->sendString (buf) <= 0)
                  return "\r";
                delay (250); 

                if (tcn->peekChar ()) {
                    tcn->recvChar ();
                    return "\r"; // break the loop and return something different than "" to let the telnet server function know that the command has been processed
                }
            }
            return "\r"; // return something different than "" to let the telnet server function know that the command has been processed
    }

    // Unhandeled - let the Telnet server try to handle the command itself
    return "";
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

    #if MQTT_CLIENT
    // Connect to MQTT broker
    // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
    // by Arduino. You need to set the IP address directly.
    mqttClient.begin(MQTT_BROKER, wifiClient);
    mqttClient.onMessage(messageReceived);

    connect(  WIFI_ACCESSPOINT, MQTT_CLIENT );
    #endif // MQTT_CLIENT

    // Create Telnet server instance that would use thread-safe wrapper arround LittleFS (or FFat or SD)
    telnetServer = new (std::nothrow) telnetServer_t (TSFS, // optional arguments:
                                                      NULL, telnetCommandHandlerCallback, 23, NULL, true);
                                                      // Cstring<255> (*getUserHomeDirectory) (const Cstring<64>& userName, const Cstring<64>& password) = NULL
                                                      // String (*telnetCommandHandlerCallback) (int argc, char *argv [], telnetConnection_t *tcn) = NULL
                                                      // int serverPort = 23
                                                      // bool (*firewallCallback) (char *clientIP, char *serverIP) = NULL
                                                      // bool runListenerInItsOwnTask = true

    // Check if Telnet server instance is created && Telnet server is running
    if (telnetServer && *telnetServer)  Serial.println ("Telnet server started");
    else                                Serial.println ("Telnet server did not start");

    #if WIFI_ACCESSPOINT == 0
    // Setting the time is only important for time commands
    // Select another (POSIX) time zones: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    setenv ("TZ", "CET-2CEST,M3.5.0,M10.5.0/3", 1);
    ntpClient_t ntpClient ("1.si.pool.ntp.org", "2.si.pool.ntp.org", "3.si.pool.ntp.org");
    ntpClient.syncTime ();
    #endif

    // Create FTP server instance that would use thread-safe wrapper arround LittleFS (or FFat or SD)
    ftpServer = new (std::nothrow) ftpServer_t (TSFS);  // optional arguments:
                                                        //    Cstring<255> (*getUserHomeDirectory) (const Cstring<64>& userName, const Cstring<64>& password) = NULL
                                                        //    int serverPort = 21
                                                        //    bool (*firewallCallback) (char *clientIP, char *serverIP) = NULL
                                                        //    bool runListenerInItsOwnTask = true

    // Check if FTP server instance is created && FTP server is running
    if (ftpServer && *ftpServer)  Serial.println ("FTP server started");
    else                          Serial.println ("FTP server did not start");

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
    int32_t  now = millis();

    // - - - - - - - - - - - - - - - - - - - - - - -
    #if WIFI_ACCESSPOINT

    #define BLINK   1000L //   [ms]
    static int      ledstate = 0;
    static int32_t  blink    = 0;

    if ( (int32_t)(now - blink) >= BLINK ) {
      blink    += BLINK;
      ledstate ^= 1;
      digitalWrite(LED, ledstate);    // Toggle the LED on/off
    }

    #endif // WIFI_ACCESSPOINT
    // - - - - - - - - - - - - - - - - - - - - - - -
    #if MQTT_CLIENT
    
    #define PERIOD  1000L  // [ms]

    static int      counter  = 0;
    static int32_t  sum      = 0;
    static int32_t  previous = 0;
           char     line[256];

    mqttClient.loop();
    delay(10);         // <- fixes some issues with WiFi stability

    if ( !mqttClient.connected() ) {
        connect( WIFI_ACCESSPOINT, MQTT_CLIENT );
    }

    // Publish MQTT message every 1 seconds (PERIOD)
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

    String topic      = MQTT_TOPIC;
    float  cumulative = cumulative_sum( sum );

    #if defined(ESP32S3)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)
    int mV = 0;
    #else
    int mV = analogReadMilliVolts( ADC_PANEL );  // Debug testing
    #endif

    // Produce Octave and GnuPlot compatible data row
    #if 1
    snprintf( line, sizeof(line), "%3d  %.3f  %6d  %4d  %4d  %4d  %4d\r\n",
              solarIntensity, cumulative, counter, adcData_diff, adcValue.panel, adcValue.diode, adcValue.panel - mV );
    #else
    snprintf( line, sizeof(line), "%3d  %.3f  %6d  %4d\r\n", solarIntensity, cumulative, counter, adcData_diff );
    #endif

    mqttClient.publish( topic.c_str(), line, strlen(line) + 2 );  // Send also string terminating NULL character

    Serial.print("Message published:        ");
    Serial.print(line);

    #endif // MQTT_CLIENT
    // - - - - - - - - - - - - - - - - - - - - - - -
}
