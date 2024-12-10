#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "FS.h"
#include "SPIFFS.h"
#include "../config/Config.h"

#define TEST_API_URL "https://jsonplaceholder.typicode.com/posts/1"
// Function to download and save the file to SPIFFS
void downloadFile(HTTPClient& http) {
  File file = SPIFFS.open("/kd_tree.bin", "w");

  if (file) {
    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[128];
    int bytesRead;

    // Read the data and write it to the file
    while (http.connected() && (bytesRead = stream->read(buffer, sizeof(buffer))) > 0) {
      file.write(buffer, bytesRead);
    }

    file.close();
    Serial.println("File downloaded successfully.");
  } else {
    Serial.println("Failed to open file for writing.");
  }
}

// Function to check if the file has been updated and download it if necessary
void checkForUpdates() {

    // Report the total Flash memory size
  uint32_t flashSize = ESP.getFlashChipSize();
  Serial.print("Flash Size: ");
  Serial.print(flashSize / (1024 * 1024)); // Convert to MB
  Serial.println(" MB");

  // Check SPIFFS allocation
  Serial.print("SPIFFS Total Space: ");
  Serial.print(SPIFFS.totalBytes() / 1024);
  Serial.println(" KB");

  
  WiFiClientSecure client;
  client.setInsecure(); // Disable SSL verification (for testing purposes)

  HTTPClient http;

  Serial.println("Starting GET request...");
  http.begin(client, TEST_API_URL); // Initialize HTTPClient with secure client and URL

  http.setTimeout(10000); // Set timeout to 10 seconds

  // Perform the GET request
  int httpCode = http.GET();

  // Log the HTTP status code
  Serial.println("HTTP Response Code: " + String(httpCode));

  if (httpCode > 0) {
    // Log the response body
    String responseBody = http.getString();
    Serial.println("Response Body:");
    Serial.println(responseBody);

    // Log all response headers
    Serial.println("Response Headers:");
    for (int i = 0; i < http.headers(); i++) {
      String headerName = http.headerName(i);
      String headerValue = http.header(i);
      Serial.println(headerName + ": " + headerValue);
    }
  } else {
    // Log the error
    Serial.println("HTTP request failed. Error code: " + String(httpCode));
  }

  http.end(); // Clean up
}


// Initialize SPIFFS
bool initializeSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return false;
  }
  return true;
}
