#include <Arduino.h>                 // TickType_t
//#include <stdint.h>
//#include <string.h>
//#include <freertos/FreeRTOS.h>     // TickType_t

#include "pinMap.h"
#include "measure.h"
#include "INA219.h"

#define TASK_PERIOD 50       // in tick(s) [ms]
#define LOGSIZE     10000

static  TickType_t  xTaskPeriod = pdMS_TO_TICKS( TASK_PERIOD );

// Public object(s) for telnet and other source code modules
INA219  INA( INA219_ADDRESS );

//-----------------------------------------------------------------------------------------

// Calibration info:
// - Panel 1:  ESP32 Devkit1, ADC_DIFF=2200, Rshunt=82  (2026-07-28)
//
// 2022-07-29 sun from clear sky 942 W/m2 (99%), Rshunt 82 ohm (2.4 h kohdalla lokissa)
// - time=12:45, Udiff=1785, Upanel=2170 mV, ADC_REF=2200, intensity=81
// - fix ADC_REF: 2200 * 81 / 99 = 1800
//
// - time=13:20, Udiff=1795, Upanel=2175 mV, ADC_REF=2200, intensity=81
// - fix ADC_REF: 2200 * 81 / 99 = 1800

#if defined(ESP32S3)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)
#define ADC_PANEL     AIN0
#define ADC_DIODE     AIN1
#else
#define ADC_PANEL     AIN6    // GPIO pin: Analog ADC1_CH6 - ESP32 DEVKIT V1
#define ADC_DIODE     AIN7    // GPIO pin: Analog ADC1_CH7 - ESP32 DEVKIT V1
#endif

#define ADC_CHANNELS     2    // 2: shunt and diode // 1: only shunt (fix voltage diode)
#define SPmax          950    // Sun's peak power [W/m2] at latitude 60 deg. north (summer time)
#define ADC_REF       1800    // Calibration value: "adcValue.diff" at "SPmax"

//-----------------------------------------------------------------------------------------

float         Iref   = 0.0270;      // Solar panel's measured "short circuit" current [A] at SPmax
float         Rshunt = 82.0;        // Current shunt resistance [ohm]: Select value <= (2.5V / Iref)
//
int           ADCref = ADC_REF;     // Calibration value: Measured "adcValue.diff" [mV] at SPmax (2100)
int           Ntaps  = 20;          // Filter coefficient
//
adcValue_t    adcValue;             // Work space variable (filtered ADC data)
//
int           loggerRun = 0;
uint32_t      loggerSamples;
loggerData_t  loggerData[LOGSIZE];

//-----------------------------------------------------------------------------------------

static void measure_sun( void );
static void measure_logger( void );


// Dummy IIR style filtering
static int floatingAverage( int32_t *sum, int x, int N )
{
    int avg = *sum / N;

    *sum -= avg;
    *sum += x;

    return *sum / N;
}


/*
// Template function to linearize ADC measurement result
int adcLinearize( int mV )
{
    // Raw: ADC linear range 200 mV ... 2500 mV:
    // - 2586/2.2V - 115/0.2V -> 1235/V
    // - 2961/2.5V - 115/0.2V -> 1238/V
    // - 2961/2.5V - 240/0.3V -> 1237/V

    // Schottky diode voltage drop (abt 300 mV at 1 mA)
    return (mV > 250) ? mV : 0;
}
*/


void taskMeasure( void UNUSED *pvParameters )
{
    static  TickType_t  xLastWakeTime;

    pinMode( ADC_DIODE, INPUT );
    pinMode( ADC_PANEL, INPUT );

    if ( Wire.begin() )  { Serial.println("I2C initialization ok");                        }
    else                 { Serial.println("I2C initialization fail");                      }
    if ( INA.begin()  )  { Serial.println("I2C connect to INA ok");                        }
    else                 { Serial.println("Could not I2C connect to INA. Fix and Reboot"); }

    // Set INA219 chip mode continuous 16 samples averaging (+-320 mV)
    INA.reset();

    // Initializetion: Get current uptime
    xLastWakeTime = xTaskGetTickCount();

    while ( 1 ) // Loop for ever
    {
        // Wait for the next cycle.
        BaseType_t UNUSED  xWasDelayed = xTaskDelayUntil( &xLastWakeTime, xTaskPeriod );

        measure_sun();
        measure_logger();
    }
}

//-----------------------------------------------------------------------------------------

static void measure_sun( void )
{
    #define DIODE_mV  265   // BAT85 typical: 250 mV / 0.3 mA - 300 mV / 1 mA

    static int32_t     sum_panel = 0, sum_diode = 0, sum_diff = 0;
           int          mV_panel,      mV_diode;
        // int          mV_diff;

    // ADC result offset and gain fixes required with raw uncalibrated ADC data
    #if defined(ESP32S3)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)
    mV_panel = 0;                                         // ESP32S3 fail/crash with
    mV_diode = 0;                                         // analogRead() and
//  mV_panel = analogRead( ADC_PANEL );                   // analogReadMilliVolts()
//  mV_diode = analogRead( ADC_DIODE );                   // functions
    #else // ESP32S3
    mV_panel       = analogReadMilliVolts( ADC_PANEL );   // Factory calibrated !!!
    #if  ADC_CHANNELS > 1
    mV_diode       = analogReadMilliVolts( ADC_DIODE );   // Factory calibrated !!!
    adcValue.debug = analogReadMilliVolts( ADC_PANEL );   // Debug testing...
    #else
    mV_diode       = DIODE_mV;                            // Single channel ADC measurement
    adcValue.debug = DIODE_mV;
    #endif
    #endif // ESP32S3

    // Filter measurement results
    adcValue.panel = floatingAverage( &sum_panel, mV_panel, Ntaps );
    adcValue.diode = floatingAverage( &sum_diode, mV_diode, Ntaps );
    adcValue.diff  = floatingAverage( &sum_diff,  adcValue.panel - adcValue.diode, Ntaps );
}

//-----------------------------------------------------------------------------------------

uint32_t measure_period( uint32_t ms )
{
    if ( ms ) {
        xTaskPeriod = pdMS_TO_TICKS( ms );
    }
    else {
        ms = pdTICKS_TO_MS( xTaskPeriod );        
    }
    Serial.print("\r\nMeasure period ");
    Serial.println(ms);
    return ms;
}


void measure_start( uint32_t seconds )
{
    uint32_t samples = seconds * 1000 / xTaskPeriod;

    if ( samples > LOGSIZE ) {
         samples = LOGSIZE;
    }
    loggerSamples = samples;
    loggerRun     = 1;
    Serial.println("\r\nMeasure start");
    return;
}


static void measure_logger( void )
{
    static uint32_t ix = 0;

    if ( ! INA.isConnected() ) {
        loggerRun = 0;
        Serial.println("\r\nError: INA is not connected!");
        return;
    }
    if ( ! loggerRun ) {
        return;
    }
    if ( ix >= loggerSamples ) {
        loggerRun = 0;
        Serial.println("\r\nMeasure stop");
        return;
    }
    int offset = -10;      // [uV]
    int Rshunt = 100;      // [mOhm]

    int uV = INA.shunt_uV() - offset;
    int mA = uV / Rshunt;
    int mV = INA.bus_mV();

    loggerData[ix].mV = mV;
    loggerData[ix].mA = mA;
    ix += 1;    
}
