#ifndef I2C_SCANNER_HPP
#define I2C_SCANNER_HPP

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief I2C Scanner utility class
 * 
 * Scans the I2C bus and reports connected devices
 */
class I2CScanner {
public:
    /**
     * @brief Scan the I2C bus and print results to Serial
     * @param wire The Wire instance to use (default: Wire)
     * @return Number of devices found
     */
    static int scan(TwoWire& wire = Wire);
    
    /**
     * @brief Scan the I2C bus silently
     * @param wire The Wire instance to use (default: Wire)
     * @return Number of devices found
     */
    static int scanSilent(TwoWire& wire = Wire);
};

#endif // I2C_SCANNER_HPP
