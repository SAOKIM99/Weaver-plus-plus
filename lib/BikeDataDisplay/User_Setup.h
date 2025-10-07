#define USER_SETUP_LOADED

// TFT display driver
#define SSD1963_480_DRIVER

// Parallel 8-bit interface
#define TFT_PARALLEL_8_BIT

// Color order
#define TFT_RGB_ORDER TFT_BGR

// Pin definitions for ESP32
#define TFT_CS   -1  // CS connected to GND (not used)
#define TFT_DC   15  // Data/Command pin
#define TFT_RST  33  // Reset pin
#define TFT_WR   32  // Write strobe pin
#define TFT_RD    2  // Read strobe pin

// Data pins D0-D7
#define TFT_D0   27
#define TFT_D1    5
#define TFT_D2   14
#define TFT_D3    4
#define TFT_D4   13
#define TFT_D5   16
#define TFT_D6   12
#define TFT_D7   17

// Display dimensions (adjust based on your actual display)
#define TFT_WIDTH  272
#define TFT_HEIGHT 480