#include "utils.hpp"
#include "config.h"

String Utils::toBinaryString(uint16_t value, int bits) {
    String binary = "";
    for (int i = bits - 1; i >= 0; i--) {
        binary += ((value >> i) & 1) ? "1" : "0";
        if (i > 0 && i % 4 == 0) {
            binary += " ";
        }
    }
    return binary;
}

void Utils::printSystemInfo() {
    Serial.println("ESP32-C3 Photodiode Message Decoder");
    Serial.println("====================================");
    Serial.println();
    
    Serial.println("ADC Configuration:");
    Serial.println("- Resolution: 12-bit (0-4095)");
    Serial.println("- Range: 0-3.3V");
    Serial.println("- Sample Rate: Every 100ms");
    Serial.println("- Buffer Size: 16 samples");
    Serial.println("- Protocol: 12 data bits + 4 hash bits");
    Serial.println("- Dynamic Threshold: Enabled");
    Serial.println();
    
    Serial.println("OLED Display:");
    Serial.print("- SDA: GPIO ");
    Serial.println(OLED_SDA);
    Serial.print("- SCL: GPIO ");
    Serial.println(OLED_SCL);
    Serial.println();
    
    Serial.println("Waiting for valid messages...");
    Serial.println();
}
