#ifndef CONFIG_H
#define CONFIG_H

#include "protocol_config.h"

#define PHOTODIODE_PIN 34
#define VIBRATION_PIN 4

#define SAMPLE_INTERVAL SAMPLE_INTERVAL_MS
#define VIBRATION_DURATION 50

const float ADC_VREF = 3.3;
const int ADC_RESOLUTION = 4095;
const float THRESHOLD_MARGIN = 0.1;

#endif
