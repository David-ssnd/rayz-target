#ifndef UTILS_HPP
#define UTILS_HPP

#include <Arduino.h>

/**
 * @brief Utility functions for the target system
 */
namespace Utils {
    /**
     * @brief Convert a value to binary string representation
     * @param value The value to convert
     * @param bits Number of bits to display
     * @return Binary string with spaces every 4 bits
     */
    String toBinaryString(uint16_t value, int bits = 16);
    
    /**
     * @brief Print system configuration to Serial
     */
    void printSystemInfo();
}

#endif // UTILS_HPP
