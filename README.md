
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
- measure panel's short circuit current (my case about 0.020 A)
- select current shunt resistor value about (2.5V / short circuit current [A])
- example: 2.5V / 0.020 A = 125 ohm, we select 120 ohm standard resistor (enough close to 125 ohm)
- install selected shunt resistor
- read  application measured ADC value
- update "*ADCref*" variable value with this measured ADC value

### Build Environments
Application can build using
- Arduino IDE / CLI
- VSCode + PIOARDUINO

### ToDo...
- daily/hourly sun intensity history
- WiFi/MQTT client to send measurement data to message broker
