/*
 * Bike CAN Protocol - Efficient Data Encoding
 * 
 * Hệ thống mã hóa hiệu quả cho truyền dữ liệu qua CAN bus
 * Mỗi CAN frame tối đa 8 bytes, cần tối ưu bandwidth
 * 
 * Author: SAOKIM99
 * Date: 2025
 */

#ifndef BIKE_CAN_PROTOCOL_H
#define BIKE_CAN_PROTOCOL_H

#include <Arduino.h>
#include "BikeDisplayData.h"

// CAN Message IDs - 11-bit standard format
namespace CANMessageIDs {
    const uint16_t ECU_DATA         = 0x100;  // ECU temperature data
    const uint16_t MOTOR_DATA       = 0x110;  // Motor temp + current
    const uint16_t BAT1_VOLTAGE_SOC = 0x120;  // Battery 1 voltage + SOC
    const uint16_t BAT1_TEMP_CURR   = 0x121;  // Battery 1 temp + current
    const uint16_t BAT1_STATUS      = 0x122;  // Battery 1 status flags
    const uint16_t BAT2_VOLTAGE_SOC = 0x130;  // Battery 2 voltage + SOC
    const uint16_t BAT2_TEMP_CURR   = 0x131;  // Battery 2 temp + current
    const uint16_t BAT2_STATUS      = 0x132;  // Battery 2 status flags
    const uint16_t SIGNALS_BLE      = 0x140;  // Signals + BLE status
    const uint16_t SYSTEM_STATUS    = 0x150;  // System health + errors
    const uint16_t HEARTBEAT        = 0x1FF;  // Heartbeat + sequence
}

// Data encoding helper class
class BikeCANEncoder {
public:
    // Encode ECU data (4 bytes used)
    // Byte 0-1: Temperature * 100 (int16_t, -327.68°C to +327.67°C)
    // Byte 2: Status flags
    // Byte 3: Reserved
    static void encodeECUData(const ECUData& ecu, uint8_t* canData);
    
    // Encode Motor data (6 bytes used)
    // Byte 0-1: Temperature * 100 (int16_t)
    // Byte 2-3: Current * 100 (int16_t, -327.68A to +327.67A)  
    // Byte 4: Status flags
    // Byte 5: Reserved
    static void encodeMotorData(const MotorData& motor, uint8_t* canData);
    
    // Encode Battery Voltage + SOC (5 bytes used)
    // Byte 0-1: Voltage * 100 (uint16_t, 0-655.35V)
    // Byte 2: SOC (uint8_t, 0-100%)
    // Byte 3-4: Cell Delta * 10000 (uint16_t, 0-6.5535V)
    static void encodeBatteryVoltageSOC(const BatteryData& battery, uint8_t* canData);
    
    // Encode Battery Temperature + Current (6 bytes used)
    // Byte 0-1: Temperature * 100 (int16_t)
    // Byte 2-3: Current * 100 (int16_t)
    // Byte 4: Connection status
    // Byte 5: Reserved
    static void encodeBatteryTempCurrent(const BatteryData& battery, uint8_t* canData);
    
    // Encode Battery Status Flags (2 bytes used)
    // Byte 0: Status flags (charging, discharging, etc.)
    // Byte 1: Error flags (over voltage, temp, etc.)
    static void encodeBatteryStatus(const BatteryData& battery, uint8_t* canData);
    
    // Encode Signals + BLE (3 bytes used)
    // Byte 0: Signal flags (left, right, hazard)
    // Byte 1: BLE status
    // Byte 2: Battery system status
    static void encodeSignalsBLE(const SignalData& signals, BLEStatus bleStatus, 
                                BatterySystemStatus batteryStatus, uint8_t* canData);
    
    // Encode System Status (4 bytes used)
    // Byte 0: System healthy flag + error count
    // Byte 1-2: Error flags (16-bit)
    // Byte 3: Reserved
    static void encodeSystemStatus(bool systemHealthy, uint8_t errorCount, 
                                  uint16_t errorFlags, uint8_t* canData);
    
    // Encode Heartbeat (8 bytes used)
    // Byte 0-3: Timestamp (uint32_t)
    // Byte 4-5: Sequence number (uint16_t)
    // Byte 6-7: Total voltage * 10 (uint16_t, 0-6553.5V)
    static void encodeHeartbeat(uint32_t timestamp, uint16_t sequence, 
                               float totalVoltage, uint8_t* canData);
};

// Data decoding helper class  
class BikeCANDecoder {
public:
    // Decode functions - mirror of encode functions
    static void decodeECUData(const uint8_t* canData, ECUData& ecu);
    static void decodeMotorData(const uint8_t* canData, MotorData& motor);
    static void decodeBatteryVoltageSOC(const uint8_t* canData, BatteryData& battery);
    static void decodeBatteryTempCurrent(const uint8_t* canData, BatteryData& battery);
    static void decodeBatteryStatus(const uint8_t* canData, BatteryData& battery);
    static void decodeSignalsBLE(const uint8_t* canData, SignalData& signals, 
                                BLEStatus& bleStatus, BatterySystemStatus& batteryStatus);
    static void decodeSystemStatus(const uint8_t* canData, bool& systemHealthy, 
                                  uint8_t& errorCount, uint16_t& errorFlags);
    static void decodeHeartbeat(const uint8_t* canData, uint32_t& timestamp, 
                               uint16_t& sequence, float& totalVoltage);
};

// CAN transmission manager
class BikeCANManager {
private:
    uint16_t sequenceNumber;
    unsigned long lastTransmission[10]; // Track last transmission time for each message type
    
public:
    BikeCANManager();
    
    // Initialize CAN bus
    bool begin(uint32_t canSpeed = 500000);
    
    // Send complete bike data (will be split into multiple CAN frames)
    bool sendBikeData(const BikeDisplayData& data);
    
    // Send individual data types (for selective updates)
    bool sendECUData(const ECUData& ecu);
    bool sendMotorData(const MotorData& motor);
    bool sendBatteryData(const BatteryData& battery, bool isBat1);
    bool sendSignalsAndBLE(const SignalData& signals, BLEStatus bleStatus, 
                          BatterySystemStatus batteryStatus);
    bool sendSystemStatus(bool systemHealthy, uint8_t errorCount, uint16_t errorFlags);
    bool sendHeartbeat(uint32_t timestamp, float totalVoltage);
    
    // Receive and reconstruct data
    bool receiveData(BikeDisplayData& data);
    
    // Check if complete data set has been received
    bool isDataComplete() const;
    
    // Get data age (time since last complete update)
    unsigned long getDataAge() const;
    
    // Statistics
    uint32_t getTotalFramesSent() const { return totalFramesSent; }
    uint32_t getTotalFramesReceived() const { return totalFramesReceived; }
    uint32_t getTransmissionErrors() const { return transmissionErrors; }
    
private:
    // Internal state for received data reconstruction
    BikeDisplayData receivedData;
    uint8_t receivedFlags; // Bitmap of received message types
    unsigned long lastCompleteUpdate;
    
    // Statistics
    uint32_t totalFramesSent;
    uint32_t totalFramesReceived;
    uint32_t transmissionErrors;
    
    // Helper functions
    bool sendCANFrame(uint16_t messageId, const uint8_t* data, uint8_t length);
    bool receiveCANFrame(uint16_t& messageId, uint8_t* data, uint8_t& length);
    void updateReceivedFlags(uint16_t messageId);
    bool shouldTransmit(uint8_t messageType, unsigned long interval);
};

// Transmission intervals (ms) for different message types
namespace TransmissionIntervals {
    const unsigned long ECU_DATA = 1000;         // 1Hz - Low priority
    const unsigned long MOTOR_DATA = 200;        // 5Hz - Medium priority
    const unsigned long BATTERY_DATA = 500;      // 2Hz - Medium priority
    const unsigned long SIGNALS_BLE = 100;       // 10Hz - High priority (safety)
    const unsigned long SYSTEM_STATUS = 1000;    // 1Hz - Low priority
    const unsigned long HEARTBEAT = 1000;        // 1Hz - Keep-alive
}

// Message type flags for received data tracking
namespace MessageFlags {
    const uint8_t ECU_RECEIVED = 0x01;
    const uint8_t MOTOR_RECEIVED = 0x02;
    const uint8_t BAT1_RECEIVED = 0x04;
    const uint8_t BAT2_RECEIVED = 0x08;
    const uint8_t SIGNALS_RECEIVED = 0x10;
    const uint8_t SYSTEM_RECEIVED = 0x20;
    const uint8_t HEARTBEAT_RECEIVED = 0x40;
    const uint8_t ALL_RECEIVED = 0x7F;
}

#endif // BIKE_CAN_PROTOCOL_H