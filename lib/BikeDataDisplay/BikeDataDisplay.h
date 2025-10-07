#ifndef BIKE_DATA_DISPLAY_H
#define BIKE_DATA_DISPLAY_H

#include <Arduino.h>
#include <cstdio>
#include <lvgl.h>

// ================================================
// SCREEN CONFIGURATION DEFINES
// ================================================
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define SCREEN_CENTER_X (SCREEN_WIDTH / 2)   // 240
#define SCREEN_CENTER_Y (SCREEN_HEIGHT / 2)  // 136

// ================================================
// ARC CONFIGURATION DEFINES
// ================================================
// Speedometer Arc Configuration
#define SPEED_ARC_WIDTH     250
#define SPEED_ARC_HEIGHT    250
#define SPEED_ARC_X         (SCREEN_CENTER_X - SPEED_ARC_WIDTH/2)   // Centered horizontally
#define SPEED_ARC_Y         (SCREEN_CENTER_Y - SPEED_ARC_HEIGHT/2 + 30)  // Centered with offset
#define SPEED_ARC_RANGE     180
#define SPEED_ARC_ROTATION  135
#define SPEED_BG_START      0
#define SPEED_BG_END        180

// Current Arc Configuration  
#define CURRENT_ARC_WIDTH     250
#define CURRENT_ARC_HEIGHT    250
#define CURRENT_ARC_X         (SCREEN_CENTER_X - CURRENT_ARC_WIDTH/2)   // Centered horizontally
#define CURRENT_ARC_Y         (SCREEN_CENTER_Y - CURRENT_ARC_HEIGHT/2 + 30)  // Centered with offset
#define CURRENT_ARC_RANGE     90
#define CURRENT_ARC_ROTATION  315
#define CURRENT_BG_START      0
#define CURRENT_BG_END        90


// ================================================
// ARC RANGE CONFIGURATION DEFINES
// ================================================
// Speed Arc Range
#define SPEED_ARC_MIN         0        // Minimum speed value (km/h)
#define SPEED_ARC_MAX         80       // Maximum speed value (km/h)

// Current Arc Range  
#define CURRENT_ARC_MIN       0        // Minimum current value (A)
#define CURRENT_ARC_MAX       50       // Maximum current value (A)

// Current Charging Arc Range
#define CURRENT_CHARGING_ARC_MIN       0        // Minimum charging current value (A)
#define CURRENT_CHARGING_ARC_MAX       60       // Maximum charging current value (A)


// Speed Level Thresholds and Colors
#define SPEED_LOW_THRESH      25       // Speed low threshold (km/h) - <25: mức thấp
#define SPEED_MEDIUM_THRESH   60       // Speed medium threshold (km/h) - <60: mức trung bình
                                       // >=60: mức cảnh báo

#define SPEED_LOW_COLOR       lv_color_hex(0x48C785)  // Green - mức thấp
#define SPEED_MEDIUM_COLOR    lv_color_hex(0xF7B731)  // Yellow - mức trung bình  
#define SPEED_WARNING_COLOR   lv_color_hex(0xE55039)  // Red - mức cảnh báo

// Current Level Thresholds and Colors
#define CURRENT_LOW_THRESH    10       // Current low threshold (A) - <10A: mức thấp
#define CURRENT_MEDIUM_THRESH 20       // Current medium threshold (A) - <20A: mức trung bình
#define CURRENT_HIGH_THRESH   60       // Current high threshold (A) - <60A: mức cao
                                       // >=60A: cảnh báo

#define CURRENT_LOW_COLOR     lv_color_hex(0x48C785)  // Green - mức thấp
#define CURRENT_MEDIUM_COLOR  lv_color_hex(0x4A90E2)  // Blue - mức trung bình
#define CURRENT_HIGH_COLOR    lv_color_hex(0xF7B731)  // Yellow - mức cao
#define CURRENT_WARNING_COLOR lv_color_hex(0xE55039)  // Red - cảnh báo



// ================================================
// TEXT CONFIGURATION DEFINES
// ================================================
// Speed Display Text
#define SPEED_LABEL_X         0
#define SPEED_LABEL_Y         15
#define SPEED_LABEL_WIDTH     LV_SIZE_CONTENT
#define SPEED_LABEL_HEIGHT    LV_SIZE_CONTENT
#define SPEED_LABEL_ALIGN     LV_ALIGN_CENTER
#define SPEED_LABEL_FONT      &orbitron
#define SPEED_LABEL_TEXT_ALIGN LV_TEXT_ALIGN_CENTER

// Speed Unit Text
#define SPEED_UNIT_X          0
#define SPEED_UNIT_Y          55
#define SPEED_UNIT_WIDTH      LV_SIZE_CONTENT
#define SPEED_UNIT_HEIGHT     LV_SIZE_CONTENT
#define SPEED_UNIT_ALIGN      LV_ALIGN_CENTER
#define SPEED_UNIT_FONT       &lv_font_montserrat_10
#define SPEED_UNIT_TEXT_ALIGN LV_TEXT_ALIGN_CENTER

// Current Display Text
#define CURRENT_VALUE_X       0
#define CURRENT_VALUE_Y       115
#define CURRENT_VALUE_WIDTH   LV_SIZE_CONTENT
#define CURRENT_VALUE_HEIGHT  LV_SIZE_CONTENT
#define CURRENT_VALUE_ALIGN   LV_ALIGN_CENTER
#define CURRENT_VALUE_FONT    &lv_font_unscii_16
#define CURRENT_VALUE_TEXT_ALIGN LV_TEXT_ALIGN_CENTER

// Current Unit Text
#define CURRENT_UNIT_X        0
#define CURRENT_UNIT_Y        130
#define CURRENT_UNIT_WIDTH    LV_SIZE_CONTENT
#define CURRENT_UNIT_HEIGHT   LV_SIZE_CONTENT
#define CURRENT_UNIT_ALIGN    LV_ALIGN_CENTER
#define CURRENT_UNIT_FONT     &lv_font_unscii_8
#define CURRENT_UNIT_TEXT_ALIGN LV_TEXT_ALIGN_CENTER

// Battery Main Text
#define BATTERY_MAIN_X        0
#define BATTERY_MAIN_Y        7
#define BATTERY_MAIN_WIDTH    LV_SIZE_CONTENT
#define BATTERY_MAIN_HEIGHT   LV_SIZE_CONTENT
#define BATTERY_MAIN_ALIGN    LV_ALIGN_TOP_MID
#define BATTERY_MAIN_FONT     &lv_font_unscii_16
#define BATTERY_MAIN_TEXT_ALIGN LV_TEXT_ALIGN_CENTER

// Battery Title Text (BAT1, BAT2)
#define BATTERY_TITLE_WIDTH   110
#define BATTERY_TITLE_HEIGHT  20
#define BATTERY_TITLE_FONT    &lv_font_unscii_8
#define BATTERY_TITLE_TEXT_ALIGN LV_TEXT_ALIGN_RIGHT

// Battery Values Text (Voltage, Percent, Diff, Temp, Current)
#define BATTERY_VALUE_WIDTH   50
#define BATTERY_VALUE_HEIGHT  12
#define BATTERY_VALUE_FONT    &lv_font_montserrat_10
#define BATTERY_VALUE_TEXT_ALIGN LV_TEXT_ALIGN_RIGHT

// Battery Current Text (wider for current values)
#define BATTERY_CURRENT_WIDTH 110
#define BATTERY_CURRENT_HEIGHT 12
#define BATTERY_CURRENT_FONT  &lv_font_montserrat_10
#define BATTERY_CURRENT_TEXT_ALIGN LV_TEXT_ALIGN_RIGHT

// ECU Display Text
#define ECU_TITLE_X           360
#define ECU_TITLE_Y           15
#define ECU_TITLE_WIDTH       110
#define ECU_TITLE_HEIGHT      20
#define ECU_TITLE_FONT        &lv_font_unscii_8
#define ECU_TITLE_TEXT_ALIGN  LV_TEXT_ALIGN_RIGHT

#define ECU_VALUE_X           360
#define ECU_VALUE_Y           30
#define ECU_VALUE_WIDTH       110
#define ECU_VALUE_HEIGHT      20
#define ECU_VALUE_FONT        &lv_font_montserrat_10
#define ECU_VALUE_TEXT_ALIGN  LV_TEXT_ALIGN_RIGHT

// Motor Display Text
#define MOTOR_TITLE_X         355
#define MOTOR_TITLE_Y         65
#define MOTOR_TITLE_WIDTH     110
#define MOTOR_TITLE_HEIGHT    20
#define MOTOR_TITLE_FONT      &lv_font_unscii_8
#define MOTOR_TITLE_TEXT_ALIGN LV_TEXT_ALIGN_RIGHT

#define MOTOR_VALUE_WIDTH     50
#define MOTOR_VALUE_HEIGHT    15
#define MOTOR_VALUE_FONT      &lv_font_montserrat_10
#define MOTOR_VALUE_TEXT_ALIGN LV_TEXT_ALIGN_RIGHT

// Odometer Display Text
#define ODO_X                 10
#define ODO_Y                 -15
#define ODO_WIDTH             LV_SIZE_CONTENT
#define ODO_HEIGHT            LV_SIZE_CONTENT
#define ODO_ALIGN             LV_ALIGN_BOTTOM_LEFT
#define ODO_FONT              &lv_font_montserrat_14
#define ODO_TEXT_ALIGN        LV_TEXT_ALIGN_LEFT

// Bluetooth Icon (Bottom Right)
#define BLUETOOTH_ICON_X      100
#define BLUETOOTH_ICON_Y      7
#define BLUETOOTH_ICON_WIDTH  LV_SIZE_CONTENT
#define BLUETOOTH_ICON_HEIGHT LV_SIZE_CONTENT
#define BLUETOOTH_ICON_ALIGN  LV_ALIGN_TOP_MID
#define BLUETOOTH_ICON_FONT   &lv_font_montserrat_20
#define BLUETOOTH_ICON_TEXT_ALIGN LV_TEXT_ALIGN_CENTER

// Turn Indicators (Left & Right of Battery %)
#define TURN_LEFT_ICON_X      -50    // Bên trái battery %
#define TURN_LEFT_ICON_Y      7      // Cùng height với battery %
#define TURN_RIGHT_ICON_X     50     // Bên phải battery %  
#define TURN_RIGHT_ICON_Y     7      // Cùng height với battery %
#define TURN_ICON_WIDTH       LV_SIZE_CONTENT
#define TURN_ICON_HEIGHT      LV_SIZE_CONTENT
#define TURN_ICON_ALIGN       LV_ALIGN_TOP_MID
#define TURN_ICON_FONT        &lv_font_montserrat_20
#define TURN_ICON_TEXT_ALIGN  LV_TEXT_ALIGN_CENTER

// ================================================
// BIKE DATA STRUCTURE
// ================================================
struct BikeData {
  float speed = 0;
  int gear = 3;
  // Battery data
  float batteryLevel = 85.5;
  int batteryPercent = 85;
  float batteryVoltage = 48.2;
  int battery = 85;  // compatibility with old code
  float voltage = 48.2;  // compatibility
  float current = 2.5;
  bool isCharging = false;
  bool bluetoothConnected = true;  // Bluetooth connection status
  // Turn indicators
  bool turnLeftActive = false;    // Rẽ trái active
  bool turnRightActive = false;   // Rẽ phải active
  // Motor data
  int motorTemp = 45;        // Nhiệt độ động cơ (°C)
  int ecuTemp = 38;          // Nhiệt độ ECU (°C)
  float motorCurrent = 4.3;  // Dòng điện động cơ (A)
  int motorPower = 750;
  
  // Battery 1 data
  float battery1Volt = 48.2;    // Điện áp battery 1 (V)
  int battery1Percent = 85;     // % battery 1
  int battery1Temp = 28;        // Nhiệt độ battery 1 (°C)
  float battery1Current = 2.5;  // Dòng điện battery 1 (A)
  float battery1DiffVolt = 0.2; // Chênh lệch điện áp battery 1 (V)
  
  // Battery 2 data  
  float battery2Volt = 47.8;    // Điện áp battery 2 (V)
  int battery2Percent = 82;     // % battery 2
  int battery2Temp = 31;        // Nhiệt độ battery 2 (°C)
  float battery2Current = 1.8;  // Dòng điện battery 2 (A)
  float battery2DiffVolt = -0.1; // Chênh lệch điện áp battery 2 (V)
  // Distance data
  float odometer = 1234.5;
  float distance = 12.3;
  float tripDistance = 12.3;
  // Time data
  int time = 1800;
  int tripTime = 1800;
  int tripHours = 0;
  int tripMinutes = 30;
  float avgSpeed = 24.5;
};

// ================================================
// BIKE DASHBOARD CLASS
// ================================================
class BikeDataDisplay {
private:
  // UI Objects
  lv_obj_t *ui_main_screen;
  lv_obj_t *ui_speed_arc;
  lv_obj_t *ui_speed_label;
  lv_obj_t *ui_battery_text;
  lv_obj_t *ui_current_text;
  lv_obj_t *ui_current_label;
  lv_obj_t *ui_current_arc;
  
  // Temperature displays 
  lv_obj_t *ui_motor_temp_label;
  lv_obj_t *ui_ecu_temp_label;
  
  // Labels
  lv_obj_t *ui_ecu_label;
  lv_obj_t *ui_motor_label;
  lv_obj_t *ui_battery1_title;
  lv_obj_t *ui_battery2_title;
  
  // Motor parameters
  lv_obj_t *ui_motor_temp_value;
  lv_obj_t *ui_motor_current_value;
  
  // Battery1 parameters
  lv_obj_t *ui_bat1_volt_value;
  lv_obj_t *ui_bat1_percent_value;
  lv_obj_t *ui_bat1_diff_value;
  lv_obj_t *ui_bat1_temp_value;
  lv_obj_t *ui_bat1_current_value;
  
  // Battery2 parameters
  lv_obj_t *ui_bat2_volt_value;
  lv_obj_t *ui_bat2_percent_value;
  lv_obj_t *ui_bat2_diff_value;
  lv_obj_t *ui_bat2_temp_value;
  lv_obj_t *ui_bat2_current_value;
  
  // ODO display
  lv_obj_t *ui_odo_label;

  // Bluetooth icon
  lv_obj_t *ui_bluetooth_icon;

  // Turn indicators
  lv_obj_t *ui_turn_left_icon;
  lv_obj_t *ui_turn_right_icon;

  // Font reference
  const lv_font_t* speed_font;

  // Helper methods
  void createSpeedometer();
  void createCurrentDisplay();
  void createBatteryDisplay();
  void createECUDisplay(); 
  void createMotorDisplay();
  void createOdometer();
  void createBluetoothIcon();
  void createTurnIndicators();
  lv_color_t getColorByTemperature(int temp, int lowThresh, int highThresh);
  lv_color_t getColorByPercent(int percent, int lowThresh, int highThresh);

public:
  BikeDataDisplay();
  ~BikeDataDisplay();
  
  // Initialization
  bool initialize(lv_obj_t *parent_screen = NULL);
  void setSpeedFont(const lv_font_t* font);
  
  // Update methods - individual components
  void updateSpeed(float speed);
  void updateCurrent(float current);
  void updateBattery(int percent);
  void updateECU(int temperature);
  void updateMotor(int temperature, float current);
  void updateBattery1(float volt, int percent, float diffVolt, int temp, float current);
  void updateBattery2(float volt, int percent, float diffVolt, int temp, float current);
  void updateOdometer(float distance);
  void updateBluetooth(bool connected);
  void updateTurnIndicators(bool leftActive, bool rightActive);
  
  // Update method - all data at once
  void updateAll(const BikeData& data);
  
  // Theme and styling
  void setTheme(int themeId);
  void setSpeedRange(int maxSpeed);
  void setCurrentRange(int maxCurrent);
  
  // Get UI objects for custom styling
  lv_obj_t* getMainScreen() { return ui_main_screen; }
  lv_obj_t* getSpeedArc() { return ui_speed_arc; }
  lv_obj_t* getCurrentArc() { return ui_current_arc; }
};

#endif // BIKE_DATA_DISPLAY_H