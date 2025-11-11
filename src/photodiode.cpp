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
  analogSetAttenuation(ADC_ATTENUATION);
  
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
  
  // Update min/max
  if (voltage < runningMin) {
    runningMin = runningMin * THRESHOLD_MIN_WEIGHT + voltage * THRESHOLD_NEW_WEIGHT;
  } else if (voltage > runningMax) {
    runningMax = runningMax * THRESHOLD_MIN_WEIGHT + voltage * THRESHOLD_NEW_WEIGHT;
  }
  
  sampleBuffer[sampleIndex] = voltage;
  sampleIndex++;
  
  if (sampleIndex >= SAMPLES_PER_BIT) {
    sampleBufferFull = true;

    // Average samples
    float avgVoltage = 0.0f;
    for (int i = 0; i < SAMPLES_PER_BIT; i++) {
      avgVoltage += sampleBuffer[i];
    }
    avgVoltage /= SAMPLES_PER_BIT;
    
    // Update threshold
    float midpoint = (runningMin + runningMax) * 0.5f;
    dynamicThreshold = dynamicThreshold * THRESHOLD_SMOOTH_OLD + midpoint * THRESHOLD_SMOOTH_NEW;
    
    if (bufferFull) {
      // Sliding window
      for (int i = 0; i < PHOTODIODE_BUFFER_SIZE - 1; i++) {
        bitBuffer[i] = bitBuffer[i + 1];
      }
      bitBuffer[PHOTODIODE_BUFFER_SIZE - 1] = avgVoltage;
    } else {
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

uint16_t Photodiode::convertToBits() {
  if (!bufferFull) {
    return 0;
  }

  sampleBufferFull = false;

  uint16_t result = 0;
  float threshold = dynamicThreshold;
  
  for (int i = 0; i < PHOTODIODE_BUFFER_SIZE; i++) {
    result <<= 1;
    if (bitBuffer[i] > threshold) {
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
