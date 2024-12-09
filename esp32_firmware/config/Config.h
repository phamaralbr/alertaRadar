#ifndef CONFIG_H
#define CONFIG_H

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C Address for OLED
#define SSD1306_I2C_ADDRESS 0x3C

// GPS Pins
#define RXD2 16 //To GPS TX
#define TXD2 17 //To GPS RX
#define GPS_SERIAL_PORT 2
#define GPS_BAUD_RATE 9600

#endif
