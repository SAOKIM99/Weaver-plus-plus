#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <BikeDataDisplay.h>

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
BikeDataDisplay dashboard;

// Sample bike data
BikeData bike;

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

// Mô phỏng dữ liệu xe
void updateBikeData() {
  static unsigned long lastUpdate = 0;
  if(millis() - lastUpdate > 100) {
    
    // Mô phỏng tốc độ thay đổi
    static float targetSpeed = 25.0;
    if(random(100) < 5) {
      targetSpeed = random(0, 80);
    }
    bike.speed += (targetSpeed - bike.speed) * 0.1;
    
    // Pin giảm dần
    if(bike.speed > 10 && !bike.isCharging) {
      bike.batteryPercent -= 0.001;
      if(bike.batteryPercent < 0) bike.batteryPercent = 0;
      bike.battery1Percent -= 0.001;
      bike.battery2Percent -= 0.001;
    }
    
    // Cập nhật điện áp và dòng điện
    bike.batteryVoltage = 48.0 + (bike.batteryPercent - 50) * 0.1;
    bike.battery1Volt = 48.0 + (bike.battery1Percent - 50) * 0.1;
    bike.battery2Volt = 48.0 + (bike.battery2Percent - 50) * 0.1;
    
    // Mô phỏng current (cho phép âm khi sạc)
    static float detalCurrent = 1.5;
    bike.current = bike.current + detalCurrent;
    if (bike.current > 50)
    {
      detalCurrent = -1.4;
    }
    else if (bike.current < -50)
    {
      detalCurrent = 1.4;
    }

    bike.isCharging = (bike.current < 0); // Nếu current âm thì đang sạc
    
    // Xóa logic sạc (charging) - không còn cần thiết
    
    // Cập nhật khoảng cách
    bike.distance += bike.speed * 0.016667 / 60;  // km/h to km/frame
    bike.odometer += bike.speed * 0.016667 / 60;
    
    // Mô phỏng nhiệt độ
    float targetMotorTemp = 25 + (bike.speed * 0.7) + (bike.motorCurrent * 0.3);
    bike.motorTemp += (targetMotorTemp - bike.motorTemp) * 0.05;
    if(bike.motorTemp > 80) bike.motorTemp = 80;
    if(bike.motorTemp < 25) bike.motorTemp = 25;
    
    float targetEcuTemp = 20 + (bike.speed * 0.3) + random(-2, 3);
    bike.ecuTemp += (targetEcuTemp - bike.ecuTemp) * 0.03;
    if(bike.ecuTemp > 50) bike.ecuTemp = 50;
    if(bike.ecuTemp < 20) bike.ecuTemp = 20;
    
    bike.motorCurrent = bike.speed * 0.05 + bike.current * 0.8 + random(-2, 3) * 0.1;
    if(bike.motorCurrent < 0) bike.motorCurrent = 0;
    if(bike.motorCurrent > 10) bike.motorCurrent = 10;
    
    // Battery simulation
    bike.battery1Current = bike.current * 0.6 + random(-1, 2) * 0.1;
    bike.battery1Temp = 20 + (bike.battery1Current * 0.4) + (bike.speed * 0.1);
    if(bike.battery1Temp > 50) bike.battery1Temp = 50;
    if(bike.battery1Temp < 15) bike.battery1Temp = 15;
    bike.battery1DiffVolt = random(-20, 30) * 0.01;
    
    bike.battery2Current = bike.current * 0.4 + random(-1, 2) * 0.1;
    bike.battery2Temp = 18 + (bike.battery2Current * 0.5) + (bike.speed * 0.15);
    if(bike.battery2Temp > 50) bike.battery2Temp = 50;
    if(bike.battery2Temp < 15) bike.battery2Temp = 15;
    bike.battery2DiffVolt = random(-15, 20) * 0.01;
    
    // Mô phỏng kết nối Bluetooth bật/tắt mỗi 5 giây
    static unsigned long lastBluetoothToggle = 0;
    if(millis() - lastBluetoothToggle > 5000) {
      bike.bluetoothConnected = !bike.bluetoothConnected;
      lastBluetoothToggle = millis();
    }
    
    // Mô phỏng turn indicators - bật/tắt ngẫu nhiên mỗi 3 giây
    static unsigned long lastTurnToggle = 0;
    if(millis() - lastTurnToggle > 3000) {
      int turnState = random(0, 4);  // 0=off, 1=left, 2=right, 3=both
      bike.turnLeftActive = (turnState == 1 || turnState == 3);
      bike.turnRightActive = (turnState == 2 || turnState == 3);
      lastTurnToggle = millis();
    }
    
    lastUpdate = millis();
  }
}

void setup() {
  Serial.begin(115200);
  
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
    Serial.println("✅ Dashboard initialized successfully!");
    
    // Set Orbitron font for speed display
    // dashboard.setSpeedFont(&orbitron);
    
    // Load màn hình
    lv_scr_load(ui_main_screen);
  } else {
    Serial.println("❌ Failed to initialize dashboard!");
  }
  
  // Initialize bike data with default values
  bike.speed = 0;
  bike.current = 2.5;
  bike.batteryPercent = 85;
  bike.ecuTemp = 38;
  bike.motorTemp = 45;
  bike.motorCurrent = 4.3;
  bike.battery1Volt = 48.2;
  bike.battery1Percent = 85;
  bike.battery1DiffVolt = 0.2;
  bike.battery1Temp = 28;
  bike.battery1Current = 2.5;
  bike.battery2Volt = 47.8;
  bike.battery2Percent = 82;
  bike.battery2DiffVolt = -0.1;
  bike.battery2Temp = 31;
  bike.battery2Current = 1.8;
  bike.odometer = 1234.5;
  bike.bluetoothConnected = true;  // Khởi tạo Bluetooth connected
  bike.turnLeftActive = false;     // Khởi tạo turn indicators tắt
  bike.turnRightActive = false;
  
  Serial.println("🚴‍♂️ LVGL Electric Bike Dashboard with BikeDataDisplay Library initialized!");
}

void loop() {
  static unsigned long lastUpdate = 0;
  
  // Cập nhật dữ liệu
  updateBikeData();
  
  // Cập nhật dashboard mỗi 100ms
  if(millis() - lastUpdate > 100) {
    dashboard.updateAll(bike);
    lastUpdate = millis();
    
    // Debug info
    Serial.print("Speed: ");
    Serial.print(bike.speed, 1);
    Serial.print(" km/h, Current: ");
    Serial.print(bike.current, 1);
    Serial.print("A, Battery: ");
    Serial.print(bike.batteryPercent);
    Serial.println("%");
  }
  
  // Xử lý LVGL
  lv_timer_handler();
  delay(5);
}