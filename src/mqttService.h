#ifndef MQTTSERVICE_H
#define MQTTSERVICE_H

// Define function prototype "mqtt_callback_t" for MQTT message call back handler
typedef void (*mqtt_callback_t)(const char*, const char*, const char*);

void mqtt_set_callback( mqtt_callback_t callback_func );
void mqtt_subscribe( const char * topic );
void mqtt_publish( const char * topic, const char * payload );

void setup_mqtt_service( int wifi_accesspoint );
void loop_mqtt_service( void );

#endif // MQTTSERVICE_H
