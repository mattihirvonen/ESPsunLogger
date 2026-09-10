
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

void taskMeasure( void UNUSED *pvParameters );

#endif // #define MEASURE_H
