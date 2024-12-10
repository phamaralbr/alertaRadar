#include "GPSModule.h"
#include "../config/Config.h"

HardwareSerial GPSSerial(GPS_SERIAL_PORT);  // Define GPS serial port
TinyGPSPlus gps;

void initGPS() {
  Serial.println("Starting GPS");
  GPSSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);  // Initialize GPS serial
}

void decodeGPSData() {
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());  // Decode GPS data
  }
}

float getLatitude() {
  return gps.location.isValid() ? gps.location.lat() : 0.0;
}

float getLongitude() {
  return gps.location.isValid() ? gps.location.lng() : 0.0;
}

float getSpeed() {
  return gps.speed.isValid() ? gps.speed.kmph() : -1.0;
}

float getCourse() {
  return gps.course.isValid() ? gps.course.deg() : -1.0;
}
