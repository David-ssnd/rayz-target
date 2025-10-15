// RayZ Target Device
// Using shared library for common protocol definitions

#include <Arduino.h>
#include "rayz_common.h"

const int LED_PIN = 2; // use GPIO2 as a reliable built-in LED pin on many dev boards

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  
  Serial.println("=== RayZ Target Device ===");
  Serial.printf("Protocol Version: %s\n", RAYZ_PROTOCOL_VERSION);
  Serial.printf("Build Version: %s\n", RAYZ_VERSION);
  Serial.printf("Device Type: TARGET (0x%02X)\n", DEVICE_TARGET);
  Serial.println("========================");
  
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Create a heartbeat message using shared protocol
  RayZMessage msg;
  msg.deviceType = DEVICE_TARGET;
  msg.messageType = MSG_HEARTBEAT;
  msg.payloadLength = 0;
  
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(250);
  
  Serial.printf("Target heartbeat - Type: 0x%02X, Msg: 0x%02X\n", 
                msg.deviceType, msg.messageType);
}