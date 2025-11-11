#ifndef PHOTODIODE_HPP
#define PHOTODIODE_HPP

#include <Arduino.h>
#include "hash.h"
#include "config.h"

class Photodiode {
private:
    float sampleBuffer[SAMPLES_PER_BIT];
    int sampleIndex;
    
    float bitBuffer[PHOTODIODE_BUFFER_SIZE];
    int bitIndex;
    bool bufferFull;
    bool sampleBufferFull;
    
    float runningMin;
    float runningMax;
    float dynamicThreshold;
    
    unsigned long lastSampleTime;
    unsigned long bitStartTime;
    
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
