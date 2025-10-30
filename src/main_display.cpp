#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <BikeDisplayUI.h>
#include <BikeCANManager.h>
#include "BikeDisplayHardware.h"
#include <OneButton.h>
#include "BLEDisplayManager.h"

// ================================================
// THEME SWITCHING CONFIGURATION DEFINES
// ================================================
#define THEME_SEQUENCE_TIMEOUT_MS     2000    // Timeout for click after long press (ms)
#define THEME_LONG_PRESS_DURATION_MS  1000    // Minimum long press duration (ms)

// ================================================
// CAN CONFIGURATION DEFINES
// ================================================
#define CAN_CONNECTION_TIMEOUT_MS     5000    // CAN connection lost timeout (ms)
#define CAN_CHECK_INTERVAL_MS         1000    // CAN status check interval (ms)

// ================================================
// DISPLAY UPDATE CONFIGURATION DEFINES
// ================================================
#define DISPLAY_UPDATE_INTERVAL_MS    100     // Dashboard update interval (ms)
#define CAN_STATS_INTERVAL_MS         10000   // CAN statistics print interval (ms)

// Khai báo TFT
TFT_eSPI tft = TFT_eSPI();

// LVGL display buffer - sử dụng defines từ thư viện BikeDataDisplay
static const uint32_t screenWidth = SCREEN_WIDTH;
static const uint32_t screenHeight = SCREEN_HEIGHT;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

// Main screen
lv_obj_t *ui_main_screen;

// BikeDataDisplay instance
BikeDisplayUI dashboard;

// CAN Manager instance
BikeCANManager canManager;

// BLE Display Manager instance
BLEDisplayManager bleDisplayManager;

// OneButton instance for theme switching
OneButton themeButton(COS_PIN, false); // Active HIGH

// Theme switching state
bool longPressCompleted = false;
unsigned long lastLongPressEnd = 0;

// Bike data - will be updated via CAN
BikeDataDisplay bike;

// CAN connection status
bool canConnected = false;
unsigned long lastCANMessage = 0;

// OneButton callback functions
void onThemeLongPressStop() {
  longPressCompleted = true;
  lastLongPressEnd = millis();
  // Serial.printf("[THEME] ✅ Long press COMPLETED - waiting for quick CLICK within %d ms\n", THEME_SEQUENCE_TIMEOUT_MS);
  // Serial.println("[THEME] 📋 Sequence: Long Press → Click (thả nút ra rồi nhấn nhanh)");
}

void onThemeClick() {
  // Serial.printf("[THEME] 🔘 Click detected, longPressCompleted=%d, timeDiff=%lu\n",
  //               longPressCompleted, millis() - lastLongPressEnd);
  
  // Check if this follows a long press within timeout
  if (longPressCompleted && (millis() - lastLongPressEnd) < THEME_SEQUENCE_TIMEOUT_MS) {
    // Quick click within 1 second after long press - switch theme!
    int currentTheme = dashboard.getCurrentTheme();
    int newTheme = (currentTheme == 0) ? 1 : 0;
    dashboard.setTheme(newTheme);
    dashboard.flashScreen();
    Serial.printf("[THEME] 🎨 Theme switched to %d\n", newTheme);
    // Serial.println("[THEME] ✅ SUCCESS: Theme changed!");
    longPressCompleted = false; // Reset
  } else {
    // Serial.println("[THEME] ❌ FAILED: Not a valid sequence or timeout");
  }
}

// LVGL flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t*)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// CAN receive callback function
void onCANMessage(uint32_t id, uint8_t* data, uint8_t length) {
    // Update last message time for connection status
    lastCANMessage = millis();
    canConnected = true;
    
    // Serial.printf("📨 [onCANMessage] Received ID=0x%03X, length=%d\n", id, length);
    
    // Parse the incoming CAN message
    if (canManager.parseCANMessage(id, data, length, bike)) {
        // Successfully parsed - log key data
        // switch(id) {
        //     case MSG_ID_BIKE_STATUS:
        //         Serial.printf("[CAN] Status: Speed=%.1f km/h, BT=%s, L=%s, R=%s\n",
        //                     bike.speed,
        //                     bike.bluetoothConnected ? "ON" : "OFF",
        //                     bike.turnLeftActive ? "ON" : "OFF",
        //                     bike.turnRightActive ? "ON" : "OFF");
        //         Serial.printf("📱 [CAN-RX] Bluetooth Status: bike.bluetoothConnected = %s\n", 
        //                     bike.bluetoothConnected ? "true" : "false");
        //         break;
                
        //     case MSG_ID_BMS_DATA + 1: // BMS1
        //         Serial.printf("[CAN] BMS1: %.2fV, %d%%, %.1f°C\n",
        //                     bike.battery1Volt,
        //                     bike.battery1Percent,
        //                     (float)bike.battery1Temp);
        //         break;
                
        //     case MSG_ID_BMS_DATA + 2: // BMS2
        //         Serial.printf("[CAN] BMS2: %.2fV, %d%%, %.1f°C\n",
        //                     bike.battery2Volt,
        //                     bike.battery2Percent,
        //                     (float)bike.battery2Temp);
        //         break;
                
        //     case MSG_ID_VESC_DATA:
        //         Serial.printf("[CAN] Motor: %.2fA, Motor=%.1f°C, ECU=%.1f°C\n",
        //                     bike.motorCurrent,
        //                     (float)bike.motorTemp,
        //                     (float)bike.ecuTemp);
        //         break;
                
        //     case MSG_ID_DISTANCE_DATA:
        //         Serial.printf("[CAN] Distance: Odo=%.1fkm, Trip=%.1fkm\n",
        //                     bike.odometer, bike.tripDistance);
        //         break;
        // }
    } else {
        // Serial.printf("[CAN] Parse failed for ID: 0x%03X\n", id);
    }
}

// Check CAN connection status
void checkCANConnection() {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > CAN_CHECK_INTERVAL_MS) {
        if (millis() - lastCANMessage > CAN_CONNECTION_TIMEOUT_MS) {
            if (canConnected) {
                canConnected = false;
                // Serial.println("[CAN] Connection lost");
            }
        }
        lastCheck = millis();
    }
}

void setup() {
  Serial.begin(115200);
  Serial.println("=== SAO KIM Display Controller ===");
  
  // Initialize CAN Manager first with display board pins
  if (canManager.begin(25, 26)) { // Display board CAN pins
    Serial.println("✅ CAN Manager initialized successfully");
    canManager.setReceiveCallback(onCANMessage);
    Serial.println("✅ CAN Receive callback registered");
  } else {
    Serial.println("❌ CAN Manager initialization failed");
    canConnected = false;
  }

  pinMode(COS_PIN, INPUT);
  // Serial.println("✅ Passing button input configured (COS_PIN)");
  
  // Initialize BLE Display Manager
  // Serial.println("🔧 Initializing BLE Display System...");
  bleDisplayManager.begin();
  bleDisplayManager.startAdvertising();
  // Serial.println("✅ BLE Display Manager initialized");
  
  // Setup OneButton callbacks for theme switching
  themeButton.attachLongPressStop(onThemeLongPressStop);
  themeButton.attachClick(onThemeClick);
  
  // Khởi tạo LCD
  tft.init();
  tft.setRotation(1); // Landscape 480x272
  delay(100);
  tft.fillScreen(TFT_BLACK);
  
  // Khởi tạo LVGL
  lv_init();
  
  // Khởi tạo display buffer
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);
  
  // Initialize display
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  
  // Tạo main screen
  ui_main_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_main_screen, LV_OBJ_FLAG_SCROLLABLE);
  
  // Initialize dashboard
  if (dashboard.initialize(ui_main_screen)) {
    // Serial.println("✅ Dashboard initialized successfully!");
    
    // Set Orbitron font for speed display
    // dashboard.setSpeedFont(&orbitron);
    
    // Load màn hình
    lv_scr_load(ui_main_screen);
  } else {
    Serial.println("❌ Failed to initialize dashboard!");
  }
  
  // Initialize bike data with default values
  bike.speed = 0;
  bike.current = 0;
  bike.batteryPercent = 0;
  bike.ecuTemp = 0;
  bike.motorTemp = 0;
  bike.motorCurrent = 0;
  bike.battery1Volt = 0;
  bike.battery1Percent = 0;
  bike.battery1DiffVolt = 0; // 200mV
  bike.battery1Temp = 0;
  bike.battery1Current = 0;
  bike.battery2Volt = 0;
  bike.battery2Percent = 0;
  bike.battery2DiffVolt = 0; // 100mV
  bike.battery2Temp = 0;
  bike.battery2Current = 0;
  bike.odometer = 0;
  bike.bluetoothConnected = false;  // Khởi tạo Bluetooth disconnected
  bike.turnLeftActive = false;     // Khởi tạo turn indicators tắt
  bike.turnRightActive = false;
  bike.passingActive = false;      // Khởi tạo passing indicator tắt
  
  Serial.println("🚴‍♂️ LVGL Electric Bike Dashboard with CAN Support initialized!");
  // Serial.println("📡 Waiting for CAN messages from main controller...");
}

void loop() {
  static unsigned long lastUpdate = 0;
  
  // Process incoming CAN messages
  canManager.update();
  
  // Check CAN connection status
  checkCANConnection();

  // Read passing button (active low)
  bool passingPressed = (digitalRead(COS_PIN) == HIGH);
  bike.passingActive = passingPressed;
  
  // Update OneButton for theme switching
  themeButton.tick();
  
  // Update BLE Display Manager
  // bleDisplayManager.update();
  
  // Reset long press state if timeout expired
  if (longPressCompleted && (millis() - lastLongPressEnd) >= THEME_SEQUENCE_TIMEOUT_MS) {
    longPressCompleted = false;
    // Serial.println("[THEME] Click timeout - reset");
  }
  
  // Cập nhật dashboard mỗi 100ms
  if(millis() - lastUpdate > DISPLAY_UPDATE_INTERVAL_MS) {
    dashboard.updateAll(bike);
    lastUpdate = millis();
    
    // Debug info với CAN status
    // Serial.printf("CAN:%s Speed:%.1f km/h Bat:%d%% Motor:%.1f°C BT:%s PASS:%s\n",
    //               canConnected ? "OK" : "LOST",
    //               bike.speed,
    //               bike.batteryPercent,
    //               (float)bike.motorTemp,
    //               bike.bluetoothConnected ? "ON" : "OFF",
    //               bike.passingActive ? "ON" : "OFF");
  }
  
  // Print CAN statistics every 10 seconds
  static unsigned long lastStats = 0;
  if (millis() - lastStats > CAN_STATS_INTERVAL_MS) {
    // Serial.printf("[STATS] CAN Messages Received: %d\n", canManager.getMessagesReceived());
    lastStats = millis();
  }
  
  // Xử lý LVGL
  lv_timer_handler();
  delay(5);
}
