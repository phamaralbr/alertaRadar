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

// Radar bin file
#define FILE_URL "https://raw.githubusercontent.com/phamaralbr/alertaRadar/main/data_preprocessing/data_output/kd_tree.bin"

// Wi-Fi credentials
#define WIFI_SSID "linksys"
#define WIFI_PASSWORD "f1a2c3a415"
#define WIFI_TIMEOUT 10000 // 10 seconds timeout for Wi-Fi connection

#endif
