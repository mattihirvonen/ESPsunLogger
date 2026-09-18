#include <Arduino.h>
#include <stdint.h>
#include <MQTT.h>          // Testing does "256dpi/MQTT@2.5.3" library work...
#include "mqttService.h"
#include "pinMap.h"        // LED, BUTTON, AIN0, AIN1, ...
#include "measure.h"       // adcValue_t
#include "solar.h"

#define  MQTT_TOPIC  "solar/tikku"         // Select topic to not conflict with public brokers!

extern   int          ADCref;        // Calibration value: Measured "adcValue.diff" [mV] at SPmax (2100)
extern   adcValue_t   adcValue;      // Work space variable (filtered ADC data)
extern   MQTTClient   mqttClient;            // "public" for "solar" 


// Return value: 1.0 per each 100% of sun intensity hour
static float cumulative_sum( int32_t sum )
{
    float value = sum;

    return value / (100.0 * 3600.0);
}


void setup_solar_intensity( void )
{
    return;
}


void loop_solar_intensity( int32_t now )
{
    #define PERIOD  1000L  // [ms]

    static int      counter  = 0;
    static int32_t  sum      = 0;
    static int32_t  previous = 0;
           char     line[256];

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
    snprintf( line, sizeof(line), "%3d  %.3f  %6d  %4d  %4d  %4d  %4d",
              solarIntensity, cumulative, counter, adcData_diff, adcValue.panel, adcValue.diode,
              adcValue.panel - adcValue.debug );
    #else
    snprintf( line, sizeof(line), "%3d  %.3f  %6d  %4d\r\n", solarIntensity, cumulative, counter, adcData_diff );
    #endif

    // Set #if to '1' for testing library: 256dpi/MQTT@2.5.3
    #if 0
    // Send also string terminating NULL character
    mqttClient.publish( topic.c_str(), line, strlen(line) + 2 );  // Use library: 256dpi/MQTT@2.5.3
    #else
    mqtt_publish( topic.c_str(), line );  // Use PicoMQTT library wrapper
    #endif

    Serial.printf("Message published:        %s", line);
}
