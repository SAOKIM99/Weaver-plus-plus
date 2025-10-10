#ifndef BIKE_DATA_H
#define BIKE_DATA_H

#include <Arduino.h>

// ================================================
// BIKE DATA STRUCTURE for MAIN
// ================================================

// Bike States
enum BikeOperationState {
    BIKE_OFF = 0,
    BIKE_ON = 1,
    BIKE_LOCKED = 2,
    BIKE_UNLOCKED = 3
};

// System Events for RTOS communication
enum SystemEvent {
    EVENT_RFID_CARD_DETECTED,
    EVENT_BIKE_UNLOCKED,
    EVENT_BIKE_LOCKED,
    EVENT_BLE_CONNECTED,
    EVENT_BLE_DISCONNECTED,
    EVENT_EMERGENCY_STOP
};

// Sensor Data Structures
struct BMSData {
    // Basic measurements
    float voltage;              // Total pack voltage (V)
    float current;              // Pack current (A) - positive = discharge, negative = charge
    float temperature;          // Battery temperature (°C)
    uint8_t soc;               // State of charge (%)
    bool connected;            // Connection status
    
    // Extended BMS data
    uint16_t cycles;           // Battery cycles
    float powerTemp;           // Power board temperature (°C)
    float boxTemp;             // BMS box temperature (°C)
    
    // Cell voltages
    uint8_t numCells;          // Number of cells
    float lowestCellVolt;      // Lowest cell voltage (V)
    float highestCellVolt;     // Highest cell voltage (V)
    uint16_t cellVoltageDelta; // Cell voltage difference (mV)
    
    // Status flags
    uint16_t alarmStatus;      // Alarm status bits
    uint16_t statusInfo;       // Status info bits
    bool isCharging;           // Charging status
    bool isDischarging;        // Discharging status
    bool chargingEnabled;      // Charging MOS enabled
    bool dischargingEnabled;   // Discharging MOS enabled
    
    // Info
    String softwareVersion;    // BMS software version
    String deviceInfo;         // Device information
};

struct VESCData {
    float motorRPM;
    float inputVoltage;
    float motorCurrent;
    float inputCurrent;
    float dutyCycle;
    float tempFET;
    float tempMotor;
    bool connected;
};

struct BikeStatus {
    BikeOperationState operationState;
    bool brakePressed;
    bool leftSignal;
    bool rightSignal;
    bool keyOn;
    float hallFrequency;    // Raw Hall sensor frequency (Hz)
    float bikeSpeed;        // Calculated bike speed (km/h)
    BMSData bms1;
    BMSData bms2;
    VESCData vesc;
    float analogReadings[8]; // For unused analog pins
};

// Shared data structure for RTOS communication
struct SharedBikeData {
    BikeStatus sensorData;
    bool bikeUnlocked;
    bool bleConnected;
    BikeOperationState currentState;
};


// ================================================
// BIKE DATA STRUCTURE for DISPLAY
// ================================================
struct BikeDataDisplay {
  float speed = 0;
  // Battery data
  int batteryPercent = 0;
  float batteryVoltage = 0;
  float voltage = 0;
  float current = 0;
  bool isCharging = false;
  bool bluetoothConnected = false;  // Bluetooth connection status
  // Turn indicators
  bool turnLeftActive = false;    // Rẽ trái active
  bool turnRightActive = false;   // Rẽ phải active
  bool passingActive = false;     // Nút passing (đèn pha nháy)
  // Motor data
  int motorTemp = 0;        // Nhiệt độ động cơ (°C)
  int ecuTemp = 0;          // Nhiệt độ ECU (°C)
  float motorCurrent = 0;  // Dòng điện động cơ (A)
  int motorPower = 0;

  // Battery 1 data
  float battery1Volt = 0;    // Điện áp battery 1 (V)
  int battery1Percent = 0;     // % battery 1
  int battery1Temp = 0;        // Nhiệt độ battery 1 (°C)
  float battery1Current = 0;  // Dòng điện battery 1 (A)
  uint16_t battery1DiffVolt = 0; // Chênh lệch điện áp battery 1 (mV)

  // Battery 2 data
  float battery2Volt = 0;    // Điện áp battery 2 (V)
  int battery2Percent = 0;     // % battery 2
  int battery2Temp = 0;        // Nhiệt độ battery 2 (°C)
  float battery2Current = 0;  // Dòng điện battery 2 (A)
  uint16_t battery2DiffVolt = 0; // Chênh lệch điện áp battery 2 (mV)

  // Distance data
  float odometer = 0;
  float distance = 0;
  float tripDistance = 0;

  // Time data
  int time = 0;

};


// ================================================
// CONVERSION FUNCTIONS
// ================================================

// Helper function: Get maximum temperature from all available sensors for a BMS
inline float getMaxBMSTemperature(const BMSData& bms) {
    float maxTemp = bms.temperature;  // Battery temperature (primary)
    
    // Check power temperature (MOSFET temperature)
    if (bms.powerTemp > -50.0f && bms.powerTemp < 150.0f) {  // Valid range check
        maxTemp = max(maxTemp, bms.powerTemp);
    }
    
    // Check box temperature (BMS controller temperature)  
    if (bms.boxTemp > -50.0f && bms.boxTemp < 150.0f) {  // Valid range check
        maxTemp = max(maxTemp, bms.boxTemp);
    }
    
    // Debug: Print max temperature calculation (disabled)
    // Serial.printf("🔥 [MaxTemp] Battery: %.1f°C, Power: %.1f°C, Box: %.1f°C → Max: %.1f°C\n", 
    //              bms.temperature, bms.powerTemp, bms.boxTemp, maxTemp);
    
    return maxTemp;
}

// Convert BikeStatus (from main) to BikeDataDisplay (for display)
// Uses leftSignal & rightSignal directly from BikeStatus
// Usage: BikeDataDisplay display = convertToDisplayData(sensorData, bleConnected);
inline BikeDataDisplay convertToDisplayData(const BikeStatus& status, bool bleConnected = false) {
    BikeDataDisplay displayData;
    
    // Basic data
    displayData.speed = status.bikeSpeed;
    
    // Current logic: Speed ~0 use battery current, otherwise use motor current
    if (status.bikeSpeed <= 0.1f) {  // Speed approximately 0 (threshold 0.1 km/h)
        displayData.current = status.bms1.current + status.bms2.current;  // Total battery current
    } else {
        displayData.current = status.vesc.motorCurrent;  // Motor current when moving
    }
    
    displayData.voltage = (status.bms1.voltage + status.bms2.voltage) / 2.0f;
    displayData.batteryVoltage = displayData.voltage;
    
    // Calculate average battery percentage
    displayData.batteryPercent = (status.bms1.soc + status.bms2.soc) / 2;

    // Charging status
    displayData.isCharging = status.bms1.isCharging || status.bms2.isCharging;
    
    // Calculate motor power (V * I)
    displayData.motorPower = (int)(displayData.voltage * displayData.current);
    
    // Motor data from VESC
    displayData.motorTemp = (int)status.vesc.tempMotor;
    displayData.ecuTemp = (int)status.vesc.tempFET;
    displayData.motorCurrent = status.vesc.motorCurrent;
    
    // Battery 1 data
    displayData.battery1Volt = status.bms1.voltage;
    displayData.battery1Percent = status.bms1.soc;
    displayData.battery1Temp = (int)getMaxBMSTemperature(status.bms1);  // Use maximum temperature
    displayData.battery1Current = status.bms1.current;
    displayData.battery1DiffVolt = status.bms1.cellVoltageDelta; // Already in mV
    
    // Battery 2 data
    displayData.battery2Volt = status.bms2.voltage;
    displayData.battery2Percent = status.bms2.soc;
    displayData.battery2Temp = (int)getMaxBMSTemperature(status.bms2);  // Use maximum temperature
    displayData.battery2Current = status.bms2.current;
    displayData.battery2DiffVolt = status.bms2.cellVoltageDelta; // Already in mV
    
    // Signal data - use directly from BikeStatus
    displayData.turnLeftActive = status.leftSignal;
    displayData.turnRightActive = status.rightSignal;
    displayData.passingActive = false;
    
    // External BLE connection
    displayData.bluetoothConnected = bleConnected;
    
    return displayData;
}

// Update BikeDataDisplay with real sensor data (for integrated system)
inline void updateBikeDataFromSensors(BikeDataDisplay& displayData, const BikeStatus& sensorData, 
                                     bool bleConnected = false) {
    displayData = convertToDisplayData(sensorData, bleConnected);
}

#endif
