#include <Arduino.h>
#include <Wire.h>
#include "hash.h"
#include "config.h"
#include "photodiode.hpp"
#include "utils.hpp"

// Global instances
Photodiode photodiode;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);
  
  Utils::printSystemInfo();
  
  photodiode.begin();
  
  Serial.println("Target device ready - waiting for signals...");
  Serial.println();
}

void loop() {
  photodiode.update();
  
  if (photodiode.isBufferFull()) {
    uint16_t message16bit = photodiode.convertToBits();
    uint16_t dataValue = 0;
    bool isValid = validateMessage16bit(message16bit, &dataValue);
    
    if (isValid) {
      digitalWrite(VIBRATION_PIN, HIGH);
      
      Serial.print("✓ ");
      Serial.print(millis());
      Serial.print(" ms | ");
      Serial.print(Utils::toBinaryString(message16bit, 16));
      Serial.print(" | Threshold: ");
      Serial.print(photodiode.getDynamicThreshold(), 4);
      Serial.println(" V");
      
      delay(VIBRATION_DURATION);
      digitalWrite(VIBRATION_PIN, LOW);
      delay(SAMPLE_INTERVAL - VIBRATION_DURATION);
    }
  }
  else {
    delay(SAMPLE_INTERVAL);
  }
}

