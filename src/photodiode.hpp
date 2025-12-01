#ifndef PHOTODIODE_HPP
#define PHOTODIODE_HPP

#include "config.h"
#include "hash.h"
#include <esp_adc/adc_oneshot.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class Photodiode
{
  private:
    float sampleBuffer[SAMPLES_PER_BIT];
    int sampleIndex;
    float bitBuffer[PHOTODIODE_BUFFER_SIZE];
    int bitHead;  // Next write position (ring buffer)
    int bitCount; // How many entries are valid until buffer fills
    bool bufferFull;
    bool sampleBufferFull;
    float runningMin;
    float runningMax;
    float dynamicThreshold;
    uint32_t lastSampleTime;
    uint32_t bitStartTime;
    adc_oneshot_unit_handle_t adc_handle;
    SemaphoreHandle_t bufferMutex;

  public:
    Photodiode();
    void begin();
    void update();
    uint16_t convertToBits();
    float getDynamicThreshold();
    bool isBufferFull();
    bool isSampleBufferFull();
    float getSignalStrength();
};

#endif
