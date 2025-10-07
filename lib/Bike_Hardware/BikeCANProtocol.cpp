/*
 * Bike CAN Protocol Implementation
 * 
 * Implementation của hệ thống mã hóa/giải mã CAN
 * 
 * Author: SAOKIM99
 * Date: 2025
 */

#include "BikeCANProtocol.h"
#include <CAN.h>

// ===== ENCODER IMPLEMENTATIONS =====

void BikeCANEncoder::encodeECUData(const ECUData& ecu, uint8_t* canData) {
    // Clear buffer
    memset(canData, 0, 8);
    
    // Byte 0-1: Temperature * 100 (int16_t)
    int16_t temp = (int16_t)(ecu.temperature * 100);
    canData[0] = temp & 0xFF;
    canData[1] = (temp >> 8) & 0xFF;
    
    // Byte 2: Status flags
    canData[2] = 0;
    if (ecu.overTemperature) canData[2] |= 0x01;
}

void BikeCANEncoder::encodeMotorData(const MotorData& motor, uint8_t* canData) {
    memset(canData, 0, 8);
    
    // Byte 0-1: Temperature * 100
    int16_t temp = (int16_t)(motor.temperature * 100);
    canData[0] = temp & 0xFF;
    canData[1] = (temp >> 8) & 0xFF;
    
    // Byte 2-3: Current * 100
    int16_t current = (int16_t)(motor.current * 100);
    canData[2] = current & 0xFF;
    canData[3] = (current >> 8) & 0xFF;
    
    // Byte 4: Status flags
    canData[4] = 0;
    if (motor.overTemperature) canData[4] |= 0x01;
    if (motor.overCurrent) canData[4] |= 0x02;
}

void BikeCANEncoder::encodeBatteryVoltageSOC(const BatteryData& battery, uint8_t* canData) {
    memset(canData, 0, 8);
    
    // Byte 0-1: Voltage * 100 (uint16_t)
    uint16_t voltage = (uint16_t)(battery.voltage * 100);
    canData[0] = voltage & 0xFF;
    canData[1] = (voltage >> 8) & 0xFF;
    
    // Byte 2: SOC
    canData[2] = battery.soc;
    
    // Byte 3-4: Cell Delta * 10000 (for high precision)
    uint16_t cellDelta = (uint16_t)(battery.cellVoltageDelta * 10000);
    canData[3] = cellDelta & 0xFF;
    canData[4] = (cellDelta >> 8) & 0xFF;
}

void BikeCANEncoder::encodeBatteryTempCurrent(const BatteryData& battery, uint8_t* canData) {
    memset(canData, 0, 8);
    
    // Byte 0-1: Temperature * 100
    int16_t temp = (int16_t)(battery.temperature * 100);
    canData[0] = temp & 0xFF;
    canData[1] = (temp >> 8) & 0xFF;
    
    // Byte 2-3: Current * 100
    int16_t current = (int16_t)(battery.current * 100);
    canData[2] = current & 0xFF;
    canData[3] = (current >> 8) & 0xFF;
    
    // Byte 4: Connection status
    canData[4] = battery.connected ? 0x01 : 0x00;
}

void BikeCANEncoder::encodeBatteryStatus(const BatteryData& battery, uint8_t* canData) {
    memset(canData, 0, 8);
    
    // Byte 0: Status flags
    canData[0] = 0;
    if (battery.charging) canData[0] |= 0x01;
    if (battery.discharging) canData[0] |= 0x02;
    
    // Byte 1: Error flags
    canData[1] = 0;
    if (battery.overVoltage) canData[1] |= 0x01;
    if (battery.underVoltage) canData[1] |= 0x02;
    if (battery.overTemperature) canData[1] |= 0x04;
    if (battery.overCurrent) canData[1] |= 0x08;
}

void BikeCANEncoder::encodeSignalsBLE(const SignalData& signals, BLEStatus bleStatus, 
                                     BatterySystemStatus batteryStatus, uint8_t* canData) {
    memset(canData, 0, 8);
    
    // Byte 0: Signal flags
    canData[0] = 0;
    if (signals.leftTurn) canData[0] |= 0x01;
    if (signals.rightTurn) canData[0] |= 0x02;
    if (signals.hazard) canData[0] |= 0x04;
    
    // Byte 1: BLE status
    canData[1] = (uint8_t)bleStatus;
    
    // Byte 2: Battery system status
    canData[2] = (uint8_t)batteryStatus;
}

void BikeCANEncoder::encodeSystemStatus(bool systemHealthy, uint8_t errorCount, 
                                       uint16_t errorFlags, uint8_t* canData) {
    memset(canData, 0, 8);
    
    // Byte 0: System healthy flag + error count
    canData[0] = (systemHealthy ? 0x80 : 0x00) | (errorCount & 0x7F);
    
    // Byte 1-2: Error flags
    canData[1] = errorFlags & 0xFF;
    canData[2] = (errorFlags >> 8) & 0xFF;
}

void BikeCANEncoder::encodeHeartbeat(uint32_t timestamp, uint16_t sequence, 
                                    float totalVoltage, uint8_t* canData) {
    memset(canData, 0, 8);
    
    // Byte 0-3: Timestamp
    canData[0] = timestamp & 0xFF;
    canData[1] = (timestamp >> 8) & 0xFF;
    canData[2] = (timestamp >> 16) & 0xFF;
    canData[3] = (timestamp >> 24) & 0xFF;
    
    // Byte 4-5: Sequence number
    canData[4] = sequence & 0xFF;
    canData[5] = (sequence >> 8) & 0xFF;
    
    // Byte 6-7: Total voltage * 10
    uint16_t voltage = (uint16_t)(totalVoltage * 10);
    canData[6] = voltage & 0xFF;
    canData[7] = (voltage >> 8) & 0xFF;
}

// ===== DECODER IMPLEMENTATIONS =====

void BikeCANDecoder::decodeECUData(const uint8_t* canData, ECUData& ecu) {
    // Byte 0-1: Temperature
    int16_t temp = (int16_t)(canData[0] | (canData[1] << 8));
    ecu.temperature = temp / 100.0f;
    
    // Byte 2: Status flags
    ecu.overTemperature = (canData[2] & 0x01) != 0;
}

void BikeCANDecoder::decodeMotorData(const uint8_t* canData, MotorData& motor) {
    // Byte 0-1: Temperature
    int16_t temp = (int16_t)(canData[0] | (canData[1] << 8));
    motor.temperature = temp / 100.0f;
    
    // Byte 2-3: Current
    int16_t current = (int16_t)(canData[2] | (canData[3] << 8));
    motor.current = current / 100.0f;
    
    // Byte 4: Status flags
    motor.overTemperature = (canData[4] & 0x01) != 0;
    motor.overCurrent = (canData[4] & 0x02) != 0;
}

void BikeCANDecoder::decodeBatteryVoltageSOC(const uint8_t* canData, BatteryData& battery) {
    // Byte 0-1: Voltage
    uint16_t voltage = canData[0] | (canData[1] << 8);
    battery.voltage = voltage / 100.0f;
    
    // Byte 2: SOC
    battery.soc = canData[2];
    
    // Byte 3-4: Cell Delta
    uint16_t cellDelta = canData[3] | (canData[4] << 8);
    battery.cellVoltageDelta = cellDelta / 10000.0f;
}

void BikeCANDecoder::decodeBatteryTempCurrent(const uint8_t* canData, BatteryData& battery) {
    // Byte 0-1: Temperature
    int16_t temp = (int16_t)(canData[0] | (canData[1] << 8));
    battery.temperature = temp / 100.0f;
    
    // Byte 2-3: Current
    int16_t current = (int16_t)(canData[2] | (canData[3] << 8));
    battery.current = current / 100.0f;
    
    // Byte 4: Connection status
    battery.connected = (canData[4] & 0x01) != 0;
}

void BikeCANDecoder::decodeBatteryStatus(const uint8_t* canData, BatteryData& battery) {
    // Byte 0: Status flags
    battery.charging = (canData[0] & 0x01) != 0;
    battery.discharging = (canData[0] & 0x02) != 0;
    
    // Byte 1: Error flags
    battery.overVoltage = (canData[1] & 0x01) != 0;
    battery.underVoltage = (canData[1] & 0x02) != 0;
    battery.overTemperature = (canData[1] & 0x04) != 0;
    battery.overCurrent = (canData[1] & 0x08) != 0;
}

void BikeCANDecoder::decodeSignalsBLE(const uint8_t* canData, SignalData& signals, 
                                     BLEStatus& bleStatus, BatterySystemStatus& batteryStatus) {
    // Byte 0: Signal flags
    signals.leftTurn = (canData[0] & 0x01) != 0;
    signals.rightTurn = (canData[0] & 0x02) != 0;
    signals.hazard = (canData[0] & 0x04) != 0;
    
    // Byte 1: BLE status
    bleStatus = (BLEStatus)canData[1];
    
    // Byte 2: Battery system status
    batteryStatus = (BatterySystemStatus)canData[2];
}

void BikeCANDecoder::decodeSystemStatus(const uint8_t* canData, bool& systemHealthy, 
                                       uint8_t& errorCount, uint16_t& errorFlags) {
    // Byte 0: System healthy + error count
    systemHealthy = (canData[0] & 0x80) != 0;
    errorCount = canData[0] & 0x7F;
    
    // Byte 1-2: Error flags
    errorFlags = canData[1] | (canData[2] << 8);
}

void BikeCANDecoder::decodeHeartbeat(const uint8_t* canData, uint32_t& timestamp, 
                                    uint16_t& sequence, float& totalVoltage) {
    // Byte 0-3: Timestamp
    timestamp = canData[0] | (canData[1] << 8) | (canData[2] << 16) | (canData[3] << 24);
    
    // Byte 4-5: Sequence
    sequence = canData[4] | (canData[5] << 8);
    
    // Byte 6-7: Total voltage
    uint16_t voltage = canData[6] | (canData[7] << 8);
    totalVoltage = voltage / 10.0f;
}

// ===== MANAGER IMPLEMENTATIONS =====

BikeCANManager::BikeCANManager() : 
    sequenceNumber(0), 
    receivedFlags(0),
    lastCompleteUpdate(0),
    totalFramesSent(0),
    totalFramesReceived(0),
    transmissionErrors(0) {
    
    memset(lastTransmission, 0, sizeof(lastTransmission));
    memset(&receivedData, 0, sizeof(receivedData));
}

bool BikeCANManager::begin(uint32_t canSpeed) {
    return CAN.begin(canSpeed);
}

bool BikeCANManager::sendBikeData(const BikeDisplayData& data) {
    bool success = true;
    uint8_t canFrame[8];
    
    // Send all data types with their respective intervals
    unsigned long currentTime = millis();
    
    // ECU Data
    if (shouldTransmit(0, TransmissionIntervals::ECU_DATA)) {
        BikeCANEncoder::encodeECUData(data.ecu, canFrame);
        success &= sendCANFrame(CANMessageIDs::ECU_DATA, canFrame, 8);
    }
    
    // Motor Data
    if (shouldTransmit(1, TransmissionIntervals::MOTOR_DATA)) {
        BikeCANEncoder::encodeMotorData(data.motor, canFrame);
        success &= sendCANFrame(CANMessageIDs::MOTOR_DATA, canFrame, 8);
    }
    
    // Battery 1 Data (3 frames)
    if (shouldTransmit(2, TransmissionIntervals::BATTERY_DATA)) {
        BikeCANEncoder::encodeBatteryVoltageSOC(data.bat1, canFrame);
        success &= sendCANFrame(CANMessageIDs::BAT1_VOLTAGE_SOC, canFrame, 8);
        
        BikeCANEncoder::encodeBatteryTempCurrent(data.bat1, canFrame);
        success &= sendCANFrame(CANMessageIDs::BAT1_TEMP_CURR, canFrame, 8);
        
        BikeCANEncoder::encodeBatteryStatus(data.bat1, canFrame);
        success &= sendCANFrame(CANMessageIDs::BAT1_STATUS, canFrame, 8);
    }
    
    // Battery 2 Data (3 frames)
    if (shouldTransmit(3, TransmissionIntervals::BATTERY_DATA)) {
        BikeCANEncoder::encodeBatteryVoltageSOC(data.bat2, canFrame);
        success &= sendCANFrame(CANMessageIDs::BAT2_VOLTAGE_SOC, canFrame, 8);
        
        BikeCANEncoder::encodeBatteryTempCurrent(data.bat2, canFrame);
        success &= sendCANFrame(CANMessageIDs::BAT2_TEMP_CURR, canFrame, 8);
        
        BikeCANEncoder::encodeBatteryStatus(data.bat2, canFrame);
        success &= sendCANFrame(CANMessageIDs::BAT2_STATUS, canFrame, 8);
    }
    
    // Signals + BLE (high priority)
    if (shouldTransmit(4, TransmissionIntervals::SIGNALS_BLE)) {
        BikeCANEncoder::encodeSignalsBLE(data.signals, data.bleStatus, data.batterySystemStatus, canFrame);
        success &= sendCANFrame(CANMessageIDs::SIGNALS_BLE, canFrame, 8);
    }
    
    // System Status
    if (shouldTransmit(5, TransmissionIntervals::SYSTEM_STATUS)) {
        // Need to create error flags from SystemErrors struct
        uint16_t errorFlags = 0; // This would need to be extracted from actual errors
        BikeCANEncoder::encodeSystemStatus(data.systemHealthy, data.errorCount, errorFlags, canFrame);
        success &= sendCANFrame(CANMessageIDs::SYSTEM_STATUS, canFrame, 8);
    }
    
    // Heartbeat
    if (shouldTransmit(6, TransmissionIntervals::HEARTBEAT)) {
        BikeCANEncoder::encodeHeartbeat(data.timestamp, data.sequence, data.totalVoltage, canFrame);
        success &= sendCANFrame(CANMessageIDs::HEARTBEAT, canFrame, 8);
    }
    
    return success;
}

bool BikeCANManager::sendCANFrame(uint16_t messageId, const uint8_t* data, uint8_t length) {
    CAN.beginPacket(messageId);
    
    for (uint8_t i = 0; i < length; i++) {
        CAN.write(data[i]);
    }
    
    if (CAN.endPacket()) {
        totalFramesSent++;
        return true;
    } else {
        transmissionErrors++;
        return false;
    }
}

bool BikeCANManager::receiveData(BikeDisplayData& data) {
    uint16_t messageId;
    uint8_t canData[8];
    uint8_t length;
    
    while (receiveCANFrame(messageId, canData, length)) {
        totalFramesReceived++;
        
        // Decode based on message ID
        switch (messageId) {
            case CANMessageIDs::ECU_DATA:
                BikeCANDecoder::decodeECUData(canData, receivedData.ecu);
                receivedFlags |= MessageFlags::ECU_RECEIVED;
                break;
                
            case CANMessageIDs::MOTOR_DATA:
                BikeCANDecoder::decodeMotorData(canData, receivedData.motor);
                receivedFlags |= MessageFlags::MOTOR_RECEIVED;
                break;
                
            case CANMessageIDs::BAT1_VOLTAGE_SOC:
            case CANMessageIDs::BAT1_TEMP_CURR:
            case CANMessageIDs::BAT1_STATUS:
                // Decode appropriate part of battery 1 data
                if (messageId == CANMessageIDs::BAT1_VOLTAGE_SOC) {
                    BikeCANDecoder::decodeBatteryVoltageSOC(canData, receivedData.bat1);
                } else if (messageId == CANMessageIDs::BAT1_TEMP_CURR) {
                    BikeCANDecoder::decodeBatteryTempCurrent(canData, receivedData.bat1);
                } else {
                    BikeCANDecoder::decodeBatteryStatus(canData, receivedData.bat1);
                }
                receivedFlags |= MessageFlags::BAT1_RECEIVED;
                break;
                
            case CANMessageIDs::BAT2_VOLTAGE_SOC:
            case CANMessageIDs::BAT2_TEMP_CURR:
            case CANMessageIDs::BAT2_STATUS:
                // Similar for battery 2
                if (messageId == CANMessageIDs::BAT2_VOLTAGE_SOC) {
                    BikeCANDecoder::decodeBatteryVoltageSOC(canData, receivedData.bat2);
                } else if (messageId == CANMessageIDs::BAT2_TEMP_CURR) {
                    BikeCANDecoder::decodeBatteryTempCurrent(canData, receivedData.bat2);
                } else {
                    BikeCANDecoder::decodeBatteryStatus(canData, receivedData.bat2);
                }
                receivedFlags |= MessageFlags::BAT2_RECEIVED;
                break;
                
            case CANMessageIDs::SIGNALS_BLE:
                BikeCANDecoder::decodeSignalsBLE(canData, receivedData.signals, 
                                               receivedData.bleStatus, receivedData.batterySystemStatus);
                receivedFlags |= MessageFlags::SIGNALS_RECEIVED;
                break;
                
            case CANMessageIDs::SYSTEM_STATUS:
                uint16_t errorFlags;
                BikeCANDecoder::decodeSystemStatus(canData, receivedData.systemHealthy, 
                                                 receivedData.errorCount, errorFlags);
                receivedFlags |= MessageFlags::SYSTEM_RECEIVED;
                break;
                
            case CANMessageIDs::HEARTBEAT:
                BikeCANDecoder::decodeHeartbeat(canData, receivedData.timestamp, 
                                              receivedData.sequence, receivedData.totalVoltage);
                receivedFlags |= MessageFlags::HEARTBEAT_RECEIVED;
                break;
        }
    }
    
    // Check if we have complete data
    if (isDataComplete()) {
        data = receivedData;
        lastCompleteUpdate = millis();
        return true;
    }
    
    return false;
}

bool BikeCANManager::receiveCANFrame(uint16_t& messageId, uint8_t* data, uint8_t& length) {
    int packetSize = CAN.parsePacket();
    
    if (packetSize > 0) {
        messageId = CAN.packetId();
        length = 0;
        
        while (CAN.available() && length < 8) {
            data[length++] = CAN.read();
        }
        
        return true;
    }
    
    return false;
}

bool BikeCANManager::isDataComplete() const {
    // Check if we have received all essential message types
    const uint8_t essentialFlags = MessageFlags::HEARTBEAT_RECEIVED | 
                                  MessageFlags::SIGNALS_RECEIVED |
                                  MessageFlags::BAT1_RECEIVED |
                                  MessageFlags::BAT2_RECEIVED;
    
    return (receivedFlags & essentialFlags) == essentialFlags;
}

unsigned long BikeCANManager::getDataAge() const {
    return millis() - lastCompleteUpdate;
}

bool BikeCANManager::shouldTransmit(uint8_t messageType, unsigned long interval) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastTransmission[messageType] >= interval) {
        lastTransmission[messageType] = currentTime;
        return true;
    }
    
    return false;
}