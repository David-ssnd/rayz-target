#include <Arduino.h>
#include <Wire.h>
#include "hash.h"
#include "config.h"
#include "photodiode.hpp"
#include "utils.h"
#include "ble_target.h"

Photodiode photodiode;
BLETarget bleTarget;
uint16_t expectedMessage = 0;
bool hasExpectedMessage = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);
  
  photodiode.begin();
  
  bleTarget.begin();
  
  Serial.println("Target device ready - waiting for signals...");
  Serial.println();
}

void loop() {
  bleTarget.update();
  
  if (bleTarget.hasMessage()) {
    expectedMessage = bleTarget.getMessage();
    hasExpectedMessage = true;
    
    Serial.println();
    Serial.print(" 📡 BLE | ");
    Serial.print(millis());
    Serial.print(" ms | ");
    Serial.print(toBinaryString(expectedMessage, MESSAGE_TOTAL_BITS));
    Serial.print(" | Expected Data: ");
    uint16_t expectedData = 0;
    if (validateMessage16bit(expectedMessage, &expectedData)) {
      Serial.println(expectedData);
    } else {
      Serial.println("INVALID");
    }
  }
  
  photodiode.update();
  
  if (photodiode.isSampleBufferFull()) {
    uint16_t message16bit = photodiode.convertToBits(); // Sample buffer = false inside
    uint16_t dataValue = 0;
    bool isValid = validateMessage16bit(message16bit, &dataValue);
    
    if (!isValid) {
      return;
    }

    bool matchesBLE = false;
    if (hasExpectedMessage && bleTarget.isConnected()) {
      matchesBLE = (message16bit == expectedMessage);
    }
    
    Serial.print("Laser | ");
    Serial.print(millis());
    Serial.print(" ms | ");
    Serial.print(toBinaryString(message16bit, 16));
    Serial.print(" | Data: ");
    
    Serial.print(dataValue);
    Serial.print(" (0b");
    Serial.print(dataValue, BIN);
    Serial.print(")");
    digitalWrite(VIBRATION_PIN, HIGH);
    
    Serial.print(" | Sig: ");
    Serial.print(photodiode.getSignalStrength(), 3);
    Serial.print("V | Thr: ");
    Serial.print(photodiode.getDynamicThreshold(), 4);
    Serial.print("V");
    
    if (bleTarget.isConnected() && hasExpectedMessage) {
      Serial.print(matchesBLE ? " | ✓ BLE MATCH" : " | ✗ BLE MISMATCH");
      if (matchesBLE) {
        hasExpectedMessage = false;
      }
    }
    Serial.println();
    
    delay(VIBRATION_DURATION);
    digitalWrite(VIBRATION_PIN, LOW);
  }
}

