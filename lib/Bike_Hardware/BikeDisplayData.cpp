/*
 * Bike Display Data Helper Implementation
 * 
 * Implementation của các helper functions cho BikeDisplayData
 * 
 * Author: SAOKIM99
 * Date: 2025
 */

#include "BikeDisplayData.h"

// Calculate simple checksum for data integrity
uint16_t BikeDisplayDataHelper::calculateChecksum(const BikeDisplayData& data) {
    uint16_t checksum = 0;
    const uint8_t* ptr = (const uint8_t*)&data;
    size_t size = sizeof(BikeDisplayData);
    
    for (size_t i = 0; i < size; i++) {
        checksum += ptr[i];
    }
    
    return checksum;
}

// Validate data package integrity
bool BikeDisplayDataHelper::validatePackage(const DisplayDataPackage& package) {
    uint16_t calculatedChecksum = calculateChecksum(package.data);
    return (calculatedChecksum == package.checksum);
}

// Convert to JSON string for debugging/logging
String BikeDisplayDataHelper::toJSON(const BikeDisplayData& data) {
    String json = "{";
    
    // Timestamp and sequence
    json += "\"timestamp\":" + String(data.timestamp) + ",";
    json += "\"sequence\":" + String(data.sequence) + ",";
    
    // ECU data
    json += "\"ecu\":{";
    json += "\"temperature\":" + String(data.ecu.temperature, 1) + ",";
    json += "\"overTemp\":" + String(data.ecu.overTemperature ? "true" : "false");
    json += "},";
    
    // Motor data
    json += "\"motor\":{";
    json += "\"temperature\":" + String(data.motor.temperature, 1) + ",";
    json += "\"current\":" + String(data.motor.current, 2) + ",";
    json += "\"overTemp\":" + String(data.motor.overTemperature ? "true" : "false") + ",";
    json += "\"overCurrent\":" + String(data.motor.overCurrent ? "true" : "false");
    json += "},";
    
    // Battery 1
    json += "\"bat1\":{";
    json += "\"voltage\":" + String(data.bat1.voltage, 2) + ",";
    json += "\"soc\":" + String(data.bat1.soc) + ",";
    json += "\"cellDelta\":" + String(data.bat1.cellVoltageDelta, 3) + ",";
    json += "\"temperature\":" + String(data.bat1.temperature, 1) + ",";
    json += "\"current\":" + String(data.bat1.current, 2) + ",";
    json += "\"connected\":" + String(data.bat1.connected ? "true" : "false");
    json += "},";
    
    // Battery 2
    json += "\"bat2\":{";
    json += "\"voltage\":" + String(data.bat2.voltage, 2) + ",";
    json += "\"soc\":" + String(data.bat2.soc) + ",";
    json += "\"cellDelta\":" + String(data.bat2.cellVoltageDelta, 3) + ",";
    json += "\"temperature\":" + String(data.bat2.temperature, 1) + ",";
    json += "\"current\":" + String(data.bat2.current, 2) + ",";
    json += "\"connected\":" + String(data.bat2.connected ? "true" : "false");
    json += "},";
    
    // Signals
    json += "\"signals\":{";
    json += "\"left\":" + String(data.signals.leftTurn ? "true" : "false") + ",";
    json += "\"right\":" + String(data.signals.rightTurn ? "true" : "false") + ",";
    json += "\"hazard\":" + String(data.signals.hazard ? "true" : "false");
    json += "},";
    
    // System status
    json += "\"bleStatus\":" + String(data.bleStatus) + ",";
    json += "\"batteryStatus\":" + String(data.batterySystemStatus) + ",";
    json += "\"systemHealthy\":" + String(data.systemHealthy ? "true" : "false") + ",";
    json += "\"errorCount\":" + String(data.errorCount);
    
    json += "}";
    return json;
}

// Check for critical system errors
bool BikeDisplayDataHelper::hasCriticalErrors(const SystemErrors& errors) {
    // Critical errors that require immediate attention
    return (errors.bits.ecuOverTemp || 
            errors.bits.motorOverTemp || 
            errors.bits.motorOverCurrent ||
            errors.bits.bat1Fault || 
            errors.bits.bat2Fault ||
            errors.bits.bat1OverVolt || 
            errors.bits.bat2OverVolt ||
            errors.bits.bat1UnderVolt || 
            errors.bits.bat2UnderVolt);
}

// Count total number of active errors
uint8_t BikeDisplayDataHelper::getErrorCount(const SystemErrors& errors) {
    uint8_t count = 0;
    uint16_t flags = errors.all;
    
    // Count set bits
    while (flags) {
        if (flags & 1) count++;
        flags >>= 1;
    }
    
    return count;
}

// Calculate battery system status based on individual batteries
BatterySystemStatus BikeDisplayDataHelper::calculateBatterySystemStatus(const BatteryData& bat1, const BatteryData& bat2) {
    // Check for faults first
    if (!bat1.connected && !bat2.connected) {
        return BATTERY_FAULT;
    }
    
    // Determine charging/discharging state
    bool anyCharging = (bat1.connected && bat1.charging) || (bat2.connected && bat2.charging);
    bool anyDischarging = (bat1.connected && bat1.discharging) || (bat2.connected && bat2.discharging);
    
    if (anyCharging && !anyDischarging) {
        return BATTERY_CHARGING;
    } else if (anyDischarging && !anyCharging) {
        return BATTERY_DISCHARGING;
    } else {
        return BATTERY_IDLE;
    }
}

// Calculate system totals from individual components
void BikeDisplayDataHelper::calculateSystemTotals(BikeDisplayData& data) {
    // Total voltage (series connection)
    data.totalVoltage = 0.0f;
    if (data.bat1.connected) data.totalVoltage += data.bat1.voltage;
    if (data.bat2.connected) data.totalVoltage += data.bat2.voltage;
    
    // Total current (average or sum depending on configuration)
    data.totalCurrent = 0.0f;
    uint8_t connectedBatteries = 0;
    
    if (data.bat1.connected) {
        data.totalCurrent += data.bat1.current;
        connectedBatteries++;
    }
    if (data.bat2.connected) {
        data.totalCurrent += data.bat2.current;
        connectedBatteries++;
    }
    
    // Average current for parallel configuration
    if (connectedBatteries > 0) {
        data.totalCurrent /= connectedBatteries;
    }
    
    // Total SOC (average)
    data.totalSOC = 0;
    if (connectedBatteries > 0) {
        uint16_t totalSOC = 0;
        if (data.bat1.connected) totalSOC += data.bat1.soc;
        if (data.bat2.connected) totalSOC += data.bat2.soc;
        data.totalSOC = totalSOC / connectedBatteries;
    }
    
    // Update battery system status
    data.batterySystemStatus = calculateBatterySystemStatus(data.bat1, data.bat2);
    
    // Update system health
    SystemErrors errors;
    errors.all = 0;
    
    // Check ECU
    errors.bits.ecuOverTemp = data.ecu.overTemperature;
    
    // Check Motor
    errors.bits.motorOverTemp = data.motor.overTemperature;
    errors.bits.motorOverCurrent = data.motor.overCurrent;
    
    // Check Batteries
    errors.bits.bat1Fault = !data.bat1.connected;
    errors.bits.bat2Fault = !data.bat2.connected;
    errors.bits.bat1OverTemp = data.bat1.overTemperature;
    errors.bits.bat2OverTemp = data.bat2.overTemperature;
    errors.bits.bat1OverVolt = data.bat1.overVoltage;
    errors.bits.bat2OverVolt = data.bat2.overVoltage;
    errors.bits.bat1UnderVolt = data.bat1.underVoltage;
    errors.bits.bat2UnderVolt = data.bat2.underVoltage;
    
    // Check BLE
    errors.bits.bleConnectionLost = (data.bleStatus == BLE_DISCONNECTED);
    
    // Update error count and system health
    data.errorCount = getErrorCount(errors);
    data.systemHealthy = !hasCriticalErrors(errors);
}