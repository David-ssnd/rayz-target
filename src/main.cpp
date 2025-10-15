// Simple test for RayZ target
// Prints a message to Serial and blinks GPIO 2 (common on ESP32 dev boards)

#include <Arduino.h>

const int LED_PIN = 2; // use GPIO2 as a reliable built-in LED pin on many dev boards

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("RayZ target: Hello from ESP32!");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(250);
  Serial.println("tick");
}