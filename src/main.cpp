#include <Arduino.h>
#include "hash.h"

#define PHOTODIODE_PIN 2
#define SAMPLE_INTERVAL 100
#define BUFFER_SIZE PHOTODIODE_BUFFER_SIZE

const float ADC_VREF = 3.3;
const int ADC_RESOLUTION = 4095;

float voltageBuffer[BUFFER_SIZE];
int bufferIndex = 0;
bool bufferFull = false;

float runningMin = 3.3;
float runningMax = 0.0;
const float THRESHOLD_MARGIN = 0.1;

void updateDynamicThreshold(float voltage) {
  if (voltage < runningMin) runningMin = voltage;
  if (voltage > runningMax) runningMax = voltage;
}

float getDynamicThreshold() {
  float midpoint = (runningMin + runningMax) / 2.0;
  return midpoint;
}

uint16_t convertToBits() {
  if (!bufferFull) {
    return 0;
  }
  
  float threshold = getDynamicThreshold();
  uint16_t result = 0;
  
  for (int i = 0; i < BUFFER_SIZE; i++) {
    int actualIndex = (bufferIndex + i) % BUFFER_SIZE;
    
    result <<= 1;
    if (voltageBuffer[actualIndex] > threshold) {
      result |= 1;
    }
  }
  
  return result;
}

String toBinaryString(uint16_t value, int bits = 16) {
  String binary = "";
  for (int i = bits - 1; i >= 0; i--) {
    binary += ((value >> i) & 1) ? "1" : "0";
    if (i > 0 && i % 4 == 0) {
      binary += " ";
    }
  }
  return binary;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32-C3 Photodiode Message Decoder");
  Serial.println("====================================");
  
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  Serial.println("ADC Configuration:");
  Serial.println("- Resolution: 12-bit (0-4095)");
  Serial.println("- Range: 0-3.3V");
  Serial.println("- Sample Rate: Every 100ms");
  Serial.println("- Buffer Size: 16 samples");
  Serial.println("- Protocol: 12 data bits + 4 hash bits");
  Serial.println("- Dynamic Threshold: Enabled");
  Serial.println();
  Serial.println("Waiting for valid messages...");
  Serial.println();
  
  for (int i = 0; i < BUFFER_SIZE; i++) {
    voltageBuffer[i] = 0.0;
  }
}

void loop() {
  int rawValue = analogRead(PHOTODIODE_PIN);
  float voltage = (rawValue * ADC_VREF) / ADC_RESOLUTION;
  
  updateDynamicThreshold(voltage);
  
  voltageBuffer[bufferIndex] = voltage;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  
  if (bufferIndex == 0 && !bufferFull) {
    bufferFull = true;
  }
  
  if (bufferFull) {
    uint16_t message16bit = convertToBits();
    uint16_t dataValue = 0;
    bool isValid = validateMessage16bit(message16bit, &dataValue);
    
    if (isValid) {
      uint8_t receivedHash = message16bit & 0x0F;
      
      Serial.println("========================================");
      Serial.print("✓ VALID MESSAGE RECEIVED at ");
      Serial.print(millis());
      Serial.println(" ms");
      Serial.println("========================================");
      Serial.print("Full 16-bit:  ");
      Serial.print(toBinaryString(message16bit, 16));
      Serial.print(" (");
      Serial.print(message16bit);
      Serial.println(")");
      Serial.print("Data (12bit): ");
      Serial.print(toBinaryString(dataValue, 12));
      Serial.print(" (");
      Serial.print(dataValue);
      Serial.println(")");
      Serial.print("Hash (4bit):  ");
      Serial.print(toBinaryString(receivedHash, 4));
      Serial.print(" (");
      Serial.print(receivedHash);
      Serial.println(")");
      Serial.print("Threshold:    ");
      Serial.print(getDynamicThreshold(), 4);
      Serial.println(" V");
      Serial.println("========================================");
      Serial.println();
    }
  }
  
  delay(SAMPLE_INTERVAL);
}