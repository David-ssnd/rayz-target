#include <Arduino.h>

#define PHOTODIODE_PIN 2  // GPIO2 (ADC1_CH2)
#define SAMPLE_INTERVAL 100  // milliseconds

// ESP32-C3 ADC parameters
const float ADC_VREF = 3.3;  // Reference voltage
const int ADC_RESOLUTION = 4095;  // 12-bit ADC (0-4095)

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32-C3 Photodiode Voltage Reader");
  Serial.println("===================================");
  
  // Configure ADC
  analogReadResolution(12);  // 12-bit resolution
  analogSetAttenuation(ADC_11db);  // 0-3.3V range
  
  Serial.println("ADC Configuration:");
  Serial.println("- Resolution: 12-bit (0-4095)");
  Serial.println("- Range: 0-3.3V");
  Serial.println("- Sample Rate: Every 100ms");
  Serial.println();
  Serial.println("Time(ms)\tRaw ADC\tVoltage(V)");
}

void loop() {
  // Read analog value
  int rawValue = analogRead(PHOTODIODE_PIN);
  
  // Convert to voltage
  float voltage = (rawValue * ADC_VREF) / ADC_RESOLUTION;
  
  // Print results
  Serial.print(millis());
  Serial.print("\t\t");
  Serial.print(rawValue);
  Serial.print("\t");
  Serial.println(voltage, 4);  // 4 decimal places
  
  delay(SAMPLE_INTERVAL);
}

/*
 * Optional: Add averaging for more stable readings
 * Uncomment the function below and call it instead of analogRead()
 */

/*
float readPhotodiodeAverage(int samples = 10) {
  long sum = 0;
  for(int i = 0; i < samples; i++) {
    sum += analogRead(PHOTODIODE_PIN);
    delayMicroseconds(100);
  }
  float avgRaw = sum / (float)samples;
  return (avgRaw * ADC_VREF) / ADC_RESOLUTION;
}
*/