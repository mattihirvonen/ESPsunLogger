// MQTT broker(s):
// - https://github.com/terrorsl/sMQTTBroker
// - https://github.com/HighVoltages/ESP32-MQTT-broker
// - https://docs.arduino.cc/libraries/smqttbroker/?_gl=1*1e6kof5*_up*MQ..*_ga*Mjc0NDAyODE0LjE3ODk0NTA2Nzg.*_ga_NEXN8H46L5*czE3ODk0NTA2NzYkbzEkZzEkdDE3ODk0NTE1MjYkajQ5JGwwJGgxNzY2MDI0OTkx
// - https://ckarduino.blog/2025/08/02/local-esp32-mqtt-broker/
// - https://github.com/ckuehnel/ESP32-MQTT-Broker/tree/main

// https://registry.platformio.org/libraries/mlesniew/PicoMQTT/installation
// https://github.com/mlesniew/PicoMQTT

#define  picoMQTT    1
#define  sMQTT       0

static void setup_mqtt_broker_picoMQTT( int wifi_accesspoint );
static void setup_mqtt_broker_sMQTT( void );

static void loop_mqtt_broker_picoMQTT( void );
static void loop_mqtt_broker_sMQTT( void );


void setup_mqtt_service( int wifi_accesspoint )
{
    #if picoMQTT
    setup_mqtt_broker_picoMQTT( wifi_accesspoint );
    #endif
    #if sMQTT
    setup_mqtt_broker_sMQTT();
    #endif
}


void loop_mqtt_service( void )
{
    #if  picoMQTT
    loop_mqtt_broker_picoMQTT();
    #endif
    #if sMQTT
    loop_mqtt_broker_sMQTT();
    #endif
}

//----------------------------------------------------------------------------------
#if picoMQTT

#include <PicoMQTT.h>
#include "mqttService.h"

static  PicoMQTT::Server  mqttPicoServerLocalhost;
static  PicoMQTT::Client  mqttPicoClientExtern1("192.168.1.184");
static  PicoMQTT::Client  mqttPicoClientExtern2("test.mosquitto.org");

static  PicoMQTT::Server *mqttPicoServer = &mqttPicoServerLocalhost;
static  PicoMQTT::Client *mqttPicoClient = &mqttPicoClientExtern1;

static  int  accesspoint = 0;

// Define function pointer "mqtt_callback()" to MQTT message call back handler
static  mqtt_callback_t  mqtt_callback = NULL;

static  void mqtt_message_handler( const char * topic, const char * payload, const char * debug );


// Example for MQTT message call back handler function (default handler)
// PicoMQTT make "topic" and "payload" null terminated
void mqtt_message_handler_example( const char * topic, const char * payload, const char * debug )
{
    Serial.printf("Received message in topic '%s': '%s' - %s\n", topic, payload, debug );
}


void mqtt_set_callback( mqtt_callback_t callback_func )
{
    mqtt_callback = callback_func;
}


void mqtt_subscribe( const char * topic )
{
    const char * payload = NULL;

    if ( accesspoint )
    {
        mqttPicoServer->subscribe(topic, [](const char * topic, const char * payload) {
            mqtt_message_handler( topic, payload, "server" );
        });
    }
    else
    {
        mqttPicoClient->subscribe(topic, [](const char * topic, const char * payload) {
            mqtt_message_handler( topic, payload, "client" );
        });
    }
}


void mqtt_publish( const char * topic, const char * payload )
{
    if ( accesspoint )  {  mqttPicoServer->publish(topic, payload);  }
    else                {  mqttPicoClient->publish(topic, payload);  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// PicoMQTT make "topic" and "payload" null terminated
static void mqtt_message_handler( const char * topic, const char * payload, const char * debug )
{
    // Set example default handler
    if ( !  mqtt_callback ) {
        mqtt_set_callback( &mqtt_message_handler_example );
    }
    if ( mqtt_callback ) {
         mqtt_callback( topic, payload, debug );
    }
}


static void setup_mqtt_broker_picoMQTT( int wifi_accesspoint )
{
    accesspoint = wifi_accesspoint;

    // Subscribe to a topic pattern and attach a callback
    #if 0
    mqttPicoServer->subscribe("#", [](const char * topic, const char * payload) {
        mqtt_message_handler( topic, payload, "server" );
    });
    mqttPicoClient->subscribe("#", [](const char * topic, const char * payload) {
        mqtt_message_handler( topic, payload, "client" );
    });
    #endif

    mqtt_subscribe( "#" );

    #if 0
    mqttPicoServer->subscribe("#", [](const char * topic, const char * payload) {
        Serial.printf("Received message in topic '%s': %s\n", topic, payload);
    });
    #endif

    if ( accesspoint )  {  mqttPicoServer->begin();  }
    else                {  mqttPicoClient->begin();  }
};


static void loop_mqtt_broker_picoMQTT( void )
{
    if ( accesspoint )  {  mqttPicoServer->loop();  }
    else                {  mqttPicoClient->loop();  }

    #if 1
    if (random(10000) == 0) {
        static int counter = 0;
        Serial.printf("picomqtt: publish %d\r\n", ++counter );
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Hello from PicoMQTT! (%d)", counter);
        mqtt_publish( "picomqtt/welcome", buffer );
    }
    #endif
}

#endif // picoMQTT
//----------------------------------------------------------------------------------
#if sMQTT

#include <sMQTTBroker.h>
#define  MQTT_PORT  1883

#define noEVENS  1
#if     noEVENS
static  sMQTTBrokerWithoutEvent  broker;
#else
static  sMQTTBroker broker;

const char* MQTT_CLIENT_USER     = "user";     // username for mqtt clients. Set your own value here.
const char* MQTT_CLIENT_PASSWORD = "password"; // password for mqtt clients. Set your own value here.


class MyBroker:public sMQTTBroker
{
public:
    bool onEvent(sMQTTEvent *event) override
    {
        switch(event->Type())
        {
            case NewClient_sMQTTEventType:
                {
                    sMQTTNewClientEvent *e=(sMQTTNewClientEvent*)event;
                        // Check username and password used for new connection
                    if ((e->Login() != MQTT_CLIENT_USER) || (e->Password() != MQTT_CLIENT_PASSWORD)) {
                    Serial.println("Invalid username or password");  
                    return false;
                    }
                };
                break;
            case LostConnect_sMQTTEventType:
                WiFi.reconnect();
                break;
            case UnSubscribe_sMQTTEventType:
            case Subscribe_sMQTTEventType:
                {
                    sMQTTSubUnSubClientEvent *e=(sMQTTSubUnSubClientEvent*)event;
                }
                break;
        }
        return true;
    }
};

static MyBroker broker;

#endif // noEVENTS

static void setup_mqtt_broker_sMQTT( void )
{
    broker.init(MQTT_PORT);
};

static void loop_mqtt_broker_sMQTT( void )
{
    broker.update();
}

#endif // sMQTT
//----------------------------------------------------------------------------------
