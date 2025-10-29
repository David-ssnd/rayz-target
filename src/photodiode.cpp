#include "photodiode.hpp"

Photodiode::Photodiode() {
  bufferIndex = 0;
  bufferFull = false;
  runningMin = 3.3;
  runningMax = 0.0;
}

void Photodiode::begin() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  for (int i = 0; i < PHOTODIODE_BUFFER_SIZE; i++) {
    voltageBuffer[i] = 0.0;
  }
}

void Photodiode::update() {
  int rawValue = analogRead(PHOTODIODE_PIN);
  float voltage = (rawValue * ADC_VREF) / ADC_RESOLUTION;
  
  if (voltage < runningMin) runningMin = voltage;
  if (voltage > runningMax) runningMax = voltage;
  
  voltageBuffer[bufferIndex] = voltage;
  bufferIndex = (bufferIndex + 1) % PHOTODIODE_BUFFER_SIZE;
  
  if (bufferIndex == 0 && !bufferFull) {
    bufferFull = true;
  }
}

uint16_t Photodiode::convertToBits() {
  if (!bufferFull) {
    return 0;
  }
  
  float threshold = getDynamicThreshold();
  uint16_t result = 0;
  
  for (int i = 0; i < PHOTODIODE_BUFFER_SIZE; i++) {
    int actualIndex = (bufferIndex + i) % PHOTODIODE_BUFFER_SIZE;
    
    result <<= 1;
    if (voltageBuffer[actualIndex] > threshold) {
      result |= 1;
    }
  }
  
  return result;
}

float Photodiode::getDynamicThreshold() {
  return (runningMin + runningMax) / 2.0;
}

bool Photodiode::isBufferFull() {
  return bufferFull;
}
