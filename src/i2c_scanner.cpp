#include "i2c_scanner.hpp"

int I2CScanner::scan(TwoWire& wire) {
    Serial.println("Scanning I2C bus...");
    
    int devicesFound = 0;
    
    for (byte address = 1; address < 127; address++) {
        wire.beginTransmission(address);
        byte error = wire.endTransmission();
        
        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println(" !");
            devicesFound++;
        } else if (error == 4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    
    if (devicesFound == 0) {
        Serial.println("No I2C devices found!");
    } else {
        Serial.print("Total I2C devices found: ");
        Serial.println(devicesFound);
    }
    Serial.println();
    
    return devicesFound;
}

int I2CScanner::scanSilent(TwoWire& wire) {
    int devicesFound = 0;
    
    for (byte address = 1; address < 127; address++) {
        wire.beginTransmission(address);
        byte error = wire.endTransmission();
        
        if (error == 0) {
            devicesFound++;
        }
    }
    
    return devicesFound;
}
