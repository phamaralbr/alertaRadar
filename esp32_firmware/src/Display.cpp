#include "Display.h"

// Define the display object here (no `extern` keyword)
SSD1306Wire display(SSD1306_I2C_ADDRESS, SDA, SCL);  // Initialize display object with I2C address and SDA/SCL pins

bool dotVisible = false;  // Toggle state for the blinking dot
unsigned long lastUpdate = 0;  // Track time for blinking

void initDisplay() {
  display.init();
  display.clear();
  display.display();
}

void displayMessage(const char* message) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);  // Set font for message
  display.drawString(0, 0, message);
  display.display();
}

void updateDisplay(float lat, float lon, float heading, float speed) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);  // Set font for data

  // Display Latitude
  display.drawString(0, 0, "Lat: ");
  display.drawString(50, 0, String(lat, 6));

  // Display Longitude
  display.drawString(0, 16, "Lon: ");
  display.drawString(50, 16, String(lon, 6));

  // Display Speed
  display.drawString(0, 32, "Speed: ");
  display.drawString(60, 32, String(speed) + " km/h");

  // Display Heading
  display.drawString(0, 48, "Heading: ");
  display.drawString(70, 48, String(heading));

  // Blinking Dot Indicator
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= 500) {  // Toggle every 500ms
    dotVisible = !dotVisible;  // Switch state
    lastUpdate = currentMillis;
  }

  if (dotVisible) {
    display.setColor(WHITE);
    display.fillCircle(120, 56, 2);  // Draw a small dot at (120, 56) with radius 2
  }

  display.display();
}

void noGpsDisplay() {
    display.clear();

    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= 300) {  // Toggle every 300ms
      dotVisible = !dotVisible;  // Switch state
      lastUpdate = currentMillis;
    }

    if (dotVisible) {
      display.setColor(WHITE);
      display.drawXbm(0, 0, 16, 16, satellite_icon);
    }

    display.display();
}
