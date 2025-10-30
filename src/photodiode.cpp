#include "photodiode.hpp"

Photodiode::Photodiode() {
  sampleIndex = 0;
  bitIndex = 0;
  bufferFull = false;
  sampleBufferFull = false;
  runningMin = 3.3f;
  runningMax = 0.0f;
  dynamicThreshold = 1.65f;
  lastSampleTime = 0;
  bitStartTime = 0;
}

void Photodiode::begin() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  for (int i = 0; i < SAMPLES_PER_BIT; i++) {
    sampleBuffer[i] = 0.0f;
  }
  for (int i = 0; i < PHOTODIODE_BUFFER_SIZE; i++) {
    bitBuffer[i] = 0.0f;
  }
  
  lastSampleTime = millis();
  bitStartTime = millis();
}

void Photodiode::update() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }
  lastSampleTime = currentTime;
  
  int rawValue = analogRead(PHOTODIODE_PIN);
  float voltage = (rawValue * ADC_VREF) / ADC_RESOLUTION;
  
  runningMin = min(runningMin * 0.99f + voltage * 0.01f, voltage);
  runningMax = max(runningMax * 0.99f + voltage * 0.01f, voltage);
  updateThreshold();
  
  sampleBuffer[sampleIndex] = voltage;
  sampleIndex++;
  
  if (sampleIndex >= SAMPLES_PER_BIT) {
    sampleBufferFull = true;

    float sum = 0.0f;
    for (int i = 0; i < SAMPLES_PER_BIT; i++) {
      sum += sampleBuffer[i];
    }
    float avgVoltage = sum / SAMPLES_PER_BIT;
    
    if (bufferFull) {
      // Sliding window: shift all values left and add new value at the end
      for (int i = 0; i < PHOTODIODE_BUFFER_SIZE - 1; i++) {
        bitBuffer[i] = bitBuffer[i + 1];
      }
      bitBuffer[PHOTODIODE_BUFFER_SIZE - 1] = avgVoltage;
    } else {
      // Still filling the buffer for the first time
      bitBuffer[bitIndex] = avgVoltage;
      bitIndex++;
      
      if (bitIndex >= PHOTODIODE_BUFFER_SIZE) {
        bufferFull = true;
      }
    }
    
    sampleIndex = 0;
    bitStartTime = currentTime;
  }
}

void Photodiode::updateThreshold() {
  float midpoint = (runningMin + runningMax) / 2.0f;
  dynamicThreshold = dynamicThreshold * 0.8f + midpoint * 0.2f;
}

uint16_t Photodiode::convertToBits() {
  if (!bufferFull) {
    return 0;
  }

  sampleBufferFull = false;

  uint16_t result = 0;
  for (int i = 0; i < PHOTODIODE_BUFFER_SIZE; i++) {
    result <<= 1;
    if (bitBuffer[i] > dynamicThreshold) {
      result |= 1;
    }
  }
  
  return result;
}

float Photodiode::getDynamicThreshold() {
  return dynamicThreshold;
}

bool Photodiode::isBufferFull() {
  return bufferFull;
}

bool Photodiode::isSampleBufferFull() {
  return sampleBufferFull;
}

float Photodiode::getSignalStrength() {
  return runningMax - runningMin;
}
