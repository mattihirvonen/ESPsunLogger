
#ifndef MEASURE_H
#define MEASURE_H

#ifndef UNUSED
#define UNUSED  __attribute__((unused))
#endif

typedef struct
{
    int     panel;     // [mV]
    int     diode;     // [mV]
    int     diff;      // [mV]
    //
    int     debug;     // [mV]
}  adcValue_t;


typedef struct
{
  uint16_t   mV;
  int16_t    mA;
} loggerData_t;


typedef struct
{
  uint32_t      run;       // Start or run logger
  uint32_t      samples;   // Log size of samples to measure
  uint32_t      count;     // Runtime current count of samples
  loggerData_t  *data;
} logger_t;


void      taskMeasure( void UNUSED *pvParameters );
void      measure_start( uint32_t seconds );
uint32_t  measure_period( uint32_t ms );

#endif // #define MEASURE_H
