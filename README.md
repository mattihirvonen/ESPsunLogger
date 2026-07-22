
# ESPsunLogger

This is tiny demo project to measure solar intensity using ESP832) processor and small solar cell.
In my test case I will use old Nokia (16xx model) phone's back plate containing small solar cell.

### Current Measurement

We will measure solar cell's "short circuit" current using small current shunt resistor.
Here in 60 deg. north latitude sun will shine 950 W/m2 from clear sky at noon (summer time).
We will use sunny weather condition as reference to calibrate / scale our measurements.
ESP32's ADC is enough linear for solar measurements in range 150 mV ... 2500 mV
(with acceptable non linearity up to 3000 mV).

https://suncalc.org

Calibration sequence
- measure panel's unloaded open circuit voltage  (my case about 8.9 V)
- measure panel's short circuit current (my case about 0.030 A)
- select current shunt resistor value less than (2V / short circuit current [A])
- example: 2V / 0.030 A = 66.7 ohm, we select 68 ohm standard resistor
- print/read measured ADC value from application
- update "*ADCref*" variable value wth this measured ADC value

