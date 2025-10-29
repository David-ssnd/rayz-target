#ifndef CONFIG_H
#define CONFIG_H

#define PHOTODIODE_PIN 2
#define SAMPLE_INTERVAL 100

#define OLED_SDA 8
#define OLED_SCL 9
#define OLED_ADDRESS 0x3C

const float ADC_VREF = 3.3;
const int ADC_RESOLUTION = 4095;
const float THRESHOLD_MARGIN = 0.1;

#endif
