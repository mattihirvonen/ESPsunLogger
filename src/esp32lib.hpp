
void setup_wifi( const char *ssid, const char *password );

void setup_uart2( HardwareSerial &Serial2, int bitrate, int RxBufferSize, int TxBufferSize );

void setup_udp( WiFiUDP &udp, const int udpPort );

void udp_send( WiFiUDP &udp, const char *udpAddress, int udpPort, const uint8_t *data, int count );

void setup_mqtt( PubSubClient  &mqttClient, const char *mqtt_server, int port );

void mqtt_reconnect( PubSubClient &mqttClient );

void mqtt_callback(char* topic, byte* message, unsigned int length);
