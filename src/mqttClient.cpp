
#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>
#include "pinMap.h"        // LED, BUTTON, AIN0, AIN1, ...
#include "solar.h"
#include "mqttClient.h"    // setup_mqtt_client(), loop_mqtt_client()

#define MQTT_CLIENT_ID  "solarcell"
#define MQTT_USERNAME   "public"              // public.cloud.shiftr.io
#define MQTT_PASSWORD   "public"              // public.cloud.shiftr.io
#define MQTT_SUBSCRIBE   0

// #define   MQTT_LOCAL   "127.0.0.1"                 // Localhost
   #define   MQTT_LOCAL   "192.168.4.1"               // Local AP
   #define   MQTT_BROKER  "192.168.1.184"             // OK
// #define   MQTT_BROKER  "test.mosquitto.org"        // OK, require empty USERNAME and PASSWORD
// #define   MQTT_BROKER  "public.cloud.shiftr.io"    // OK, require non empty USERNAME and PASSWORD
// #define   MQTT_BROKER  "broker.hivemq.com"         // Test topic conflict with wild card using

static  int          mqtt_enabled   = 0;    // No matter to use int or bool here...
static  int          wifi_localhost = 0;    // WiFi AP mode use localhost message broker
static  WiFiClient   net;
        MQTTClient   mqttClient;            // "public" for "solar" 

//void loop_solar_intensity( int32_t now );   // Real "work horse"


static void connect( int wifi_accesspoint )
{
  // No need to check connection to WiFi router when WiFi is in AP mode
  if ( ! wifi_accesspoint )
  {
    Serial.print("\nChecking   WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED, LED_OFF);
      Serial.print(".");
      delay(1000);
    }
    digitalWrite(LED, LED_ON);
    Serial.print("\nConnected  WiFi (router)");
  }
}


static void connect_mqtt( int wifi_accesspoint )
{
    connect( wifi_accesspoint );

    Serial.print("\nConnecting MQTT...");
//  while (!mqttClient.connect(MQTT_CLIENT_ID)) {
//  while (!mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {   // "public.cloud.shiftr.io"
    while (!mqttClient.connect(MQTT_CLIENT_ID, "", "")) {                         // "test.mosquitto.org"
      digitalWrite(LED, LED_OFF);
      Serial.print(".");
      delay(1000);
    }
    digitalWrite(LED, LED_ON);
    Serial.println("\nConnected  MQTT");

    #if MQTT_SUBSCRIBE
    mqttClient.subscribe(MQTT_TOPIC);
//  mqttClient.unsubscribe(MQTT_TOPIC);
    #endif // MQTT_SUBSCRIBE
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


void setup_mqtt_client( int wifi_accesspoint, int mqtt_client )
{
    // Connect to MQTT broker
    // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
    // by Arduino. You need to set the IP address directly.

    mqtt_enabled = mqtt_client;
    if ( ! mqtt_enabled ) {
        return;
    }
    if ( wifi_accesspoint ) {
        mqttClient.begin(MQTT_LOCAL,  net);
        wifi_localhost = 1;
    }
    else {
        mqttClient.begin(MQTT_BROKER, net);
    }
    mqttClient.onMessage(messageReceived);

    connect_mqtt( wifi_localhost );
}


void loop_mqtt_client( int32_t now )
{
  if ( ! mqtt_enabled ) {  // Nothing to do?
        return;
    }
    mqttClient.loop();
    delay(10);             // <- fixes some issues with WiFi stability

    if ( !mqttClient.connected() ) {
        connect_mqtt( wifi_localhost );
    }
}

//------------------------------------------------------------------------------------------------
