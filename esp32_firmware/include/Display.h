#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include "SSD1306Wire.h"
#include "../config/Config.h"

// Declare the display object as extern
extern SSD1306Wire display;  // Declare the display object here (extern)

// Function declarations
void initDisplay();
void displayMessage(const char* message);
void updateDisplay(float lat, float lon, float heading, float speed);
void noGpsDisplay();

// 'satellite-alt', 16x16px
const unsigned char satellite_icon [] PROGMEM = {
  0x00, 0x08, 0x00, 0x14, 0x00, 0x26, 0x00, 0x49, 0x00, 0x32, 0xc0, 0x16, 0xe0, 0x09, 0xe0, 0x03, 
	0xc8, 0x03, 0xb4, 0x51, 0x26, 0x50, 0x49, 0x48, 0x32, 0x26, 0x14, 0x10, 0x08, 0x0e, 0x00, 0x00
};

#endif
