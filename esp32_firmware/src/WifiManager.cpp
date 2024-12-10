#include <WiFi.h>
#include "ota_update.h"
#include "../config/Config.h"

// Wi-Fi connection variables
unsigned long wifiStartTime = 0;
bool wifiChecked = false;

void startWifiAttempt(){
    // Start Wi-Fi connection attempt
    Serial.println("Starting Wi-Fi connection attempt...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifiStartTime = millis();
}

void manageWiFiConnection() {

  if (wifiChecked) {
    return; // If already connected or timeout occurred, exit
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected successfully!");

    checkForUpdates();

    // Turn off Wi-Fi to save resources
    WiFi.disconnect();
    Serial.println("Wi-Fi disconnected to save resources.");
    wifiChecked = true; // Mark that Wi-Fi attempt is completed
    return;
  }

  // Check for timeout
  if (millis() - wifiStartTime > WIFI_TIMEOUT) {
    Serial.println("Wi-Fi connection timed out. Proceeding without Wi-Fi...");
    wifiChecked = true; // Stop further attempts
    return;
  }

}