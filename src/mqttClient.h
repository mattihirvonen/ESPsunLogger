#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

void setup_mqtt_client( int wifi_accesspoint, int mqtt_client );
void loop_mqtt_client( int32_t now );

#endif // MQTTCLIENT_H
