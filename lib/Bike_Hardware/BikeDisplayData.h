/*
 * Bike Display Data Structure
 * 
 * Định nghĩa struct data cho việc truyền thông tin xe
 * từ ECU chính đến mạch hiển thị
 * 
 * Author: SAOKIM99
 * Date: 2025
 */

#ifndef BIKE_DISPLAY_DATA_H
#define BIKE_DISPLAY_DATA_H

#include <Arduino.h>

// ECU Information
struct ECUData {
    float temperature;          // Nhiệt độ ECU (°C)
    bool overTemperature;       // Cảnh báo quá nhiệt
};

// Motor Information  
struct MotorData {
    float temperature;          // Nhiệt độ motor (°C)
    float current;              // Cường độ dòng điện motor (A)
    bool overTemperature;       // Cảnh báo quá nhiệt motor
    bool overCurrent;           // Cảnh báo quá dòng motor
};

// Battery Information (BAT1 & BAT2)
struct BatteryData {
    float voltage;              // Điện áp pin (V)
    uint8_t soc;                // % pin (0-100%)
    float cellVoltageDelta;     // Chênh lệch điện áp cell (V)
    float temperature;          // Nhiệt độ pin (°C)
    float current;              // Cường độ dòng điện pin (A)
    bool connected;             // Trạng thái kết nối BMS
    bool charging;              // Đang sạc pin
    bool discharging;           // Đang xả pin
    bool overVoltage;           // Cảnh báo quá áp
    bool underVoltage;          // Cảnh báo thiếu áp
    bool overTemperature;       // Cảnh báo quá nhiệt
    bool overCurrent;           // Cảnh báo quá dòng
};

// Signal States
struct SignalData {
    bool leftTurn;              // Xi nhan trái
    bool rightTurn;             // Xi nhan phải
    bool hazard;                // Đèn cảnh báo (xi nhan 2 bên)
};

// BLE Connection Status
enum BLEStatus {
    BLE_DISCONNECTED = 0,       // Không kết nối
    BLE_CONNECTING = 1,         // Đang kết nối
    BLE_CONNECTED = 2,          // Đã kết nối
    BLE_PAIRING = 3            // Đang ghép đôi
};

// Battery System Status
enum BatterySystemStatus {
    BATTERY_IDLE = 0,           // Không hoạt động
    BATTERY_CHARGING = 1,       // Đang sạc
    BATTERY_DISCHARGING = 2,    // Đang xả (sử dụng)
    BATTERY_FAULT = 3          // Lỗi hệ thống pin
};

// Main Display Data Structure
struct BikeDisplayData {
    // System info
    uint32_t timestamp;         // Timestamp (ms)
    uint16_t sequence;          // Số thứ tự packet
    
    // Component data
    ECUData ecu;               // Thông tin ECU
    MotorData motor;           // Thông tin Motor
    BatteryData bat1;          // Pin 1
    BatteryData bat2;          // Pin 2
    
    // Vehicle signals
    SignalData signals;        // Xi nhan
    
    // Communication status
    BLEStatus bleStatus;       // Trạng thái BLE
    
    // Battery system status
    BatterySystemStatus batterySystemStatus;  // Trạng thái sạc/sử dụng
    
    // System health
    bool systemHealthy;        // Tổng thể hệ thống ổn định
    uint8_t errorCount;        // Số lượng lỗi hiện tại
    
    // Additional info
    float totalVoltage;        // Tổng điện áp hệ thống (V)
    float totalCurrent;        // Tổng dòng điện hệ thống (A)
    uint8_t totalSOC;          // % pin trung bình hệ thống
};

// Error flags for quick status check
struct SystemErrors {
    union {
        struct {
            uint16_t ecuOverTemp : 1;       // ECU quá nhiệt
            uint16_t motorOverTemp : 1;     // Motor quá nhiệt  
            uint16_t motorOverCurrent : 1;  // Motor quá dòng
            uint16_t bat1Fault : 1;         // Pin 1 lỗi
            uint16_t bat2Fault : 1;         // Pin 2 lỗi
            uint16_t bat1OverTemp : 1;      // Pin 1 quá nhiệt
            uint16_t bat2OverTemp : 1;      // Pin 2 quá nhiệt
            uint16_t bat1OverVolt : 1;      // Pin 1 quá áp
            uint16_t bat2OverVolt : 1;      // Pin 2 quá áp
            uint16_t bat1UnderVolt : 1;     // Pin 1 thiếu áp
            uint16_t bat2UnderVolt : 1;     // Pin 2 thiếu áp
            uint16_t bleConnectionLost : 1; // Mất kết nối BLE
            uint16_t reserved : 4;          // Dự phòng
        } bits;
        uint16_t all;                       // Tất cả flags
    };
};

// Complete data package for transmission
struct DisplayDataPackage {
    BikeDisplayData data;       // Dữ liệu chính
    SystemErrors errors;        // Flags lỗi
    uint16_t checksum;          // Checksum để kiểm tra tính toàn vẹn
};

// Helper functions for data processing
class BikeDisplayDataHelper {
public:
    // Calculate checksum for data integrity
    static uint16_t calculateChecksum(const BikeDisplayData& data);
    
    // Validate data package
    static bool validatePackage(const DisplayDataPackage& package);
    
    // Convert to JSON string for debugging
    static String toJSON(const BikeDisplayData& data);
    
    // Check if system has critical errors
    static bool hasCriticalErrors(const SystemErrors& errors);
    
    // Get error count from flags
    static uint8_t getErrorCount(const SystemErrors& errors);
    
    // Update battery system status based on individual battery states
    static BatterySystemStatus calculateBatterySystemStatus(const BatteryData& bat1, const BatteryData& bat2);
    
    // Calculate total system values
    static void calculateSystemTotals(BikeDisplayData& data);
};

// Constants for thresholds
namespace BikeThresholds {
    const float ECU_TEMP_WARNING = 70.0f;      // °C
    const float MOTOR_TEMP_WARNING = 80.0f;    // °C
    const float MOTOR_CURRENT_WARNING = 50.0f; // A
    const float BATTERY_TEMP_WARNING = 50.0f;  // °C
    const float BATTERY_VOLT_MIN = 40.0f;      // V
    const float BATTERY_VOLT_MAX = 58.0f;      // V
    const float BATTERY_CURRENT_MAX = 30.0f;   // A
    const float CELL_VOLTAGE_DELTA_MAX = 0.1f; // V
}

#endif // BIKE_DISPLAY_DATA_H