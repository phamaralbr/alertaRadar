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
} __attribute__((packed));

// Rolling buffer size and search radius
const int BUFFER_SIZE = 100;
double SEARCH_RADIUS = 10.0;  // 10 km search radius

// Buffer to store nearby radar data and its distance
struct RadarBuffer {
  Node node;
  float distance;
};

RadarBuffer rollingBuffer[BUFFER_SIZE];
int bufferCount = 0;  // Tracks number of nodes in buffer

// Haversine formula to calculate distance between two GPS coordinates (in km)
float haversine(float lon1, float lat1, float lon2, float lat2) {
  const float R = 6371.0;  // Earth radius in kilometers
  lon1 = radians(lon1);
  lat1 = radians(lat1);
  lon2 = radians(lon2);
  lat2 = radians(lat2);

  float dlon = lon2 - lon1;
  float dlat = lat2 - lat1;
  float a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
  return R * 2 * atan2(sqrt(a), sqrt(1 - a));
}

// Reads a node from the KD Tree binary file at the specified offset
Node read_node(File &file, uint32_t offset) {
  file.seek(offset);
  Node node;
  file.read((uint8_t*)&node, sizeof(Node));  // Read node data
  return node;
}

// Add a node to the buffer, replacing the farthest node if buffer is full
void addToBuffer(Node node, float distance) {
  if (bufferCount < BUFFER_SIZE) {
    rollingBuffer[bufferCount++] = {node, distance};
  } else {
    int farthestIndex = 0;
    for (int i = 1; i < BUFFER_SIZE; i++) {
      if (rollingBuffer[i].distance > rollingBuffer[farthestIndex].distance) {
        farthestIndex = i;
      }
    }
    if (distance < rollingBuffer[farthestIndex].distance) {
      rollingBuffer[farthestIndex] = {node, distance};
    }
  }
}

// Update the rolling buffer with nearby radar nodes from a binary file
void updateRollingBuffer(File &file, float current_lon, float current_lat, float max_distance_km) {
  unsigned long start_time_offset = millis();
  bufferCount = 0;  // Reset buffer

  const float delta_lat = max_distance_km / 111.0;  // Convert km to latitude degrees 
  
  uint32_t low = 0;
  uint32_t high = (file.size() / sizeof(Node)) - 1;  
  uint32_t mid; 

  Serial.println("Searching lat");
  while (low <= high) {
    mid = low + ((high - low) / 2);  // Avoid potential overflow from (low + high) / 2

    if (read_node(file, mid * sizeof(Node)).lat < current_lat - delta_lat) {
      low = mid + 1;  // Search in the upper half
    } 
    else {
      high = mid - 1;  // Search in the lower half
    }
  }
  


  Serial.printf("Time taken to find offset: %lu ms\n", millis() - start_time_offset);
  
  file.seek(low * sizeof(Node));

  // Process nodes within the search latitude range
  while (file.available()) {
    Node node = read_node(file, file.position());
    if (node.lat > current_lat + delta_lat) break;

    float distance = haversine(current_lon, current_lat, node.lon, node.lat);
    if (distance <= max_distance_km) {
      addToBuffer(node, distance);
    }
  }
}

// Sort the buffer by distance in ascending order
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

// Task to update the radar buffer and log nearby radars
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

    updateRollingBuffer(file, target_lon, target_lat, SEARCH_RADIUS);  // Update buffer with nearby radars
    sortBufferByDistance();  // Sort buffer by distance
    logRollingBuffer();  // Log sorted radar data

    Serial.printf("Time taken: %lu ms\n", millis() - start_time);
    file.close();
  }
}

// Task to update GPS data and display
void updateDisplayAndGPS(void *parameter) {
  unsigned long lastDisplayUpdate = 0;
  const int updateInterval = 300;  // Update every 300ms

  while (true) {
    decodeGPSData();
    float lat = getLatitude();
    float lon = getLongitude();
    float speed = getSpeed();
    float heading = getCourse();

    if (millis() - lastDisplayUpdate >= updateInterval) {
      lastDisplayUpdate = millis();
      if (speed != -1) {
        updateDisplay(lat, lon, heading, speed);  // Update display with GPS data
      } else {
        noGpsDisplay();  // Show "No GPS" if no GPS data
      }
    }
    delay(300);  // Consistent delay for display update
  }
}

void setup() {
  Serial.begin(115200);
  initDisplay();  // Initialize the display
  initGPS();  // Initialize GPS module

  if (!initializeSPIFFS()) return;  // Initialize SPIFFS for file storage

  // Create tasks for display and radar buffer updates
  xTaskCreatePinnedToCore(updateDisplayAndGPS, "UpdateDisplayAndGPS", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(checkRadars, "checkRadars", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // Main loop does nothing, tasks are handled in the background
}
