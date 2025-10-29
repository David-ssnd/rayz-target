#include <Arduino.h>
#include <Wire.h>
#include "hash.h"
#include "config.h"
#include "photodiode.hpp"
#include "display.hpp"
#include "i2c_scanner.hpp"
#include "utils.hpp"

// Global instances
Photodiode photodiode;
Display display;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Print system information
  Utils::printSystemInfo();
  
  // Initialize I2C and scan for devices
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(100000);
  I2CScanner::scan(Wire);
  
  // Initialize peripherals
  photodiode.begin();
  display.begin();
}

void loop() {
  photodiode.update();
  
  if (photodiode.isBufferFull()) {
    uint16_t message16bit = photodiode.convertToBits();
    uint16_t dataValue = 0;
    bool isValid = validateMessage16bit(message16bit, &dataValue);
    
    if (isValid) {
      uint8_t receivedHash = message16bit & 0x0F;
      
      // Update display with received ID
      display.showID(dataValue);
      
      // Print detailed information to Serial
      Serial.println("========================================");
      Serial.print("✓ VALID MESSAGE RECEIVED at ");
      Serial.print(millis());
      Serial.println(" ms");
      Serial.println("========================================");
      Serial.print("Full 16-bit:  ");
      Serial.print(Utils::toBinaryString(message16bit, 16));
      Serial.print(" (");
      Serial.print(message16bit);
      Serial.println(")");
      Serial.print("Data (12bit): ");
      Serial.print(Utils::toBinaryString(dataValue, 12));
      Serial.print(" (");
      Serial.print(dataValue);
      Serial.println(")");
      Serial.print("Hash (4bit):  ");
      Serial.print(Utils::toBinaryString(receivedHash, 4));
      Serial.print(" (");
      Serial.print(receivedHash);
      Serial.println(")");
      Serial.print("Threshold:    ");
      Serial.print(photodiode.getDynamicThreshold(), 4);
      Serial.println(" V");
      Serial.println("========================================");
      Serial.println();
    }
  }
  
  delay(SAMPLE_INTERVAL);
}

