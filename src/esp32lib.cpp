
#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>    // MQTT

// ---------------------------------------------------------------------------------------
// Connect to the WiFi router

void setup_wifi( const char *ssid, const char *password )
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

// ---------------------------------------------------------------------------------------
// Example: Create an instance of the HardwareSerial class for Serial 2
// HardwareSerial  SerialVE(2);

// Start Serial 2 with the defined RX and TX pins and a baud rate 
void setup_uart2( HardwareSerial &Serial2, int bitrate, int RxBufferSize, int TxBufferSize )
{
  // Define the RX and TX pins for Serial 2
  #define RXD2     16
  #define TXD2     17

  Serial2.setRxBufferSize( RxBufferSize );
  Serial2.setTxBufferSize( TxBufferSize );
  Serial2.begin( bitrate, SERIAL_8N1, RXD2, TXD2 );
  //
  Serial.println("Serial 2 started at 19200 baud rate");
}

// ---------------------------------------------------------------------------------------
// Example: Create an instance of WiFiUDP
// WiFiUDP  udp;

void setup_udp( WiFiUDP &udp, const int udpPort )
{
  udp.begin(udpPort);
}


void udp_send( WiFiUDP &udp, const char *udpAddress, int udpPort, const uint8_t *data, int count )
{
    udp.beginPacket(udpAddress, udpPort);
    udp.write( data, count );
    udp.endPacket();
}


// ---------------------------------------------------------------------------------------
// Example: Create an instance of "mqttClient"" 

#if 0
const char*   mqtt_server = "192.168.1.184";  // "broker.hivemq.com";
//
WiFiClient    espClient;
PubSubClient  mqttClient( espClient );

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
#endif

void setup_mqtt( PubSubClient  &mqttClient, const char *mqtt_server, int port )
{
  mqttClient.setServer( mqtt_server, port );
//mqttClient.setCallback( mqtt_callback );
}

// ---------------------------------------------------------------------------------------
// Example: Create an instance of MQTTClient 
// MQTTClient client;

/*
#include <MQTT.h>

// Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.

void messageReceived(String &topic, String &payload) {
    ...
}

client.begin("public.cloud.shiftr.io", net);
client.onMessage(messageReceived);
client.subscribe("/hello");
client.unsubscribe("/hello");
client.publish("/hello", "world");

*/

void mqtt_reconnect( PubSubClient &mqttClient )
{
  while ( !mqttClient.connected() )
  {
    Serial.print("Attempting MQTT connection...");
    if ( mqttClient.connect("ESP32Client") ) {
      Serial.println("connected");
//    mqttClient.subscribe("#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
