#include <Arduino.h>
#include "GPSModule.h"
//#include "Compass.h"  // Uncomment and implement when ready
#include "Display.h"
#include "../config/Config.h"

void setup() {
  Serial.begin(115200);
  initDisplay();
  initGPS();  // Initialize GPS module
  // initCompass();  // Uncomment when compass is ready

  // displayMessage("Initializing...");

  // delay(200);
}

unsigned long lastDisplayUpdate = 0;  // Tracks the last update time
const int updateInterval = 300;  // Update every 300ms

void loop() {
  decodeGPSData();  // Decode GPS data once per loop

  // Fetch GPS data
  float lat = getLatitude();
  float lon = getLongitude();
  float speed = getSpeed();
  float heading = 0;  // Replace with getCompassHeading() when ready

  // Check if it's time to update the display
  if (millis() - lastDisplayUpdate >= updateInterval) {
    lastDisplayUpdate = millis();  // Reset the timer
    
    if (speed != -1) {  // If GPS is providing data
      updateDisplay(lat, lon, heading, speed);
    } else {  // GPS has no data
      noGpsDisplay();
    }
  }

  // Other code here can run without blocking
}
