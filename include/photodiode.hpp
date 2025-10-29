#ifndef PHOTODIODE_HPP
#define PHOTODIODE_HPP

#include <Arduino.h>
#include "hash.h"
#include "config.h"

/**
 * @brief Photodiode signal processor class
 * 
 * Handles reading, buffering, and processing photodiode signals
 * to decode binary messages
 */
class Photodiode {
private:
    float voltageBuffer[PHOTODIODE_BUFFER_SIZE];
    int bufferIndex;
    bool bufferFull;
    float runningMin;
    float runningMax;
    
public:
    Photodiode();
    
    /**
     * @brief Initialize the photodiode ADC
     */
    void begin();
    
    /**
     * @brief Update buffer with new reading
     * Call this regularly to maintain the buffer
     */
    void update();
    
    /**
     * @brief Convert buffered voltages to 16-bit message
     * @return 16-bit value (12 data bits + 4 hash bits)
     */
    uint16_t convertToBits();
    
    /**
     * @brief Get the dynamic threshold voltage
     * @return Threshold voltage calculated from running min/max
     */
    float getDynamicThreshold();
    
    /**
     * @brief Check if buffer is full and ready
     * @return true if buffer contains valid data
     */
    bool isBufferFull();
};

#endif // PHOTODIODE_HPP
