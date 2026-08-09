
# ESPsunLogger

This is tiny/dirty two evening demo project to measure solar intensity using ESP(32) processor and small solar cell.
In my test case I will use old Nokia (16xx model) phone's back plate containing small solar cell.
Project's goal is to visualize how much clouds drop solar panel's output power.
ESP32 ADC is not precision measurement instrument.
Using careful calibration sequence results will be some how 10% accuracy.
Solar intensites below 10% of "sun full power" will be more inaccurate.

When estimate true solar panel output, we have to understand also solar panel's orientation versus sun's direction.
- azimuth (compass direction from north - degrees)
- height (above horizon - degrees)

![Panel horizontal - Caravan "roof installation"](sunHistory_2026-08-09.pdf)

Scale
- Intensity 100% is same as theretical 100W solar panel max output at noon from clear sky (100W)
- Cumulative 1.0 is same as 100 Wh (with 100W solar panel))

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
- measure panel's raw short circuit current (my case about 0.027 A measured with multimeter)
- select current shunt resistor value about (2.0V / short circuit current \[A\])
- example: 2.0V / 0.024 A = 83 ohm, we select 82 ohm standard resistor (enough close to 83 ohm)
- install selected shunt resistor as load to solar panel
- check that loaded panel voltage over shunt resistor and (schottky) diode is less than 3.2V (when sun shine at full power)
- connect panel+shunt resistor to ESP processor
- read  application measured ADCdiff value
- update "*ADCref*" variable value with this measured ADCdiff value

### Build Environments
Application can build using
- Arduino IDE / CLI
- VSCode + PIOARDUINO

### External Libraries
Application use following libraries
- *MQTT* MQTT library by Joel Gaehwiler
- *PubSubClient* MQTT library by Nick O'Leary (obsolete, not used any more)

### Data Post Prosessing And Visualization
Read file *linux/README.md*

Data post prosessing and visualization tools.
- https://octave.org/
- http://gnuplot.info/

### Some Free public MQTT Servers
Look also using browser following sites web URL ( https://... ) 
- test.mosquitto.org - Ports: MQTT 1883, WebSocket 8081 (wss://test.mosquitto.org)
- https://test.mosquitto.org
- public.cloud.shiftr.io - Ports: MQTT 1883, WebSocket 443 (wss://public.cloud.shiftr.io , Public credentials: try / try , Security: Always prefer wss:// with TLS)
- https://www.shiftr.io/docs/cloud/
- broker.hivemq.com - Ports: MQTT 1883, WebSocket 8884 (wss://broker.hivemq.com , Path: /mqtt)
- https://www.hivemq.com/mqtt/public-mqtt-broker/
- https://www.hivemq.com/blog/
- https://www.hivemq.com/mqtt-toolbox/
HiveMQ site have many (generic) interesting artichles about MQTT

### Example commands to Testing MQTT Server Message Passing
- http://www.steves-internet-guide.com/mosquitto_pub-sub-clients/
- mosquitto_sub -v -h test.mosquitto.org  -t "home/sundata/#"
- mosquitto_pub    -h test.mosquitto.org  -t "home/sundata/value"  -m "12345"


### ToDo...
- Add some info to project how to use GnuPlot for data visualization
- daily/hourly sun intensity history
