#include <Arduino.h>
#include "GPSModule.h"
#include "Display.h"
#include "../config/Config.h"
#include "ota_update.h"
#include "WifiManager.h"
#include "FS.h"
#include "SPIFFS.h"

// Node structure for radar data
struct Node {
  float lon, lat;
  uint8_t node_type, speed, direction_type;
  uint16_t direction;
  uint32_t left_offset, right_offset;
} __attribute__((packed));

// Reads a node from the KD Tree binary file
Node read_node(File &file, uint32_t offset) {
  file.seek(offset);
  Node node;
  file.read((uint8_t*)&node, sizeof(Node));
  return node;
}

// Haversine formula to calculate distance between two points
float haversine(float lon1, float lat1, float lon2, float lat2) {
  const float R = 6371.0; // Earth radius in kilometers
  lon1 = radians(lon1);
  lat1 = radians(lat1);
  lon2 = radians(lon2);
  lat2 = radians(lat2);

  float dlon = lon2 - lon1;
  float dlat = lat2 - lat1;
  float a = sin(dlat / 2) * sin(dlat / 2) +
            cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
  return R * 2 * atan2(sqrt(a), sqrt(1 - a));
}

// Rolling buffer for nearby radars
const int BUFFER_SIZE = 20;
struct RadarBuffer {
  Node node;
  float distance;  // Distance to the radar
};

RadarBuffer rollingBuffer[BUFFER_SIZE];
int bufferCount = 0;  // Number of nodes in the buffer

// Degree threshold for latitude and longitude
float degreeThreshold(float latitude, float distance_km) {
  const float EARTH_RADIUS_KM = 111.0;  // Approximate 1 degree of lat
  return distance_km / (EARTH_RADIUS_KM * cos(radians(latitude)));
}

// Find the farthest node in the buffer
int findFarthestNode() {
  int index = 0;
  float maxDistance = rollingBuffer[0].distance;
  for (int i = 1; i < BUFFER_SIZE; i++) {
    if (rollingBuffer[i].distance > maxDistance) {
      maxDistance = rollingBuffer[i].distance;
      index = i;
    }
  }
  return index;
}

// Add a node to the buffer
void addToBuffer(Node node, float distance) {
  if (bufferCount < BUFFER_SIZE) {
    rollingBuffer[bufferCount++] = {node, distance};
  } else {
    int farthestIndex = findFarthestNode();
    if (distance < rollingBuffer[farthestIndex].distance) {
      rollingBuffer[farthestIndex] = {node, distance};
    }
  }
}

// Update the rolling buffer with nearby radars
void updateRollingBuffer(File &file, float current_lon, float current_lat, float max_distance_km) {
  const float delta_lat = max_distance_km / 111.0;  
  const float delta_lon = degreeThreshold(current_lat, max_distance_km);  

  bufferCount = 0;  // Reset buffer
  file.seek(0);

  while (file.available() && bufferCount < BUFFER_SIZE) {
    Node node = read_node(file, file.position());
    if (fabs(node.lat - current_lat) < delta_lat && fabs(node.lon - current_lon) < delta_lon) {
      float distance = haversine(current_lon, current_lat, node.lon, node.lat);
      if (distance <= max_distance_km) {
        addToBuffer(node, distance);
      }
    }
  }
}

// Sort buffer by distance (ascending)
void sortBufferByDistance() {
  for (int i = 0; i < bufferCount - 1; i++) {
    for (int j = i + 1; j < bufferCount; j++) {
      if (rollingBuffer[i].distance > rollingBuffer[j].distance) {
        RadarBuffer temp = rollingBuffer[i];
        rollingBuffer[i] = rollingBuffer[j];
        rollingBuffer[j] = temp;
      }
    }
  }
}

// Log the contents of the rolling buffer
void logRollingBuffer() {
  Serial.println("=== Rolling Buffer ===");
  for (int i = 0; i < bufferCount; i++) {
    Serial.printf("Radar %d: Lat: %.6f, Lon: %.6f, Dist: %.2f km, Speed Limit: %d km/h, Direction: %d°, Type: %d\n", 
                  i + 1, rollingBuffer[i].node.lat, rollingBuffer[i].node.lon, rollingBuffer[i].distance, 
                  rollingBuffer[i].node.speed, rollingBuffer[i].node.direction, rollingBuffer[i].node.direction_type);
  }
  Serial.println("=====================");
}

// Task for radar buffer update and alert checking
void checkRadars(void *parameter) {
  while (true) {
    unsigned long start_time = millis();
    File file = SPIFFS.open("/radars.bin", "r");
    if (!file) {
      Serial.println("Failed to open radars file!");
      return;
    }

    float target_lon = -47.809293;
    float target_lat = -15.882209;

    updateRollingBuffer(file, target_lon, target_lat, 5.0);  // 5 km search radius
    sortBufferByDistance();
    logRollingBuffer();

    Serial.printf("Time taken: %lu ms\n", millis() - start_time);
    file.close();
  }
}

// Task for GPS and display updates
unsigned long lastDisplayUpdate = 0;
const int updateInterval = 300;  // Update every 300ms
void updateDisplayAndGPS(void *parameter) {
  while (true) {
    decodeGPSData();
    float lat = getLatitude();
    float lon = getLongitude();
    float speed = getSpeed();
    float heading = getCourse(); 

    if (millis() - lastDisplayUpdate >= updateInterval) {
      lastDisplayUpdate = millis();
      if (speed != -1) {
        updateDisplay(lat, lon, heading, speed);  // Update display
      } else {
        noGpsDisplay();  // Show "No GPS"
      }
    }
    delay(300);  // Delay to update display at a consistent rate
  }
}

void setup() {
  Serial.begin(115200);
  initDisplay();
  initGPS();

  if (!initializeSPIFFS()) return;  // Initialize SPIFFS

  // Create tasks for display and radar buffer updates
  xTaskCreatePinnedToCore(updateDisplayAndGPS, "UpdateDisplayAndGPS", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(checkRadars, "checkRadars", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // Main loop does nothing, tasks are handled in the background
}