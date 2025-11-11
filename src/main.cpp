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

uint16_t all_expected_messages = 0;
uint16_t correct_messages = 0;
uint16_t not_expected_messages = 0;

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
  photodiode.update();
  
  if (photodiode.isSampleBufferFull()) {
    uint16_t message16bit = photodiode.convertToBits();
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
    
    Serial.print(" | Sig: ");
    Serial.print(photodiode.getSignalStrength(), 3);
    Serial.print("V | Thr: ");
    Serial.print(photodiode.getDynamicThreshold(), 4);
    Serial.print("V");

    if (bleTarget.isConnected() && hasExpectedMessage && matchesBLE) {
      Serial.print(" | ✓ BLE MATCH");
      correct_messages++;
    } else {
      Serial.print(" | ✗ BLE MISMATCH");
      not_expected_messages++;
    }
    Serial.println();
    Serial.print("Stats: ");
    Serial.print(correct_messages);
    Serial.print("/");
    Serial.print(all_expected_messages);
    Serial.print(" | Incorrect: ");
    Serial.print(all_expected_messages - correct_messages);
    Serial.print(" | Not Expected: ");
    Serial.print(not_expected_messages);
    Serial.print(" | Accuracy: ");
    float accuracy = (all_expected_messages > 0) ? (correct_messages * 100.0f / all_expected_messages) : 0.0f;
    Serial.print(accuracy, 2);
    Serial.print("%");
    Serial.println();
  }
  
  bleTarget.update();
  
  if (bleTarget.hasMessage()) {
    all_expected_messages++;

    expectedMessage = bleTarget.getMessage();
    hasExpectedMessage = true;
    
    Serial.println();
    Serial.print("📡BLE | ");
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
}

