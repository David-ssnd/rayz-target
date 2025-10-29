#include "display.hpp"
#include "utils.hpp"

Display::Display() : oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

bool Display::begin() {
  Serial.println("Initializing display...");
  
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(100000);
  
  Serial.print("SDA: GPIO ");
  Serial.println(OLED_SDA);
  Serial.print("SCL: GPIO ");
  Serial.println(OLED_SCL);
  
  delay(100);
  
  Serial.print("Trying address 0x");
  Serial.print(OLED_ADDRESS, HEX);
  Serial.print("...");
  
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(" FAILED!");
    Serial.println("Trying alternate address 0x3D...");
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("FAILED on both addresses!");
      Serial.println("\nPossible issues:");
      Serial.println("1. Wrong I2C address (not 0x3C or 0x3D)");
      Serial.println("2. Wiring problem");
      Serial.println("3. Display requires 5V instead of 3.3V");
      Serial.println("4. Defective display");
      return false;
    }
    Serial.println(" SUCCESS at 0x3D!");
  } else {
    Serial.println(" SUCCESS!");
  }
  
  Serial.println("Clearing display...");
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("RayZ Target");
  oled.println("Ready...");
  
  Serial.println("Updating display...");
  oled.display();
  Serial.println("Display initialized!");
  
  delay(2000);
  return true;
}

void Display::showID(uint16_t id) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("ID: ");
  oled.setTextSize(2);
  oled.println(id);
  
  oled.setTextSize(1);
  oled.println(Utils::toBinaryString(id, 12));
  
  oled.display();
}

void Display::showWaiting() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("Waiting...");
  oled.display();
}

void Display::showError(const char* message) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("ERROR:");
  oled.println(message);
  oled.display();
}
