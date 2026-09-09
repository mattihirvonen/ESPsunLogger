#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

void setup_mqtt( int wifi_accesspoint, int mqtt_client );
void loop_mqtt( int32_t now, int wifi_accesspoint, int mqtt_client, int adc_panel );

#endif // MQTTCLIENT_H
