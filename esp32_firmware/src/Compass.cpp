// #include "Compass.h"
// #include <Adafruit_HMC5883_U.h>
// #include <Wire.h>

// Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

// void initCompass() {
//   if (!mag.begin()) {
//     Serial.println("Compass not detected. Check wiring!");
//     while (1);
//   }
// }

// float getCompassHeading() {
//   sensors_event_t event;
//   mag.getEvent(&event);
//   float heading = atan2(event.magnetic.y, event.magnetic.x);
//   heading = heading * 180 / PI;
//   if (heading < 0) heading += 360;
//   return heading;
// }
