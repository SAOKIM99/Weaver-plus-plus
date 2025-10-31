#include "BikeDisplayUI.h"
#include <cstdio>
// External font - include the font file
#include "font/orbitron_num_80.c"

// Color themes - same as original

#define THEME_DARK 0
#define THEME_LIGHT 1

// Default theme
static int CURRENT_THEME = THEME_DARK;

// Theme 0: Minimalist Dark Theme (Original)
#define UI_COLOR_BG_DARK         lv_color_hex(0x000000)
#define UI_COLOR_PANEL_DARK      lv_color_hex(0x1A1E23)
#define UI_COLOR_ACCENT_DARK     lv_color_hex(0x4A90E2)
#define UI_COLOR_SUCCESS_DARK    lv_color_hex(0x48C785)
#define UI_COLOR_WARNING_DARK    lv_color_hex(0xF7B731)
#define UI_COLOR_DANGER_DARK     lv_color_hex(0xE55039)
#define UI_COLOR_ARC_BG_DARK     lv_color_hex(0x1A1E23)

// Theme 1: Light Theme (Dark mode: white bg, black text)
#define UI_COLOR_BG_LIGHT         lv_color_hex(0xFFFFFF)
#define UI_COLOR_PANEL_LIGHT      lv_color_hex(0xFFFFFF)
#define UI_COLOR_ACCENT_LIGHT     lv_color_hex(0x000000)
#define UI_COLOR_SUCCESS_LIGHT    lv_color_hex(0x000000)
#define UI_COLOR_WARNING_LIGHT    lv_color_hex(0x000000)
#define UI_COLOR_DANGER_LIGHT     lv_color_hex(0x000000)
#define UI_COLOR_ARC_BG_LIGHT     lv_color_hex(0xFFFFFF)

// Macros to get color by theme
#define UI_COLOR_BG         (CURRENT_THEME == THEME_DARK ? UI_COLOR_BG_DARK : UI_COLOR_BG_LIGHT)
#define UI_COLOR_PANEL      (CURRENT_THEME == THEME_DARK ? UI_COLOR_PANEL_DARK : UI_COLOR_PANEL_LIGHT)
#define UI_COLOR_ACCENT     (CURRENT_THEME == THEME_DARK ? UI_COLOR_ACCENT_DARK : UI_COLOR_ACCENT_LIGHT)
#define UI_COLOR_SUCCESS    (CURRENT_THEME == THEME_DARK ? UI_COLOR_SUCCESS_DARK : UI_COLOR_SUCCESS_LIGHT)
#define UI_COLOR_WARNING    (CURRENT_THEME == THEME_DARK ? UI_COLOR_WARNING_DARK : UI_COLOR_WARNING_LIGHT)
#define UI_COLOR_DANGER     (CURRENT_THEME == THEME_DARK ? UI_COLOR_DANGER_DARK : UI_COLOR_DANGER_LIGHT)
#define UI_COLOR_ARC_BG     (CURRENT_THEME == THEME_DARK ? UI_COLOR_ARC_BG_DARK : UI_COLOR_ARC_BG_LIGHT)

// Constructor
BikeDisplayUI::BikeDisplayUI() {
  ui_main_screen = NULL;
  ui_speed_arc = NULL;
  ui_speed_label = NULL;
  ui_speed_unit_label = NULL;
  ui_battery_text = NULL;
  ui_current_text = NULL;
  ui_current_label = NULL;
  ui_current_arc = NULL;
  ui_motor_temp_label = NULL;
  ui_ecu_temp_label = NULL;
  ui_ecu_label = NULL;
  ui_motor_label = NULL;
  ui_battery1_title = NULL;
  ui_battery2_title = NULL;
  ui_motor_temp_value = NULL;
  ui_motor_current_value = NULL;
  ui_bat1_volt_value = NULL;
  ui_bat1_percent_value = NULL;
  ui_bat1_diff_value = NULL;
  ui_bat1_temp_value = NULL;
  ui_bat1_current_value = NULL;
  ui_bat2_volt_value = NULL;
  ui_bat2_percent_value = NULL;
  ui_bat2_diff_value = NULL;
  ui_bat2_temp_value = NULL;
  ui_bat2_current_value = NULL;
  ui_odo_label = NULL;
  ui_bluetooth_icon = NULL;
  ui_turn_left_icon = NULL;
  ui_turn_right_icon = NULL;
  ui_passing_label = NULL;
  speed_font = NULL;
}

// Destructor
BikeDisplayUI::~BikeDisplayUI() {
  // LVGL objects are managed by the LVGL library
  // No manual cleanup needed
}

// Initialize the display
bool BikeDisplayUI::initialize(lv_obj_t *parent_screen) {
  if (parent_screen == NULL) {
    ui_main_screen = lv_scr_act();
  } else {
    ui_main_screen = parent_screen;
  }
  
  // Set background style
  lv_obj_set_style_bg_color(ui_main_screen, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  // Create all UI elements
  createSpeedometer();
  createCurrentDisplay(); 
  createBatteryDisplay();
  createECUDisplay();
  createMotorDisplay();
  createOdometer();
  createBluetoothIcon();
  createParkingIcon();
  createTurnIndicators();
  createPassingIndicator();
  
  // Apply default theme colors to all elements
  applyThemeToAllElements();
  
  return true;
}

// Set speed font
void BikeDisplayUI::setSpeedFont(const lv_font_t* font) {
  speed_font = font;
  if (ui_speed_label && font) {
    lv_obj_set_style_text_font(ui_speed_label, font, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

// Create speedometer - khôi phục layout gốc
void BikeDisplayUI::createSpeedometer() {
  /* Speed arc - L shape (từ góc dưới trái → lên trái → sang phải trên) */
  ui_speed_arc = lv_arc_create(ui_main_screen);
  lv_obj_set_size(ui_speed_arc, SPEED_ARC_WIDTH, SPEED_ARC_HEIGHT);
  lv_obj_set_pos(ui_speed_arc, SPEED_ARC_X, SPEED_ARC_Y);
  lv_arc_set_range(ui_speed_arc, SPEED_ARC_MIN, SPEED_ARC_MAX);
  lv_arc_set_value(ui_speed_arc, 35);  // Default speed
  
  // Cấu hình góc và rotation cho hình chữ L
  lv_arc_set_angles(ui_speed_arc, SPEED_BG_START, SPEED_ARC_RANGE);
  lv_arc_set_rotation(ui_speed_arc, SPEED_ARC_ROTATION);
  lv_arc_set_bg_angles(ui_speed_arc, SPEED_BG_START, SPEED_BG_END);
  
  // Style cho background arc (để hiển thị track)
  lv_obj_set_style_arc_color(ui_speed_arc, lv_color_hex(0x1A1E23), LV_PART_MAIN);
  lv_obj_set_style_arc_width(ui_speed_arc, 16, LV_PART_MAIN);
  lv_obj_set_style_arc_opa(ui_speed_arc, 255, LV_PART_MAIN);
  
  // Style cho indicator arc (phần active)
  lv_obj_set_style_arc_color(ui_speed_arc, lv_color_hex(0x4ECDC4), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(ui_speed_arc, 16, LV_PART_INDICATOR);
  lv_obj_set_style_arc_opa(ui_speed_arc, 255, LV_PART_INDICATOR);
  
  // Giữ bo cong ở 2 đầu arc (rounded ends)
  lv_obj_set_style_arc_rounded(ui_speed_arc, true, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(ui_speed_arc, true, LV_PART_MAIN);
  
  // Loại bỏ knob
  lv_obj_remove_style(ui_speed_arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(ui_speed_arc, LV_OBJ_FLAG_CLICKABLE);

  // Large speed number in center
  ui_speed_label = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_speed_label, "35");
  lv_obj_set_style_text_color(ui_speed_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  if (speed_font) {
    lv_obj_set_style_text_font(ui_speed_label, speed_font, LV_PART_MAIN | LV_STATE_DEFAULT);
  } else {
    lv_obj_set_style_text_font(ui_speed_label, SPEED_LABEL_FONT, LV_PART_MAIN);
  }
  lv_obj_set_style_text_align(ui_speed_label, SPEED_LABEL_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_align(ui_speed_label, SPEED_LABEL_ALIGN, SPEED_LABEL_X, SPEED_LABEL_Y);
  
  // KM/H unit text below speed
  ui_speed_unit_label = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_speed_unit_label, "Km/h");
  lv_obj_set_style_text_color(ui_speed_unit_label, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_speed_unit_label, SPEED_UNIT_FONT, LV_PART_MAIN);
  lv_obj_align(ui_speed_unit_label, SPEED_UNIT_ALIGN, SPEED_UNIT_X, SPEED_UNIT_Y);
}

// Create current display - khôi phục layout gốc  
void BikeDisplayUI::createCurrentDisplay() {
  /* Current arc */
  ui_current_arc = lv_arc_create(ui_main_screen);
  lv_obj_set_size(ui_current_arc, CURRENT_ARC_WIDTH, CURRENT_ARC_HEIGHT);
  lv_obj_set_pos(ui_current_arc, CURRENT_ARC_X, CURRENT_ARC_Y);
  lv_arc_set_range(ui_current_arc, CURRENT_ARC_MIN, CURRENT_ARC_MAX);
  lv_arc_set_value(ui_current_arc, 5);  // Default current
  
  // Cấu hình góc và rotation
  lv_arc_set_angles(ui_current_arc, CURRENT_BG_START, CURRENT_ARC_RANGE);
  lv_arc_set_rotation(ui_current_arc, CURRENT_ARC_ROTATION);
  lv_arc_set_bg_angles(ui_current_arc, CURRENT_BG_START, CURRENT_BG_END);
  lv_arc_set_mode(ui_current_arc, LV_ARC_MODE_REVERSE);
  
  // Style cho background arc (để hiển thị track)
  lv_obj_set_style_arc_color(ui_current_arc, lv_color_hex(0x1A1E23), LV_PART_MAIN);
  lv_obj_set_style_arc_width(ui_current_arc, 16, LV_PART_MAIN);
  lv_obj_set_style_arc_opa(ui_current_arc, 255, LV_PART_MAIN);
  
  // Style cho indicator arc (phần active)
  lv_obj_set_style_arc_color(ui_current_arc, lv_color_hex(0x4ECDC4), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(ui_current_arc, 16, LV_PART_INDICATOR);
  lv_obj_set_style_arc_opa(ui_current_arc, 255, LV_PART_INDICATOR);
  
  // Giữ bo cong ở 2 đầu arc (rounded ends)
  lv_obj_set_style_arc_rounded(ui_current_arc, true, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(ui_current_arc, true, LV_PART_MAIN);
  
  // Loại bỏ knob
  lv_obj_remove_style(ui_current_arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(ui_current_arc, LV_OBJ_FLAG_CLICKABLE);

  ui_current_text = lv_label_create(ui_main_screen);
  char cur_text[10];
  sprintf(cur_text, "%.1f", 2.5f);
  lv_label_set_text(ui_current_text, cur_text);
  lv_obj_set_style_text_color(ui_current_text, lv_color_hex(0x00D4AA), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_current_text, CURRENT_VALUE_FONT, LV_PART_MAIN);
  lv_obj_align(ui_current_text, CURRENT_VALUE_ALIGN, CURRENT_VALUE_X, CURRENT_VALUE_Y);
  
  // Current Unit "A" - ở bên cạnh current value
  ui_current_label = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_current_label, "A");
  lv_obj_set_style_text_color(ui_current_label, lv_color_hex(0x9E9E9E), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_current_label, CURRENT_UNIT_FONT, LV_PART_MAIN);
  lv_obj_align(ui_current_label, CURRENT_UNIT_ALIGN, CURRENT_UNIT_X, CURRENT_UNIT_Y);
}

// Create battery display - khôi phục layout gốc
void BikeDisplayUI::createBatteryDisplay() {
  // Battery percentage ở giữa trên
  ui_battery_text = lv_label_create(ui_main_screen);
  char bat_text[10];
  sprintf(bat_text, "%d%%", 85);
  lv_label_set_text(ui_battery_text, bat_text);
  lv_obj_set_style_text_color(ui_battery_text, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_battery_text, BATTERY_MAIN_FONT, LV_PART_MAIN);
  lv_obj_align(ui_battery_text, BATTERY_MAIN_ALIGN, BATTERY_MAIN_X, BATTERY_MAIN_Y);

  // Battery 1 Label "BAT1" riêng - căn lề phải, phía trên
  ui_battery1_title = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_battery1_title, "BAT1");
  lv_obj_set_style_text_color(ui_battery1_title, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_battery1_title, BATTERY_TITLE_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_battery1_title, BATTERY_TITLE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_battery1_title, BATTERY_TITLE_WIDTH, BATTERY_TITLE_HEIGHT);
  lv_obj_set_pos(ui_battery1_title, 355, 115);

  // Battery 1 Voltage - căn trái (bên trái) 
  ui_bat1_volt_value = lv_label_create(ui_main_screen);
  char bat1_volt_text[20];
  sprintf(bat1_volt_text, "%.1fV", 48.2f);
  lv_label_set_text(ui_bat1_volt_value, bat1_volt_text);
  lv_obj_set_style_text_color(ui_bat1_volt_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat1_volt_value, BATTERY_VALUE_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat1_volt_value, BATTERY_VALUE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_bat1_volt_value, BATTERY_VALUE_WIDTH, BATTERY_VALUE_HEIGHT);
  lv_obj_set_pos(ui_bat1_volt_value, 385, 130);
  
  // Battery 1 Percentage - căn phải (bên phải)
  ui_bat1_percent_value = lv_label_create(ui_main_screen);
  char bat1_percent_text[20];
  sprintf(bat1_percent_text, "%d%%", 85);
  lv_label_set_text(ui_bat1_percent_value, bat1_percent_text);
  lv_obj_set_style_text_color(ui_bat1_percent_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat1_percent_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat1_percent_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat1_percent_value, 50, 12);
  lv_obj_set_pos(ui_bat1_percent_value, 415, 130);
  
  // Battery 1 Diff Voltage - căn trái (bên trái)
  ui_bat1_diff_value = lv_label_create(ui_main_screen);
  char bat1_diff_text[20];
  sprintf(bat1_diff_text, "%+.2fV", 0.2f);
  lv_label_set_text(ui_bat1_diff_value, bat1_diff_text);
  lv_obj_set_style_text_color(ui_bat1_diff_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat1_diff_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat1_diff_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat1_diff_value, 50, 12);
  lv_obj_set_pos(ui_bat1_diff_value, 385, 142);
  
  // Battery 1 Temperature - căn phải (bên phải)
  ui_bat1_temp_value = lv_label_create(ui_main_screen);
  char bat1_temp_text[20];
  sprintf(bat1_temp_text, "%d°C", 28);
  lv_label_set_text(ui_bat1_temp_value, bat1_temp_text);
  lv_obj_set_style_text_color(ui_bat1_temp_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat1_temp_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat1_temp_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat1_temp_value, 50, 12);
  lv_obj_set_pos(ui_bat1_temp_value, 415, 142);
  
  // Battery 1 Current - căn giữa (giữa)
  ui_bat1_current_value = lv_label_create(ui_main_screen);
  char bat1_current_text[20];
  sprintf(bat1_current_text, "%.1fA", 2.5f);
  lv_label_set_text(ui_bat1_current_value, bat1_current_text);
  lv_obj_set_style_text_color(ui_bat1_current_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat1_current_value, BATTERY_CURRENT_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat1_current_value, BATTERY_CURRENT_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_bat1_current_value, BATTERY_CURRENT_WIDTH, BATTERY_CURRENT_HEIGHT);
  lv_obj_set_pos(ui_bat1_current_value, 355, 154);
  
  // Battery 2 Label "BAT2" riêng - căn lề phải, phía trên
  ui_battery2_title = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_battery2_title, "BAT2");
  lv_obj_set_style_text_color(ui_battery2_title, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_battery2_title, &lv_font_unscii_8, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_battery2_title, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_battery2_title, 110, 20);
  lv_obj_set_pos(ui_battery2_title, 355, 185);
  
  // Battery 2 Voltage - căn trái (bên trái)
  ui_bat2_volt_value = lv_label_create(ui_main_screen);
  char bat2_volt_text[20];
  sprintf(bat2_volt_text, "%.1fV", 47.8f);
  lv_label_set_text(ui_bat2_volt_value, bat2_volt_text);
  lv_obj_set_style_text_color(ui_bat2_volt_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat2_volt_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat2_volt_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat2_volt_value, 50, 12);
  lv_obj_set_pos(ui_bat2_volt_value, 385, 200);
  
  // Battery 2 Percentage - căn phải (bên phải)
  ui_bat2_percent_value = lv_label_create(ui_main_screen);
  char bat2_percent_text[20];
  sprintf(bat2_percent_text, "%d%%", 82);
  lv_label_set_text(ui_bat2_percent_value, bat2_percent_text);
  lv_obj_set_style_text_color(ui_bat2_percent_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat2_percent_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat2_percent_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat2_percent_value, 50, 12);
  lv_obj_set_pos(ui_bat2_percent_value, 415, 200);
  
  // Battery 2 Diff Voltage - căn trái (bên trái)
  ui_bat2_diff_value = lv_label_create(ui_main_screen);
  char bat2_diff_text[20];
  sprintf(bat2_diff_text, "%+.2fV", -0.1f);
  lv_label_set_text(ui_bat2_diff_value, bat2_diff_text);
  lv_obj_set_style_text_color(ui_bat2_diff_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat2_diff_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat2_diff_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat2_diff_value, 50, 12);
  lv_obj_set_pos(ui_bat2_diff_value, 385, 212);
  
  // Battery 2 Temperature - căn phải (bên phải)
  ui_bat2_temp_value = lv_label_create(ui_main_screen);
  char bat2_temp_text[20];
  sprintf(bat2_temp_text, "%d°C", 31);
  lv_label_set_text(ui_bat2_temp_value, bat2_temp_text);
  lv_obj_set_style_text_color(ui_bat2_temp_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat2_temp_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat2_temp_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat2_temp_value, 50, 12);
  lv_obj_set_pos(ui_bat2_temp_value, 415, 212);
  
  // Battery 2 Current - căn giữa (giữa)
  ui_bat2_current_value = lv_label_create(ui_main_screen);
  char bat2_current_text[20];
  sprintf(bat2_current_text, "%.1fA", 1.8f);
  lv_label_set_text(ui_bat2_current_value, bat2_current_text);
  lv_obj_set_style_text_color(ui_bat2_current_value, lv_color_hex(0x4CAF50), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bat2_current_value, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bat2_current_value, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_size(ui_bat2_current_value, 110, 12);
  lv_obj_set_pos(ui_bat2_current_value, 355, 224);
}

// Create ECU temperature display - khôi phục layout gốc
void BikeDisplayUI::createECUDisplay() {
  // ECU Label "ECU" riêng - căn lề phải, phía trên
  ui_ecu_label = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_ecu_label, "ECU");
  lv_obj_set_style_text_color(ui_ecu_label, lv_color_hex(0x00D4AA), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_ecu_label, ECU_TITLE_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_ecu_label, ECU_TITLE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_ecu_label, ECU_TITLE_WIDTH, ECU_TITLE_HEIGHT);
  lv_obj_set_pos(ui_ecu_label, ECU_TITLE_X, ECU_TITLE_Y);
  
  // ECU thông số - căn lề phải, phía dưới label
  ui_ecu_temp_label = lv_label_create(ui_main_screen);
  char ecu_text[40];
  sprintf(ecu_text, "%3d°C", 38);
  lv_label_set_text(ui_ecu_temp_label, ecu_text);
  lv_obj_set_style_text_color(ui_ecu_temp_label, lv_color_hex(0x00D4AA), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_ecu_temp_label, ECU_VALUE_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_ecu_temp_label, ECU_VALUE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_ecu_temp_label, ECU_VALUE_WIDTH, ECU_VALUE_HEIGHT);
  lv_obj_set_pos(ui_ecu_temp_label, ECU_VALUE_X, ECU_VALUE_Y);
}

// Create motor display - khôi phục layout gốc
void BikeDisplayUI::createMotorDisplay() {
  // Motor Label "MOTOR" riêng - căn lề phải, phía trên
  ui_motor_label = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_motor_label, "MOTOR");
  lv_obj_set_style_text_color(ui_motor_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_motor_label, MOTOR_TITLE_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_motor_label, MOTOR_TITLE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_motor_label, MOTOR_TITLE_WIDTH, MOTOR_TITLE_HEIGHT);
  lv_obj_set_pos(ui_motor_label, MOTOR_TITLE_X, MOTOR_TITLE_Y);
  
  // Motor Current Value - căn trái (bên trái)
  ui_motor_current_value = lv_label_create(ui_main_screen);
  char motor_current_text[20];
  sprintf(motor_current_text, "%.1fA", 4.3f);
  lv_label_set_text(ui_motor_current_value, motor_current_text);
  lv_obj_set_style_text_color(ui_motor_current_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_motor_current_value, MOTOR_VALUE_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_motor_current_value, MOTOR_VALUE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_motor_current_value, MOTOR_VALUE_WIDTH, MOTOR_VALUE_HEIGHT);
  lv_obj_set_pos(ui_motor_current_value, 385, 80);
  
  // Motor Temperature Value - căn phải (bên phải)
  ui_motor_temp_value = lv_label_create(ui_main_screen);
  char motor_temp_text[20];
  sprintf(motor_temp_text, "%d°C", 45);
  lv_label_set_text(ui_motor_temp_value, motor_temp_text);
  lv_obj_set_style_text_color(ui_motor_temp_value, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_motor_temp_value, MOTOR_VALUE_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_motor_temp_value, MOTOR_VALUE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(ui_motor_temp_value, MOTOR_VALUE_WIDTH, MOTOR_VALUE_HEIGHT);
  lv_obj_set_pos(ui_motor_temp_value, 415, 80);
}

// Create odometer - khôi phục layout gốc
void BikeDisplayUI::createOdometer() {
  // ODO Display (bottom center) 
  ui_odo_label = lv_label_create(ui_main_screen);
  char odo_text[30];
  sprintf(odo_text, "ODO: %.1f km", 1234.5f);
  lv_label_set_text(ui_odo_label, odo_text);
  lv_obj_set_style_text_color(ui_odo_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_odo_label, ODO_FONT, LV_PART_MAIN);
  lv_obj_align(ui_odo_label, ODO_ALIGN, ODO_X, ODO_Y);
}

// Helper: Get color by temperature
lv_color_t BikeDisplayUI::getColorByTemperature(int temp, int lowThresh, int highThresh) {
  if (temp < lowThresh) return UI_COLOR_SUCCESS;
  else if (temp < highThresh) return UI_COLOR_WARNING;
  else return UI_COLOR_DANGER;
}

// Helper: Get color by percentage
lv_color_t BikeDisplayUI::getColorByPercent(int percent, int lowThresh, int highThresh) {
  if (percent < lowThresh) return UI_COLOR_DANGER;
  else if (percent < highThresh) return UI_COLOR_WARNING;
  else return UI_COLOR_SUCCESS;
}

// Create Bluetooth Icon - khôi phục layout gốc
void BikeDisplayUI::createBluetoothIcon() {
  // Bluetooth Symbol Icon ở phía dưới bên phải
  ui_bluetooth_icon = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_bluetooth_icon, LV_SYMBOL_BLUETOOTH);
  lv_obj_set_style_text_color(ui_bluetooth_icon, UI_COLOR_SUCCESS, LV_PART_MAIN);
  lv_obj_set_style_text_font(ui_bluetooth_icon, BLUETOOTH_ICON_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_bluetooth_icon, BLUETOOTH_ICON_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_align(ui_bluetooth_icon, BLUETOOTH_ICON_ALIGN, BLUETOOTH_ICON_X, BLUETOOTH_ICON_Y);
}

// Create Parking Icon (P indicator - next to Bluetooth)
void BikeDisplayUI::createParkingIcon() {
  // Parking P Icon - hiển thị khi phanh được nhấn
  ui_parking_icon = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_parking_icon, "P");
  lv_obj_set_style_text_color(ui_parking_icon, UI_COLOR_BG, LV_PART_MAIN);  // Ẩn ban đầu
  lv_obj_set_style_text_font(ui_parking_icon, PARKING_ICON_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_parking_icon, PARKING_ICON_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_align(ui_parking_icon, PARKING_ICON_ALIGN, PARKING_ICON_X, PARKING_ICON_Y);
}

// Create Turn Indicators
void BikeDisplayUI::createTurnIndicators() {
  // Turn Left Arrow (bên trái battery %)
  ui_turn_left_icon = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_turn_left_icon, LV_SYMBOL_LEFT);
  lv_obj_set_style_text_color(ui_turn_left_icon, UI_COLOR_BG, LV_PART_MAIN);  // Ẩn ban đầu
  lv_obj_set_style_text_font(ui_turn_left_icon, TURN_ICON_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_turn_left_icon, TURN_ICON_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_align(ui_turn_left_icon, TURN_ICON_ALIGN, TURN_LEFT_ICON_X, TURN_LEFT_ICON_Y);
  
  // Turn Right Arrow (bên phải battery %)
  ui_turn_right_icon = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_turn_right_icon, LV_SYMBOL_RIGHT);
  lv_obj_set_style_text_color(ui_turn_right_icon, UI_COLOR_BG, LV_PART_MAIN);  // Ẩn ban đầu
  lv_obj_set_style_text_font(ui_turn_right_icon, TURN_ICON_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_turn_right_icon, TURN_ICON_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_align(ui_turn_right_icon, TURN_ICON_ALIGN, TURN_RIGHT_ICON_X, TURN_RIGHT_ICON_Y);
}

void BikeDisplayUI::createPassingIndicator() {
  ui_passing_label = lv_label_create(ui_main_screen);
  lv_label_set_text(ui_passing_label, PASSING_LABEL_TEXT);
  lv_obj_set_style_text_font(ui_passing_label, PASSING_LABEL_FONT, LV_PART_MAIN);
  lv_obj_set_style_text_color(ui_passing_label, UI_COLOR_BG, LV_PART_MAIN);
  lv_obj_set_style_text_align(ui_passing_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_align(ui_passing_label, PASSING_LABEL_ALIGN, PASSING_LABEL_X, PASSING_LABEL_Y);
}

// Update speed - direct update without animation
void BikeDisplayUI::updateSpeed(float speed) {
  if (!ui_speed_arc || !ui_speed_label) return;
  
  static float cachedSpeed = -1.0f;
  float _speed = abs(speed);  // Ensure speed is non-negative
  
  // Check if speed changed significantly (>0.5 km/h)
  if (abs(_speed - cachedSpeed) < 0.5f) {
    return;  // No significant change, skip update
  }
  
  // Update cached value
  cachedSpeed = _speed;
  
  // Update speed label
  char buffer[16];
  sprintf(buffer, "%.0f", _speed);
  lv_label_set_text(ui_speed_label, buffer);
  
  // Update speed arc
  int arc_value = (int)_speed;
  if (arc_value > SPEED_ARC_MAX) arc_value = SPEED_ARC_MAX;
  if (arc_value < SPEED_ARC_MIN) arc_value = SPEED_ARC_MIN;
  lv_arc_set_value(ui_speed_arc, arc_value);
  
  // Update color based on speed
  lv_color_t speed_color;
  if (_speed < SPEED_LOW_THRESH) {
    speed_color = SPEED_LOW_COLOR;
  } else if (_speed < SPEED_MEDIUM_THRESH) {
    speed_color = SPEED_MEDIUM_COLOR;
  } else {
    speed_color = SPEED_WARNING_COLOR;
  }
  lv_obj_set_style_arc_color(ui_speed_arc, speed_color, LV_PART_INDICATOR | LV_STATE_DEFAULT);
}

// Update current - direct update without animation
void BikeDisplayUI::updateCurrent(float current, bool isCharging) {
  if (!ui_current_arc || !ui_current_text) return;
  
  static float cachedCurrent = -1.0f;
  static bool cachedIsCharging = false;
  
  float absCurrent = abs(current);  // Get absolute value
  
  // Check if current changed
  if (abs(absCurrent - cachedCurrent) < 0.25f && isCharging == cachedIsCharging) {
    return;  // No significant change, skip update
  }
  
  // Update cached values
  cachedCurrent = absCurrent;
  cachedIsCharging = isCharging;
  
  // Update current label with +/- prefix
  char buffer[16];
  if (isCharging) {
    sprintf(buffer, "+%.1f", absCurrent);
  } else {
    sprintf(buffer, "%.1f", absCurrent);
  }
  lv_label_set_text(ui_current_text, buffer);
  
  // Update current arc
  int arc_value = (int)absCurrent;
  if (arc_value > CURRENT_ARC_MAX) arc_value = CURRENT_ARC_MAX;
  if (arc_value < CURRENT_ARC_MIN) arc_value = CURRENT_ARC_MIN;
  lv_arc_set_value(ui_current_arc, arc_value);
  
  // Update color based on current
  lv_color_t current_color;
  if (absCurrent < CURRENT_LOW_THRESH) {
    current_color = CURRENT_LOW_COLOR;
  } else if (absCurrent < CURRENT_MEDIUM_THRESH) {
    current_color = CURRENT_MEDIUM_COLOR;
  } else if (absCurrent < CURRENT_HIGH_THRESH) {
    current_color = CURRENT_HIGH_COLOR;
  } else {
    current_color = CURRENT_WARNING_COLOR;
  }
  lv_obj_clear_flag(ui_current_arc, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_arc_color(ui_current_arc, current_color, LV_PART_INDICATOR | LV_STATE_DEFAULT);
}

// Update battery percentage
void BikeDisplayUI::updateBattery(int percent) {
  if (!ui_battery_text) return;
  
  static int cachedBatteryPercent = -1;
  
  // Only update if battery percent changed
  if (percent == cachedBatteryPercent) {
    return;
  }
  
  cachedBatteryPercent = percent;
  
  char buffer[16];
  sprintf(buffer, "%d%%", percent);
  lv_label_set_text(ui_battery_text, buffer);
  
  // Color coding for battery
  lv_color_t battery_color = getColorByPercent(percent, 20, 50);
  lv_obj_set_style_text_color(ui_battery_text, battery_color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

// Update ECU temperature
void BikeDisplayUI::updateECU(int temperature) {
  if (!ui_ecu_temp_label) return;
  
  static int cachedEcuTemp = -1;
  
  // Only update if temperature changed
  if (temperature == cachedEcuTemp) {
    return;
  }
  
  cachedEcuTemp = temperature;
  
  char buffer[16];
  sprintf(buffer, "%d°C", temperature);
  lv_label_set_text(ui_ecu_temp_label, buffer);
  
  // Color coding for ECU temperature
  lv_color_t temp_color = getColorByTemperature(temperature, 50, 70);
  lv_obj_set_style_text_color(ui_ecu_temp_label, temp_color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

// Update motor temperature and current
void BikeDisplayUI::updateMotor(int temperature, float current) {
  static int cachedMotorTemp = -1;
  static float cachedMotorCurrent = -1.0f;
  
  // Check if motor temp changed
  if (temperature != cachedMotorTemp) {
    cachedMotorTemp = temperature;
    
    if (ui_motor_temp_value) {
      char temp_buffer[16];
      sprintf(temp_buffer, "%d°C", temperature);
      lv_label_set_text(ui_motor_temp_value, temp_buffer);
      
      // Color coding for motor temperature  
      lv_color_t temp_color = getColorByTemperature(temperature, 60, 80);
      lv_obj_set_style_text_color(ui_motor_temp_value, temp_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
  
  // Check if motor current changed
  if (abs(current - cachedMotorCurrent) >= 0.1f) {
    cachedMotorCurrent = current;
    
    if (ui_motor_current_value) {
      char current_buffer[16];
      sprintf(current_buffer, "%.1fA", current);
      lv_label_set_text(ui_motor_current_value, current_buffer);
    }
  }
}

// Update Battery 1
void BikeDisplayUI::updateBattery1(float volt, int percent, uint16_t diffVoltMv, int temp, float current) {
  if (ui_bat1_volt_value) {
    char buffer[16];
    sprintf(buffer, "%.1fV", volt);
    lv_label_set_text(ui_bat1_volt_value, buffer);
  }
  
  if (ui_bat1_percent_value) {
    char buffer[16];
    sprintf(buffer, "%d%%", percent);
    lv_label_set_text(ui_bat1_percent_value, buffer);
    lv_color_t color = getColorByPercent(percent, 20, 50);
    lv_obj_set_style_text_color(ui_bat1_percent_value, color, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  if (ui_bat1_diff_value) {
    char buffer[16];
    sprintf(buffer, "%dmV", diffVoltMv);
    lv_label_set_text(ui_bat1_diff_value, buffer);
    lv_color_t color = (diffVoltMv > 500) ? UI_COLOR_DANGER : UI_COLOR_SUCCESS;  // 500mV threshold
    lv_obj_set_style_text_color(ui_bat1_diff_value, color, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  if (ui_bat1_temp_value) {
    char buffer[16];
    sprintf(buffer, "%d°C", temp);
    lv_label_set_text(ui_bat1_temp_value, buffer);
    lv_color_t color = getColorByTemperature(temp, 35, 45);
    lv_obj_set_style_text_color(ui_bat1_temp_value, color, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  if (ui_bat1_current_value) {
    char buffer[16];
    sprintf(buffer, "%.1fA", current);
    lv_label_set_text(ui_bat1_current_value, buffer);
  }
}

// Update Battery 2
void BikeDisplayUI::updateBattery2(float volt, int percent, uint16_t diffVoltMv, int temp, float current) {
  if (ui_bat2_volt_value) {
    char buffer[16];
    sprintf(buffer, "%.1fV", volt);
    lv_label_set_text(ui_bat2_volt_value, buffer);
  }
  
  if (ui_bat2_percent_value) {
    char buffer[16];
    sprintf(buffer, "%d%%", percent);
    lv_label_set_text(ui_bat2_percent_value, buffer);
    lv_color_t color = getColorByPercent(percent, 20, 50);
    lv_obj_set_style_text_color(ui_bat2_percent_value, color, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  if (ui_bat2_diff_value) {
    char buffer[16];
    sprintf(buffer, "%dmV", diffVoltMv);
    lv_label_set_text(ui_bat2_diff_value, buffer);
    lv_color_t color = (diffVoltMv > 500) ? UI_COLOR_DANGER : UI_COLOR_SUCCESS;  // 500mV threshold
    lv_obj_set_style_text_color(ui_bat2_diff_value, color, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  if (ui_bat2_temp_value) {
    char buffer[16];
    sprintf(buffer, "%d°C", temp);
    lv_label_set_text(ui_bat2_temp_value, buffer);
    lv_color_t color = getColorByTemperature(temp, 35, 45);
    lv_obj_set_style_text_color(ui_bat2_temp_value, color, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  if (ui_bat2_current_value) {
    char buffer[16];
    sprintf(buffer, "%.1fA", current);
    lv_label_set_text(ui_bat2_current_value, buffer);
  }
}

// Update odometer
void BikeDisplayUI::updateOdometer(float distance) {
  if (!ui_odo_label) return;
  
  static float cachedOdometer = -1.0f;
  
  // Only update if odometer changed significantly (>0.1 km)
  if (abs(distance - cachedOdometer) < 0.1f) {
    return;
  }
  
  cachedOdometer = distance;
  
  char buffer[32];
  sprintf(buffer, "ODO: %.1fkm", distance);
  lv_label_set_text(ui_odo_label, buffer);
}

// Update Bluetooth status
void BikeDisplayUI::updateBluetooth(bool connected) {
  if (!ui_bluetooth_icon) return;
  
  static bool cachedBluetoothConnected = false;
  
  // Only update if status changed
  if (connected == cachedBluetoothConnected) {
    return;
  }
  
  cachedBluetoothConnected = connected;
  
  if (connected) {
    lv_obj_set_style_text_color(ui_bluetooth_icon, UI_COLOR_ACCENT, LV_PART_MAIN | LV_STATE_DEFAULT);
  } else {
    lv_obj_set_style_text_color(ui_bluetooth_icon, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

// Update Parking status (P indicator)
void BikeDisplayUI::updateParking(bool parking) {
  if (!ui_parking_icon) return;
  
  static bool cachedParkingActive = false;
  
  // Only update if status changed
  if (parking == cachedParkingActive) {
    return;
  }
  
  cachedParkingActive = parking;
  
  if (parking) {
    lv_obj_set_style_text_color(ui_parking_icon, UI_COLOR_WARNING, LV_PART_MAIN | LV_STATE_DEFAULT);
  } else {
    lv_obj_set_style_text_color(ui_parking_icon, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

// Update Turn Indicators
void BikeDisplayUI::updateTurnIndicators(bool leftActive, bool rightActive) {
  if (!ui_turn_left_icon || !ui_turn_right_icon) return;
  
  static bool cachedTurnLeftActive = false;
  static bool cachedTurnRightActive = false;
  
  // Only update if status changed
  if (leftActive == cachedTurnLeftActive && rightActive == cachedTurnRightActive) {
    return;
  }
  
  cachedTurnLeftActive = leftActive;
  cachedTurnRightActive = rightActive;
  
  // Left turn indicator
  if (leftActive) {
    lv_obj_set_style_text_color(ui_turn_left_icon, UI_COLOR_WARNING, LV_PART_MAIN | LV_STATE_DEFAULT);
  } else {
    lv_obj_set_style_text_color(ui_turn_left_icon, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  // Right turn indicator  
  if (rightActive) {
    lv_obj_set_style_text_color(ui_turn_right_icon, UI_COLOR_WARNING, LV_PART_MAIN | LV_STATE_DEFAULT);
  } else {
    lv_obj_set_style_text_color(ui_turn_right_icon, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void BikeDisplayUI::updatePassing(bool active) {
  if (!ui_passing_label) return;
  
  static bool cachedPassingActive = false;
  
  // Only update if status changed
  if (active == cachedPassingActive) {
    return;
  }
  
  cachedPassingActive = active;
  
  lv_color_t color = active ? UI_COLOR_WARNING : UI_COLOR_BG;
  lv_obj_set_style_text_color(ui_passing_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

// Update all data at once
void BikeDisplayUI::updateAll(const BikeDataDisplay& data) {
  updateSpeed(data.speed);
  updateCurrent(data.current, data.isCharging);
  updateBattery(data.batteryPercent);
  updateECU(data.ecuTemp);
  updateMotor(data.motorTemp, data.motorCurrent);
  updateBattery1(data.battery1Volt, data.battery1Percent, data.battery1DiffVolt, data.battery1Temp, data.battery1Current);
  updateBattery2(data.battery2Volt, data.battery2Percent, data.battery2DiffVolt, data.battery2Temp, data.battery2Current);
  updateOdometer(data.odometer);
  updateBluetooth(data.bluetoothConnected);
  updateParking(data.parkingActive);
  updateTurnIndicators(data.turnLeftActive, data.turnRightActive);
  updatePassing(data.passingActive);
}

// Apply theme colors to all UI elements
void BikeDisplayUI::applyThemeToAllElements() {
  if (!ui_main_screen) return;
  
  // Background
  lv_obj_set_style_bg_color(ui_main_screen, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  // Main text colors
  lv_color_t textColor = UI_COLOR_ACCENT;
  lv_color_t secondaryTextColor = UI_COLOR_WARNING;
  
  // Speed display
  if (ui_speed_label) {
    lv_obj_set_style_text_color(ui_speed_label, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_speed_unit_label) {
    lv_obj_set_style_text_color(ui_speed_unit_label, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  // Current display
  if (ui_current_text) {
    lv_obj_set_style_text_color(ui_current_text, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_current_label) {
    lv_obj_set_style_text_color(ui_current_label, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  // Battery
  if (ui_battery_text) {
    lv_obj_set_style_text_color(ui_battery_text, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_battery1_title) {
    lv_obj_set_style_text_color(ui_battery1_title, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_battery2_title) {
    lv_obj_set_style_text_color(ui_battery2_title, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  // Battery 1 values
  if (ui_bat1_volt_value) lv_obj_set_style_text_color(ui_bat1_volt_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat1_percent_value) lv_obj_set_style_text_color(ui_bat1_percent_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat1_diff_value) lv_obj_set_style_text_color(ui_bat1_diff_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat1_temp_value) lv_obj_set_style_text_color(ui_bat1_temp_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat1_current_value) lv_obj_set_style_text_color(ui_bat1_current_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  // Battery 2 values
  if (ui_bat2_volt_value) lv_obj_set_style_text_color(ui_bat2_volt_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat2_percent_value) lv_obj_set_style_text_color(ui_bat2_percent_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat2_diff_value) lv_obj_set_style_text_color(ui_bat2_diff_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat2_temp_value) lv_obj_set_style_text_color(ui_bat2_temp_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_bat2_current_value) lv_obj_set_style_text_color(ui_bat2_current_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  // ECU and Motor labels
  if (ui_ecu_label) {
    lv_obj_set_style_text_color(ui_ecu_label, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_motor_label) {
    lv_obj_set_style_text_color(ui_motor_label, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  // ECU and Motor values
  if (ui_ecu_temp_label) lv_obj_set_style_text_color(ui_ecu_temp_label, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_motor_temp_value) lv_obj_set_style_text_color(ui_motor_temp_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  if (ui_motor_current_value) lv_obj_set_style_text_color(ui_motor_current_value, secondaryTextColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  // Odometer
  if (ui_odo_label) {
    lv_obj_set_style_text_color(ui_odo_label, textColor, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  
  // Icons (Bluetooth, Parking, Turn indicators, Passing)
  if (ui_bluetooth_icon) {
    lv_obj_set_style_text_color(ui_bluetooth_icon, UI_COLOR_SUCCESS, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_parking_icon) {
    lv_obj_set_style_text_color(ui_parking_icon, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_turn_left_icon) {
    lv_obj_set_style_text_color(ui_turn_left_icon, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_turn_right_icon) {
    lv_obj_set_style_text_color(ui_turn_right_icon, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  if (ui_passing_label) {
    lv_obj_set_style_text_color(ui_passing_label, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

// Theme setting
void BikeDisplayUI::setTheme(int themeId) {
  // Đổi theme: 0 = dark, 1 = light
  if (themeId == THEME_DARK || themeId == THEME_LIGHT) {
    CURRENT_THEME = themeId;
    
    // Apply new theme to all elements
    applyThemeToAllElements();
    
    Serial.printf("[THEME] Theme changed to %d, updated all UI elements\n", themeId);
  }
}

// Get current theme
int BikeDisplayUI::getCurrentTheme() {
  return CURRENT_THEME;
}

// Flash screen effect for theme confirmation
void BikeDisplayUI::flashScreen() {
  if (!ui_main_screen) return;
  
  // Save current background color
  lv_color_t originalColor = lv_obj_get_style_bg_color(ui_main_screen, LV_PART_MAIN);
  
  // Flash to white
  lv_obj_set_style_bg_color(ui_main_screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  
  // Use a timer to revert after 200ms
  lv_timer_t *flashTimer = lv_timer_create([](lv_timer_t *timer) {
    BikeDisplayUI *ui = (BikeDisplayUI *)timer->user_data;
    if (ui->ui_main_screen) {
      lv_obj_set_style_bg_color(ui->ui_main_screen, UI_COLOR_BG, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_timer_del(timer);
  }, 200, this);
}

// Set speed range
void BikeDisplayUI::setSpeedRange(int maxSpeed) {
  if (ui_speed_arc) {
    lv_arc_set_range(ui_speed_arc, SPEED_ARC_MIN, maxSpeed);
  }
}

// Set current range
void BikeDisplayUI::setCurrentRange(int maxCurrent) {
  if (ui_current_arc) {
    lv_arc_set_range(ui_current_arc, CURRENT_ARC_MIN, maxCurrent);
  }
}
