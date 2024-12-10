#include <Arduino.h>
#include "GPSModule.h"
//#include "Compass.h"  // Uncomment and implement when ready
#include "Display.h"
#include "../config/Config.h"
#include "ota_update.h"
#include "WifiManager.h"

#include "FS.h"
#include "SPIFFS.h"
#include <math.h>


// Radar point structure (matches binary file format)
typedef struct {
    float lon;       // Longitude
    float lat;       // Latitude
    uint16_t type;   // Radar type
    uint16_t speed;  // Speed limit
    uint16_t dir_type;  // Direction type
    uint16_t direction; // Direction
} RadarPoint;

#define RADAR_FILE "/kd_tree.bin"  // Path to binary file in SPIFFS
#define RADAR_RANGE_METERS 100     // Search range in meters
#define EARTH_RADIUS 6371000       // Earth radius in meters

// Function to calculate Haversine distance between two points (in meters)
float haversineDistance(float lat1, float lon1, float lat2, float lon2) {
    float dLat = (lat2 - lat1) * M_PI / 180.0;
    float dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(lat1) * cos(lat2) * sin(dLon / 2) * sin(dLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return EARTH_RADIUS * c;
}

// Function to load and check radar points
void checkRadarInRange(float lon, float lat, uint16_t speed, uint16_t direction) {
    radarFile = SPIFFS.open(RADAR_FILE, "r");
    if (!radarFile) {
        Serial.println("Failed to open radar file.");
        return;
    }

    RadarPoint radar;
    bool radarFound = false;

    Serial.println("Checking for radars in range...");

    // Read radar points from the file
    while (radarFile.available()) {
        radarFile.readBytes((char*)&radar, sizeof(RadarPoint));

        // Calculate distance to the radar
        float distance = haversineDistance(lat, lon, radar.lat, radar.lon);

        // Check if radar is within range and matches direction
        if (distance <= RADAR_RANGE_METERS && radar.direction == direction) {
            Serial.println("Radar Found:");
            Serial.printf("Distance: %.2f meters, Speed Limit: %d, Direction: %d\n",
                          distance, radar.speed, radar.direction);
            radarFound = true;
        }
    }

    if (!radarFound) {
        Serial.println("No radar found in range.");
    }

    radarFile.close();
}







void setup() {
  Serial.begin(115200);
  initDisplay();
  initGPS();  // Initialize GPS module
  // initCompass();  // Uncomment when compass is ready

  // Initialize SPIFFS
  if (!initializeSPIFFS()) {
    return;
  }

  startWifiAttempt();

}

unsigned long lastDisplayUpdate = 0;  // Tracks the last update time
const int updateInterval = 300;  // Update every 300ms

void loop() {
  // manageWiFiConnection();
  decodeGPSData();  // Decode GPS data once per loop

  // Fetch GPS data
  float lat = getLatitude();
  float lon = getLongitude();
  float speed = getSpeed();
  float heading = getCourse();  // Replace with getCompassHeading() when ready

  // Check if it's time to update the display
  if (millis() - lastDisplayUpdate >= updateInterval) {
    lastDisplayUpdate = millis();  // Reset the timer
    
    if (speed != -1) {  // If GPS is providing data
      updateDisplay(lat, lon, heading, speed);

      // Check for radars in range
      checkRadarInRange(lon, lat, speed, heading);
      
    } else {  // GPS has no data
      noGpsDisplay();
    }
  }

  // Other code here can run without blocking
}

