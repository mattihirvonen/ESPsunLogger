#ifndef PINCONFIG_H
#define PINCONFIG_H

#if defined(ESP32S3)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)

#define LED           21      // GPIO21 (yellow  LED on XIAO ESP32S3)
#define LED_ON         0      // Pull LED to ground
#define LED_OFF        1
#define BUTTON         0      // GPIO0 - Boot button on XIAO ESP32S3
#define AIN0           1      // GPIO1 - A0
#define AIN1           2      // GPIO2 - A1
#define AIN2           3      // GPIO3 - A2
#define AIN3           4      // GPIO4 - A3
#define AIN4           5      // GPIO5 - A4 - SCL
#define AIN5           6      // GPIO6 - A5 - SDA
#define AIN8           7      // GPIO7 - A3 - SCK
#define AIN9           8      // GPIO8 - A3 - MISO
#define AIN10          9      // GPIO9 - A3 - MOSI

#else // ESP32 devkit (NodeMCU) - DOIT / JoyIT

#define LED            2      // GPIO2  (blue LED    on NodeMCU devkit)
#define LED_ON         1      // Push HI to LED
#define LED_OFF        0
#define BUTTON         0      // GPIO0  - Boot button on NodeMCU devkit
#define AIN0          36      // GPIO36 - ADC1_CH0
#define AIN3          39      // GPIO39 - ADC1_CH3
#define AIN4          32      // GPIO32 - ADC1_CH4
#define AIN5          33      // GPIO33 - ADC1_CH5
#define AIN6          34      // GPIO34 - ADC1_CH6
#define AIN7          35      // GPIO35 - ADC1_CH7

#endif

#endif //  PINCONFIG_H

