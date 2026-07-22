
//
// ESP32 ADC is enough linear at range 150 mV ... 2500 mV to measure
// small solar panel's current (we are here interrested only 10% accuracy.
//
// https://suncalc.org
// https://lucidar.me/en/esp32/linearity-of-the-esp32-adc/
// https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
// https://randomnerdtutorials.com/esp-idf-esp32-gpio-analog-adc/
//
// Use here floating points (non efficient and only sign+23 bits mantissa)
//
// Select current measurement shunt resistance value:
// - resistance value "Rshunt" is <= (3.0V / Iref)
// - where "Iref" is measured solar panel's short circuit current at max solar intensity

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <WiFi.h>
#include <stdint.h>

#define UNUSED  __attribute__((unused))

#define ADC_IOPIN  34      // Analog ADC1_CH6
#define SPmax      950     // Sun's peak power [W/m2] at latitude 60 deg. north (summer time)

float   Iref   = 0.024;    // Solar panel's measured "short circuit" current [A] at SPmax
float   Rshunt = 100.0;    // Current shunt resistance [ohm]: Select value <= (2.5V / Iref)
//
int     ADCref = 2960;     // Measured ADC value at SPmax (and  also at Iref)
int     Ntaps  = 100;      // Filter coefficient

int     adcValue;          // Work space variable


// Dummy IIR style filtering
int floatingAverage( int32_t *sum, int x, int N )
{
	int avg = *sum / N;
	
	sum -= avg;
	sum += x;
	
	return *sum / N;
}


void taskMeasure( void *pvParameters )
{
	#define TASK_PERIOD  10  // in tick(s) [ms]
	
	static TickType_t  xLastWakeTime = 1000;
	static int32_t     sum           = 0;

    // Wait for the next cycle.
    BaseType_t UNUSED  xWasDelayed = xTaskDelayUntil( &xLastWakeTime, TASK_PERIOD );
	
    int adc_result = analogRead( ADC_IOPIN );
	
	adcValue = floatingAverage( &sum, adc_result, Ntaps );
}


void setup( void )
{
	Serial.begin( 115200 );
	
	xTaskCreate(
      taskMeasure,    // function name
      "Measure",      // task name (for debugging)
      128,            // stack size in words (not bytes)
      NULL,           // parameters to pass
      2,              // priority (1 = lowest)
      NULL            // task handle (optional)
  );
}


void loop( void )
{
	#define PERIOD  1000L  // [ms]
	
    static int      counter  = 0;
	static int32_t  sum      = 0;
	static int32_t  previous = 0;
	       int32_t  now      = millis();
		   char     line[256];
	
	if ( (int32_t)(now - previous) < PERIOD ) {
		return;
	}
	previous += PERIOD;
	
	int solarIntensity = (100 * adcValue) / ADCref;    // Solar's intensity [%]
	
	sum += solarIntensity;                             // Note: Overflows after few years
	counter++;
	snprintf( line, sizeof(line), "%d: adc %4d - solar intensity %3d - cumulative %2ld\r\n", counter, adcValue, solarIntensity, sum / counter );

	Serial.printf("%s", line);
}
