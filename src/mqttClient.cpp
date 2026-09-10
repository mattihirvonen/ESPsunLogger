#include <WiFi.h>
#include <MQTT.h>
#include <stdint.h>
#include "pinMap.h"        // LED, BUTTON, AIN0, AIN1, ...
#include "measure.h"       // adcValue_t
#include "mqttClient.h"    // setup_mqtt(), loop_mqtt()

#define MQTT_CLIENT_ID  "aurinkopaneeli"
#define MQTT_USERNAME   "public"              // public.cloud.shiftr.io
#define MQTT_PASSWORD   "public"              // public.cloud.shiftr.io
#define MQTT_TOPIC      "solar/tikku"         // Select topic to not conflict with public brokers!
#define MQTT_SUBSCRIBE   0

   #define   MQTT_LOCAL   "127.0.0.1"                 // Localhost
   #define   MQTT_BROKER  "192.168.1.184"             // OK
// #define   MQTT_BROKER  "test.mosquitto.org"        // OK, require empty USERNAME and PASSWORD
// #define   MQTT_BROKER  "public.cloud.shiftr.io"    // OK, require non empty USERNAME and PASSWORD
// #define   MQTT_BROKER  "broker.hivemq.com"         // Test topic conflict with wild card using


extern  int          ADCref;            // Calibration value: Measured "adcValue.diff" [mV] at SPmax (2100)
extern  adcValue_t   adcValue;          // Work space variable (filtered ADC data)
extern  WiFiClient   wifiClient;
        MQTTClient   mqttClient;


void  connect( int wifi_accesspoint );


// Return value: 1.0 per each 100% of sun intensity hour
static float cumulative_sum( int32_t sum )
{
    float value = sum;

    return value / (100.0 * 3600.0);
}


static void connect_mqtt( int wifi_accesspoint, int mqtt_client )
{
  if ( ! wifi_accesspoint ) {
    connect( wifi_accesspoint );
  }

  if ( mqtt_client )
  {
    Serial.print("\nConnecting MQTT...");
  //while (!mqttClient.connect(MQTT_CLIENT_ID)) {
  //while (!mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {   // "public.cloud.shiftr.io"
    while (!mqttClient.connect(MQTT_CLIENT_ID, "", "")) {                         // "test.mosquitto.org"
      digitalWrite(LED, LED_OFF);
      Serial.print(".");
      delay(1000);
    }
    digitalWrite(LED, LED_ON);
    Serial.println("\nConnected  MQTT");

    #if MQTT_SUBSCRIBE
    mqttClient.subscribe(MQTT_TOPIC);
  //mqttClient.unsubscribe(MQTT_TOPIC);
    #endif // MQTT_SUBSCRIBE
  }
}


static void messageReceived( String &topic, String &payload )
{
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


void setup_mqtt( int wifi_accesspoint, int mqtt_client )
{
    // Connect to MQTT broker
    // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
    // by Arduino. You need to set the IP address directly.
    if ( ! mqtt_client ) {
        return;
    }
    if ( wifi_accesspoint ) {
        mqttClient.begin(MQTT_LOCAL,  wifiClient);
    }
    else {
        mqttClient.begin(MQTT_BROKER, wifiClient);
    }
    mqttClient.onMessage(messageReceived);

    connect_mqtt(  wifi_accesspoint, mqtt_client );
}


void loop_mqtt( int32_t now, int wifi_accesspoint, int mqtt_client )
{
    #define PERIOD  1000L  // [ms]

    static int      counter  = 0;
    static int32_t  sum      = 0;
    static int32_t  previous = 0;
           char     line[256];

    if ( ! mqtt_client ) {  // Nothing to do?
        return;
    }
    mqttClient.loop();
    delay(10);         // <- fixes some issues with WiFi stability

    if ( !mqttClient.connected() ) {
        connect_mqtt( wifi_accesspoint, mqtt_client );
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

    // Produce Octave and GnuPlot compatible data row
    #if 1
    snprintf( line, sizeof(line), "%3d  %.3f  %6d  %4d  %4d  %4d  %4d\r\n",
              solarIntensity, cumulative, counter, adcData_diff, adcValue.panel, adcValue.diode,
              adcValue.panel - adcValue.debug );
    #else
    snprintf( line, sizeof(line), "%3d  %.3f  %6d  %4d\r\n", solarIntensity, cumulative, counter, adcData_diff );
    #endif

    mqttClient.publish( topic.c_str(), line, strlen(line) + 2 );  // Send also string terminating NULL character

    Serial.print("Message published:        ");
    Serial.print(line);
}
