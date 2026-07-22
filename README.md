
# ESPsunLogger

This is tiny demo project to measure solar intensity using ESP(32) processor and small solar cell.
In my test case I will use old Nokia (16xx model) phone's back plate containing small solar cell.

### Measuring Strategy
We will measure solar cell's "short circuit" current using small current shunt resistor.
Here in 60 deg. north latitude sun will shine about 950 W/m2 from clear sky at noon (summer time).
- https://suncalc.org

### Current Measurement Calibration
We will use sunny weather condition as reference to calibrate / scale our measurements.
ESP32's ADC is enough linear for solar measurements in range 150 mV ... 2500 mV
(with acceptable non linearity up to 3000 mV).

Calibration sequence
- measure panel's raw unloaded open circuit voltage (my case about 10.8 V which can damage ESP processor)
- measure panel's raw short circuit current (my case about 0.024 A measured with multimeter)
- select current shunt resistor value about (2.5V / short circuit current [A])
- example: 2.5V / 0.024 A = 104 ohm, we select 100 ohm standard resistor (enough close to 104 ohm)
- install selected shunt resistor as load to solar panel
- check that loaded panel voltage over shunt resistor is less than 3.2V (when sun shine at full power)
- connect panel+shunt resistor to ESP processor 
- read  application measured ADC value
- update "*ADCref*" variable value with this measured ADC value

### Build Environments
Application can build using
- Arduino IDE / CLI
- VSCode + PIOARDUINO

### ToDo...
- daily/hourly sun intensity history
- WiFi/MQTT client to send measurement data to message broker
