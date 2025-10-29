#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

/**
 * @brief OLED Display controller class
 * 
 * Manages the SSD1306 OLED display for showing target ID information
 */
class Display {
private:
    Adafruit_SSD1306 oled;
    
public:
    Display();
    
    /**
     * @brief Initialize the display
     * @return true if successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Display the received ID on screen
     * @param id The 12-bit ID value to display
     */
    void showID(uint16_t id);
    
    /**
     * @brief Show waiting message
     */
    void showWaiting();
    
    /**
     * @brief Show error message
     * @param message The error message to display
     */
    void showError(const char* message);
};

#endif // DISPLAY_HPP
