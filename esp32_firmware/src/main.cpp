#include <Arduino.h>
#include "GPSModule.h"
//#include "Compass.h"  // Uncomment and implement when ready
#include "Display.h"
#include "../config/Config.h"
#include "ota_update.h"
#include "WifiManager.h"

#include "FS.h"
#include "SPIFFS.h"

struct Node {
  float lon;
  float lat;
  uint8_t node_type;
  uint8_t speed;
  uint8_t direction_type;
  uint16_t direction;
  uint32_t left_offset;
  uint32_t right_offset;
} __attribute__((packed));


Node read_node(File &file, uint32_t offset) {
  file.seek(offset);  // Seek to the correct position in the file
  
  Node node;
  file.read((uint8_t*)&node, sizeof(Node));  // Read 21 bytes (size of Node structure)
  
  return node;
}

float haversine(float lon1, float lat1, float lon2, float lat2) {
  const float R = 6371.0;  // Earth radius in kilometers
  lon1 = radians(lon1);
  lat1 = radians(lat1);
  lon2 = radians(lon2);
  lat2 = radians(lat2);
  
  float dlon = lon2 - lon1;
  float dlat = lat2 - lat1;
  float a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

Node best_node;
float best_distance = -1;

Node nearest_neighbor_search(File &file, float target_lon, float target_lat, uint32_t offset, int depth, bool is_root) {
  if (!is_root && offset == 0) {
    return best_node;  // If no child exists, return the best result found
  }

  // Read the current node
  Node node = read_node(file, offset);
  
  // If the node is invalid (e.g., end of tree or bad data), return the best node found
  if (node.left_offset == 0 && node.right_offset == 0) {
    return best_node;  // No further child nodes, stop the search
  }

  // Compute the distance from the target to the current node
  float current_distance = haversine(target_lon, target_lat, node.lon, node.lat);

  // Update the best result if the current node is closer
  if (best_distance == -1 || current_distance < best_distance) {
    best_node = node;
    best_distance = current_distance;
  }

  // Determine which subtree to search based on depth (longitude or latitude split)
  int axis = depth % 2;
  float target_value = (axis == 0) ? target_lon : target_lat;
  float node_value = (axis == 0) ? node.lon : node.lat;

  // Choose the primary and secondary subtrees to search
  uint32_t primary_offset = (target_value < node_value) ? node.left_offset : node.right_offset;
  uint32_t secondary_offset = (target_value < node_value) ? node.right_offset : node.left_offset;

  // Recursively search the primary subtree
  nearest_neighbor_search(file, target_lon, target_lat, primary_offset, depth + 1, false);

  // Check if we need to search the secondary subtree
  if (fabs(target_value - node_value) < best_distance) {
    nearest_neighbor_search(file, target_lon, target_lat, secondary_offset, depth + 1, false);
  }

  return best_node;
}


void checkRadars(){
  // Record the start time
  unsigned long start_time = millis();

  // Open the KD Tree binary file
  File file = SPIFFS.open("/radars.bin", "r");
  if (!file) {
      Serial.println("Failed to open radars file!");
      return;
  }

  float target_lon = -47.809293;
  float target_lat = -15.882209;

  // Start the search from the root of the KD Tree (offset 0)
  Node nearest = nearest_neighbor_search(file, target_lon, target_lat, 0, 0, true);

  // Output the nearest node's data
  Serial.printf("Nearest Node: Lon: %.6f, Lat: %.6f, Distance: %d meters\n", nearest.lon, nearest.lat, (int)(best_distance * 1000));
  
  Serial.printf("Time taken to find nearest node: %lu ms\n", millis() - start_time);

  file.close();  // Close the file after use
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

  // startWifiAttempt();

}

unsigned long lastDisplayUpdate = 0;  // Tracks the last update time
const int updateInterval = 300;  // Update every 300ms

void loop() {
  // manageWiFiConnection();
  Serial.println("============ Loop ============");
  decodeGPSData();  // Decode GPS data once per loop

  // Fetch GPS data
  float lat = getLatitude();
  float lon = getLongitude();
  float speed = getSpeed();
  float heading = getCourse();  // Replace with getCompassHeading() when ready

  // Check if it's time to update the display
  if (millis() - lastDisplayUpdate >= updateInterval) {
    lastDisplayUpdate = millis();  // Reset the timer

    checkRadars();
    
    if (speed != -1) {  // If GPS is providing data
      updateDisplay(lat, lon, heading, speed);

    } else {  // GPS has no data
      noGpsDisplay();
    }
  }

  // Other code here can run without blocking
}