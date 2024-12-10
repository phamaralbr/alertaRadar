#ifndef GPSMODULE_H
#define GPSMODULE_H

#include <HardwareSerial.h>
#include <TinyGPS++.h>  // Include TinyGPS++ header for GPS functionality

extern HardwareSerial GPSSerial;  // Declare external variable for GPS Serial

void initGPS();
void decodeGPSData();  // Separate function for GPS data decoding
float getLatitude();
float getLongitude();
float getSpeed();
float getCourse();

#endif
