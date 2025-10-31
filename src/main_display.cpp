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
#define THEME_SEQUENCE_TIMEOUT_MS     4000    // Timeout for click after long press (ms) - INCREASED to 4s
#define THEME_LONG_PRESS_DURATION_MS  600     // Minimum long press duration (ms) - REDUCED to 600ms for easy trigger
#define THEME_BUTTON_DEBOUNCE_MS      50      // OneButton hardware debounce (ms) - handled by library

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

// ================================================
// RTOS TASK CONFIGURATION
// ================================================
#define LVGL_TASK_PRIORITY            2       // LVGL rendering priority
#define CAN_TASK_PRIORITY             3       // CAN processing priority (higher)
#define UI_UPDATE_TASK_PRIORITY       2       // UI update priority
#define BUTTON_TASK_PRIORITY          1       // Button handling priority (lower)

#define LVGL_TASK_STACK_SIZE          4096    // Stack size for LVGL task
#define CAN_TASK_STACK_SIZE           3072    // Stack size for CAN task
#define UI_UPDATE_TASK_STACK_SIZE     3072    // Stack size for UI update task
#define BUTTON_TASK_STACK_SIZE        2048    // Stack size for button task

#define LVGL_TASK_CORE                1       // Run LVGL on core 1
#define CAN_TASK_CORE                 0       // Run CAN on core 0 
#define UI_UPDATE_TASK_CORE           1       // Run UI updates on core 1
#define BUTTON_TASK_CORE              0       // Run button handling on core 0

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
unsigned long lastLongPressEnd = 0;

// Bike data - will be updated via CAN
BikeDataDisplay bike;

// CAN connection status
bool canConnected = false;
unsigned long lastCANMessage = 0;

// RTOS Task Handles
TaskHandle_t lvglTaskHandle = NULL;
TaskHandle_t canTaskHandle = NULL;
TaskHandle_t uiUpdateTaskHandle = NULL;
TaskHandle_t buttonTaskHandle = NULL;

// Mutex for bike data protection
SemaphoreHandle_t bikeDataMutex = NULL;

// OneButton callback functions
void onThemeLongPressStop() {
  lastLongPressEnd = millis();
}

void onThemeClick() {
  // Check if this follows a long press within timeout
  if (millis() - lastLongPressEnd < THEME_SEQUENCE_TIMEOUT_MS) {
    // Quick click within timeout after long press - switch theme!
    int currentTheme = dashboard.getCurrentTheme();
    int newTheme = (currentTheme == 0) ? 1 : 0;
    dashboard.setTheme(newTheme);
    dashboard.flashScreen();
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
    
    // Parse the incoming CAN message with mutex protection
    if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        canManager.parseCANMessage(id, data, length, bike);
        xSemaphoreGive(bikeDataMutex);
    }
}

void checkCANConnection() {
    if (canConnected && (millis() - lastCANMessage > CAN_CONNECTION_TIMEOUT_MS)) {
        canConnected = false;
        Serial.println("⚠️  CAN connection lost!");
    }
}

// ================================================
// RTOS TASK FUNCTIONS
// ================================================

// Task 1: LVGL Handler - runs on Core 1
void lvglTask(void* parameter) {
    Serial.println("🎨 LVGL Task started on Core " + String(xPortGetCoreID()));
    
    for(;;) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));  // 5ms refresh rate
    }
}

// Task 2: CAN Processing - runs on Core 0 (high priority)
void canTask(void* parameter) {
    Serial.println("📡 CAN Task started on Core " + String(xPortGetCoreID()));
    
    for(;;) {
        canManager.update();
        checkCANConnection();
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms CAN polling
    }
}

// Task 3: UI Update - runs on Core 1
void uiUpdateTask(void* parameter) {
    Serial.println("🖥️  UI Update Task started on Core " + String(xPortGetCoreID()));
    
    for(;;) {
        // Copy bike data with mutex protection
        BikeDataDisplay localBike;
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            localBike = bike;
            xSemaphoreGive(bikeDataMutex);
        }
        
        // Update dashboard with local copy
        dashboard.updateAll(localBike);
        
        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_INTERVAL_MS));  // 100ms update rate
    }
}

// Task 4: Button Handling - runs on Core 0
void buttonTask(void* parameter) {
    Serial.println("🔘 Button Task started on Core " + String(xPortGetCoreID()));
    
    for(;;) {
        // Read passing button
        bool passingPressed = (digitalRead(COS_PIN) == HIGH);
        
        // Update bike data with mutex
        if (xSemaphoreTake(bikeDataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            bike.passingActive = passingPressed;
            xSemaphoreGive(bikeDataMutex);
        }
        
        // Process theme button
        themeButton.tick();
        
        vTaskDelay(pdMS_TO_TICKS(10));  // 10ms button polling
    }
}

// ================================================
// ARDUINO SETUP & LOOP
// ================================================

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
  themeButton.setPressMs(THEME_LONG_PRESS_DURATION_MS);
  themeButton.setDebounceMs(THEME_BUTTON_DEBOUNCE_MS);
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
  
  // ================================================
  // CREATE MUTEX & RTOS TASKS
  // ================================================
  
  // Create mutex for bike data protection
  bikeDataMutex = xSemaphoreCreateMutex();
  if (bikeDataMutex == NULL) {
    Serial.println("❌ Failed to create bike data mutex!");
    while(1); // Halt
  }
  Serial.println("✅ Bike data mutex created");
  
  // Create LVGL Task on Core 1 (display rendering)
  xTaskCreatePinnedToCore(
    lvglTask,              // Task function
    "LVGL_Task",           // Task name
    LVGL_TASK_STACK_SIZE,  // Stack size
    NULL,                  // Parameters
    LVGL_TASK_PRIORITY,    // Priority
    &lvglTaskHandle,       // Task handle
    LVGL_TASK_CORE         // Core ID
  );
  
  // Create CAN Task on Core 0 (high priority I/O)
  xTaskCreatePinnedToCore(
    canTask,
    "CAN_Task",
    CAN_TASK_STACK_SIZE,
    NULL,
    CAN_TASK_PRIORITY,
    &canTaskHandle,
    CAN_TASK_CORE
  );
  
  // Create UI Update Task on Core 1
  xTaskCreatePinnedToCore(
    uiUpdateTask,
    "UI_Update_Task",
    UI_UPDATE_TASK_STACK_SIZE,
    NULL,
    UI_UPDATE_TASK_PRIORITY,
    &uiUpdateTaskHandle,
    UI_UPDATE_TASK_CORE
  );
  
  // Create Button Task on Core 0
  xTaskCreatePinnedToCore(
    buttonTask,
    "Button_Task",
    BUTTON_TASK_STACK_SIZE,
    NULL,
    BUTTON_TASK_PRIORITY,
    &buttonTaskHandle,
    BUTTON_TASK_CORE
  );
  
  Serial.println("✅ All RTOS tasks created successfully!");
  Serial.println("📡 System running with FreeRTOS task scheduler");
  // Serial.println("📡 Waiting for CAN messages from main controller...");
  vTaskDelete(NULL);  // Delete Arduino loop task
}

void loop() {
  // Empty loop - all work is done by RTOS tasks
  // FreeRTOS scheduler handles everything
  // vTaskDelay(pdMS_TO_TICKS(1000));  // Yield to scheduler, wake up every 1s for watchdog
}
