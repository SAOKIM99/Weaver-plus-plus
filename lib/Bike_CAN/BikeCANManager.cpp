#include "BikeCANManager.h"

BikeCANManager::BikeCANManager() : 
    initialized(false),
    messagesSent(0),
    messagesReceived(0),
    sendSequence(0),
    receiveCallback(nullptr) {
}

BikeCANManager::~BikeCANManager() {
}

bool BikeCANManager::begin(int txPin, int rxPin) {
    Serial.println("=== CAN Manager Initialization ===");
    
    // Set CAN pins
    CAN.setPins(rxPin, txPin);
    
    // Initialize CAN bus
    if (!CAN.begin(CAN_SPEED)) {
        Serial.println("❌ CAN initialization failed!");
        initialized = false;
        return false;
    }
    
    initialized = true;
    messagesSent = 0;
    messagesReceived = 0;
    sendSequence = 0;
    
    Serial.println("✅ CAN Manager initialized successfully");
    Serial.printf("CAN Speed: %.0f kbps\n", CAN_SPEED / 1000.0f);
    Serial.printf("TX Pin: %d, RX Pin: %d\n", txPin, rxPin);
    
    return true;
}

void BikeCANManager::update() {
    if (!initialized) return;
    
    // Check for incoming messages
    int packetSize = CAN.parsePacket();
    if (packetSize > 0) {
        uint32_t id = CAN.packetId();
        uint8_t data[8];
        uint8_t length = 0;
        
        // Read data
        while (CAN.available() && length < 8) {
            data[length++] = CAN.read();
        }
        
        messagesReceived++;
        
        // Call callback if registered
        if (receiveCallback) {
            receiveCallback(id, data, length);
        }
    }
}

bool BikeCANManager::sendBikeStatus(const BikeStatus& status, bool bikeUnlocked, bool bleConnected) {
    if (!initialized) return false;
    
    CAN.beginPacket(MSG_ID_BIKE_STATUS);
    CAN.write(status.operationState);           // Bike operation state
    CAN.write(bikeUnlocked ? 1 : 0);            // Unlock status
    CAN.write(bleConnected ? 1 : 0);            // BLE status
    CAN.write((uint8_t)(status.bikeSpeed));     // Speed (km/h)
    
    // Status flags byte (key, brake, charging, signals)
    uint8_t flags = 0;
    if (status.keyOn) flags |= 0x01;
    if (status.brakePressed) flags |= 0x02;
    if (status.bms1.isCharging || status.bms2.isCharging) flags |= 0x04;
    if (status.leftSignal) flags |= 0x08;
    if (status.rightSignal) flags |= 0x10;
    CAN.write(flags);
    
    // Reserved bytes
    CAN.write(0x00);
    CAN.write(0x00);
    
    if (CAN.endPacket()) {
        messagesSent++;
        return true;
    }
    
    return false;
}

bool BikeCANManager::sendBMSData(const BMSData& bms, uint8_t bmsId) {
    if (!initialized) return false;
    
    CAN.beginPacket(MSG_ID_BMS_DATA + bmsId);
    
    // Pack voltage (2 bytes) - multiply by 100 to preserve 2 decimal places
    uint16_t voltage = (uint16_t)(bms.voltage * 100);
    CAN.write(voltage >> 8);
    CAN.write(voltage & 0xFF);
    
    // Pack current (2 bytes) - multiply by 100, handle negative values
    int16_t current = (int16_t)(bms.current * 100);
    CAN.write(current >> 8);
    CAN.write(current & 0xFF);
    
    // SOC percentage (1 byte)
    CAN.write(bms.soc);
    
    // Calculate maximum temperature from all available sensors
    float maxTemp = bms.temperature;  // Battery temperature (primary)
    
    // Check power temperature (MOSFET temperature)
    if (bms.powerTemp > -50.0f && bms.powerTemp < 150.0f) {  // Valid range check
        maxTemp = max(maxTemp, bms.powerTemp);
    }
    
    // Check box temperature (BMS controller temperature)  
    if (bms.boxTemp > -50.0f && bms.boxTemp < 150.0f) {  // Valid range check
        maxTemp = max(maxTemp, bms.boxTemp);
    }
    
    // Temperature (1 byte) - send maximum temperature with offset
    CAN.write((uint8_t)(maxTemp + 50));
    
    // Debug: Log temperature being sent via CAN (simplified)
    // Serial.printf("📤 [CAN-BMS%d] Temp: %.1f°C\n", bmsId, maxTemp);
    
    // Status flags (2 bytes)
    uint8_t status1 = 0;
    status1 |= (bms.connected ? 0x01 : 0x00);
    status1 |= (bms.isCharging ? 0x02 : 0x00);
    status1 |= (bms.isDischarging ? 0x04 : 0x00);
    status1 |= (bms.chargingEnabled ? 0x08 : 0x00);
    status1 |= (bms.dischargingEnabled ? 0x10 : 0x00);
    
    CAN.write(status1);
    CAN.write(bms.numCells);
    
    if (CAN.endPacket()) {
        messagesSent++;
        return true;
    }
    
    return false;
}

bool BikeCANManager::sendVESCData(const VESCData& vesc) {
    if (!initialized) return false;
    
    CAN.beginPacket(MSG_ID_VESC_DATA);
    
    // Motor RPM (2 bytes) - divide by 10 to fit in 16-bit
    int16_t rpm = (int16_t)(vesc.motorRPM / 10);
    CAN.write(rpm >> 8);
    CAN.write(rpm & 0xFF);
    
    // Input voltage (2 bytes) - multiply by 100
    uint16_t voltage = (uint16_t)(vesc.inputVoltage * 100);
    CAN.write(voltage >> 8);
    CAN.write(voltage & 0xFF);
    
    // Motor current (2 bytes) - multiply by 100
    int16_t current = (int16_t)(vesc.motorCurrent * 100);
    CAN.write(current >> 8);
    CAN.write(current & 0xFF);
    
    // Temperature (2 bytes) - Motor and FET temps
    CAN.write((uint8_t)(vesc.tempMotor + 50));
    CAN.write((uint8_t)(vesc.tempFET + 50));
    
    if (CAN.endPacket()) {
        messagesSent++;
        return true;
    }
    
    return false;
}



void BikeCANManager::sendNextInSequence(const SharedBikeData& sharedData) {
    if (!initialized) return;
    
    bool success = false;
    
    // Convert to display data for additional info
    BikeDataDisplay displayData = convertToDisplayData(sharedData.sensorData, sharedData.bleConnected);
    
    switch (sendSequence % CAN_MSG_COUNT) {
        case CAN_MSG_BIKE_STATUS:
            success = sendBikeStatus(sharedData.sensorData, sharedData.bikeUnlocked, sharedData.bleConnected);
            break;
            
        case CAN_MSG_BMS1_DATA:
            success = sendBMSData(sharedData.sensorData.bms1, 1);
            break;
            
        case CAN_MSG_BMS2_DATA:
            success = sendBMSData(sharedData.sensorData.bms2, 2);
            break;
            
        case CAN_MSG_VESC_DATA:
            success = sendVESCData(sharedData.sensorData.vesc);
            break;
            
        case CAN_MSG_BATTERY_EXT:
            success = sendBatteryExtended(sharedData.sensorData.bms1, sharedData.sensorData.bms2);
            break;
            
        case CAN_MSG_DISTANCE_DATA:
            success = sendDistanceData(displayData.odometer, displayData.distance, displayData.tripDistance);
            break;
            
        case CAN_MSG_TIME_DATA:
            success = sendTimeData(displayData.time);
            break;
    }
    
    sendSequence++;
}

void BikeCANManager::setReceiveCallback(CANReceiveCallback callback) {
    receiveCallback = callback;
}



bool BikeCANManager::isInitialized() {
    return initialized;
}

uint32_t BikeCANManager::getMessagesSent() {
    return messagesSent;
}

uint32_t BikeCANManager::getMessagesReceived() {
    return messagesReceived;
}



bool BikeCANManager::sendBatteryExtended(const BMSData& bms1, const BMSData& bms2) {
    if (!initialized) return false;
    
    CAN.beginPacket(MSG_ID_BATTERY_EXT);
    
    // Battery differential voltages (already in mV, clip to 255mV max)
    uint8_t bms1DeltaByte = (bms1.cellVoltageDelta > 255) ? 255 : (uint8_t)bms1.cellVoltageDelta;
    uint8_t bms2DeltaByte = (bms2.cellVoltageDelta > 255) ? 255 : (uint8_t)bms2.cellVoltageDelta;
    
    CAN.write(bms1DeltaByte); 
    CAN.write(bms2DeltaByte);
    
    // Calculate maximum temperatures for each BMS
    float maxTemp1 = bms1.temperature;
    if (bms1.powerTemp > -50.0f && bms1.powerTemp < 150.0f) maxTemp1 = max(maxTemp1, bms1.powerTemp);
    if (bms1.boxTemp > -50.0f && bms1.boxTemp < 150.0f) maxTemp1 = max(maxTemp1, bms1.boxTemp);
    
    float maxTemp2 = bms2.temperature;
    if (bms2.powerTemp > -50.0f && bms2.powerTemp < 150.0f) maxTemp2 = max(maxTemp2, bms2.powerTemp);
    if (bms2.boxTemp > -50.0f && bms2.boxTemp < 150.0f) maxTemp2 = max(maxTemp2, bms2.boxTemp);
    
    // Battery maximum temperatures (1 byte each + 40 offset)
    CAN.write((uint8_t)(maxTemp1 + 40));
    CAN.write((uint8_t)(maxTemp2 + 40));
    
    // Debug: Log extended temperatures (simplified)
    // Serial.printf("📤 [CAN-EXT] BMS1: %.1f°C, BMS2: %.1f°C\n", maxTemp1, maxTemp2);
    
    // Power calculations (2 bytes)
    int16_t motorPower = (int16_t)((bms1.voltage + bms2.voltage) * (bms1.current + bms2.current) / 2);
    CAN.write(motorPower >> 8);
    CAN.write(motorPower & 0xFF);
    
    // Status flags
    uint8_t flags = 0;
    if (bms1.isCharging || bms2.isCharging) flags |= 0x01;
    if (bms1.connected) flags |= 0x02;
    if (bms2.connected) flags |= 0x04;
    CAN.write(flags);
    
    if (CAN.endPacket()) {
        messagesSent++;
        return true;
    }
    
    return false;
}

bool BikeCANManager::sendDistanceData(float odometer, float distance, float tripDistance) {
    if (!initialized) return false;
    
    CAN.beginPacket(MSG_ID_DISTANCE_DATA);
    
    // Odometer (4 bytes, km * 100)
    uint32_t odometerScaled = (uint32_t)(odometer * 100);
    CAN.write((odometerScaled >> 24) & 0xFF);
    CAN.write((odometerScaled >> 16) & 0xFF);
    CAN.write((odometerScaled >> 8) & 0xFF);
    CAN.write(odometerScaled & 0xFF);
    
    // Current distance (2 bytes, km * 100)
    uint16_t distanceScaled = (uint16_t)(distance * 100);
    CAN.write((distanceScaled >> 8) & 0xFF);
    CAN.write(distanceScaled & 0xFF);
    
    // Trip distance (2 bytes, km * 100)
    uint16_t tripScaled = (uint16_t)(tripDistance * 100);
    CAN.write((tripScaled >> 8) & 0xFF);
    CAN.write(tripScaled & 0xFF);
    
    if (CAN.endPacket()) {
        messagesSent++;
        return true;
    }
    
    return false;
}

bool BikeCANManager::sendTimeData(int time) {
    if (!initialized) return false;
    
    CAN.beginPacket(MSG_ID_TIME_DATA);
    
    // Total time (4 bytes, seconds)
    CAN.write((time >> 24) & 0xFF);
    CAN.write((time >> 16) & 0xFF);
    CAN.write((time >> 8) & 0xFF);
    CAN.write(time & 0xFF);
    
    // Reserved bytes for future use
    CAN.write(0x00);
    CAN.write(0x00);
    CAN.write(0x00);
    CAN.write(0x00);
    
    if (CAN.endPacket()) {
        messagesSent++;
        return true;
    }
    
    return false;
}

// =============================================================================
// CAN MESSAGE PARSING FUNCTIONS
// =============================================================================

bool BikeCANManager::parseBikeStatus(uint8_t* data, uint8_t length, BikeStatus& status, bool& bikeUnlocked, bool& bleConnected) {
    if (!data || length < 7) return false;
    
    // Debug raw data (disabled)
    // Serial.printf("🔍 [parseBikeStatus] Raw: [%d][%d][%d][%d][%d]\n", data[0], data[1], data[2], data[3], data[4]);
    
    status.operationState = (BikeOperationState)data[0];
    bikeUnlocked = (data[1] == 1);
    bleConnected = (data[2] == 1);
    status.bikeSpeed = (float)data[3];
    
    // Debug parsed data (disabled)
    // Serial.printf("🔍 [parseBikeStatus] BLE: %s, Speed: %.0f\n", bleConnected ? "ON" : "OFF", status.bikeSpeed);
    
    // Parse status flags
    uint8_t flags = data[4];
    status.keyOn = (flags & 0x01) != 0;
    status.brakePressed = (flags & 0x02) != 0;
    // Charging status is in flags but will be parsed from BMS data
    status.leftSignal = (flags & 0x08) != 0;
    status.rightSignal = (flags & 0x10) != 0;
    
    return true;
}

bool BikeCANManager::parseBMSData(uint8_t* data, uint8_t length, BMSData& bms) {
    if (!data || length < 8) return false;
    
    // Voltage (2 bytes)
    uint16_t voltage = (data[0] << 8) | data[1];
    bms.voltage = voltage / 100.0f;
    
    // Current (2 bytes, signed)
    int16_t current = (data[2] << 8) | data[3];
    bms.current = current / 100.0f;
    
    // SOC
    bms.soc = data[4];
    
    // Temperature (with offset)
    bms.temperature = (float)data[5] - 50.0f;
    
    // Status flags
    uint8_t status1 = data[6];
    bms.connected = (status1 & 0x01) != 0;
    bms.isCharging = (status1 & 0x02) != 0;
    bms.isDischarging = (status1 & 0x04) != 0;
    bms.chargingEnabled = (status1 & 0x08) != 0;
    bms.dischargingEnabled = (status1 & 0x10) != 0;
    
    // Number of cells
    bms.numCells = data[7];
    
    return true;
}

bool BikeCANManager::parseVESCData(uint8_t* data, uint8_t length, VESCData& vesc) {
    if (!data || length < 8) return false;
    
    // Motor RPM (2 bytes, multiply by 10 to restore)
    int16_t rpm = (data[0] << 8) | data[1];
    vesc.motorRPM = rpm * 10.0f;
    
    // Input voltage (2 bytes)
    uint16_t voltage = (data[2] << 8) | data[3];
    vesc.inputVoltage = voltage / 100.0f;
    
    // Motor current (2 bytes, signed)
    int16_t current = (data[4] << 8) | data[5];
    vesc.motorCurrent = current / 100.0f;
    
    // Temperatures (with offset)
    vesc.tempMotor = (float)data[6] - 50.0f;
    vesc.tempFET = (float)data[7] - 50.0f;
    
    vesc.connected = true; // If we receive data, assume connected
    
    return true;
}

bool BikeCANManager::parseBatteryExtended(uint8_t* data, uint8_t length, BMSData& bms1, BMSData& bms2, int& motorPower) {
    if (!data) {
        Serial.println("❌ [parseBatteryExtended] data is NULL");
        return false;
    }
    if (length < 7) {
        Serial.printf("❌ [parseBatteryExtended] length=%d < 7\n", length);
        return false;
    }
    
    // Cell voltage deltas (keep in mV)
    bms1.cellVoltageDelta = (uint16_t)data[0];
    bms2.cellVoltageDelta = (uint16_t)data[1];
    
    // Temperatures (with offset)
    bms1.temperature = (float)data[2] - 40.0f;
    bms2.temperature = (float)data[3] - 40.0f;
    
    // Motor power (2 bytes, signed)
    int16_t power = (data[4] << 8) | data[5];
    motorPower = power;
    
    // Status flags
    uint8_t flags = data[6];
    bool isCharging = (flags & 0x01) != 0;
    bms1.connected = (flags & 0x02) != 0;
    bms2.connected = (flags & 0x04) != 0;
    
    // Update charging status
    if (isCharging) {
        bms1.isCharging = true;
        bms2.isCharging = true;
    }
    
    return true;
}

bool BikeCANManager::parseDistanceData(uint8_t* data, uint8_t length, float& odometer, float& distance, float& tripDistance) {
    if (!data || length < 8) return false;
    
    // Odometer (4 bytes)
    uint32_t odometerScaled = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    odometer = odometerScaled / 100.0f;
    
    // Current distance (2 bytes)
    uint16_t distanceScaled = (data[4] << 8) | data[5];
    distance = distanceScaled / 100.0f;
    
    // Trip distance (2 bytes)
    uint16_t tripScaled = (data[6] << 8) | data[7];
    tripDistance = tripScaled / 100.0f;
    
    return true;
}

bool BikeCANManager::parseTimeData(uint8_t* data, uint8_t length, int& time) {
    if (!data || length < 8) return false;
    
    // Total time (4 bytes)
    time = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    
    return true;
}

bool BikeCANManager::parseCANMessage(uint32_t id, uint8_t* data, uint8_t length, BikeDataDisplay& displayData) {
    if (!data) return false;
    
    bool success = false;
    
    switch(id) {
        case MSG_ID_BIKE_STATUS: {
            BikeStatus tempStatus;
            bool bikeUnlocked, bleConnected;
            success = parseBikeStatus(data, length, tempStatus, bikeUnlocked, bleConnected);
            if (success) {
                displayData.speed = tempStatus.bikeSpeed;
                displayData.bluetoothConnected = bleConnected;
                displayData.turnLeftActive = tempStatus.leftSignal;
                displayData.turnRightActive = tempStatus.rightSignal;
            }
            break;
        }
        
        case MSG_ID_BMS_DATA + 1: { // BMS1
            BMSData tempBMS;
            success = parseBMSData(data, length, tempBMS);
            if (success) {
                displayData.battery1Volt = tempBMS.voltage;
                displayData.battery1Percent = tempBMS.soc;
                displayData.battery1Temp = (int)tempBMS.temperature;
                displayData.battery1Current = tempBMS.current;
                displayData.isCharging = tempBMS.isCharging;
                
                // Debug: Log BMS1 temperature (simplified)
                // Serial.printf("🌡️ [CAN-BMS1] %.1f°C\n", tempBMS.temperature);
            }
            break;
        }
        
        case MSG_ID_BMS_DATA + 2: { // BMS2
            BMSData tempBMS;
            success = parseBMSData(data, length, tempBMS);
            if (success) {
                displayData.battery2Volt = tempBMS.voltage;
                displayData.battery2Percent = tempBMS.soc;
                displayData.battery2Temp = (int)tempBMS.temperature;
                displayData.battery2Current = tempBMS.current;
                if (tempBMS.isCharging) displayData.isCharging = true;
                
                // Debug: Log BMS2 temperature (simplified)
                // Serial.printf("🌡️ [CAN-BMS2] %.1f°C\n", tempBMS.temperature);
            }
            break;
        }
        
        case MSG_ID_VESC_DATA: {
            VESCData tempVESC;
            success = parseVESCData(data, length, tempVESC);
            if (success) {
                displayData.motorTemp = (int)tempVESC.tempMotor;
                displayData.ecuTemp = (int)tempVESC.tempFET;
                displayData.motorCurrent = tempVESC.motorCurrent;
                
                // Debug: Log VESC temperatures (simplified)
                // Serial.printf("🌡️ [CAN-VESC] Motor: %.1f°C, FET: %.1f°C\n", tempVESC.tempMotor, tempVESC.tempFET);
            }
            break;
        }
        
        case MSG_ID_BATTERY_EXT: {
            BMSData tempBMS1, tempBMS2;
            int motorPower;
            success = parseBatteryExtended(data, length, tempBMS1, tempBMS2, motorPower);
            if (success) {
                displayData.battery1DiffVolt = tempBMS1.cellVoltageDelta;
                displayData.battery2DiffVolt = tempBMS2.cellVoltageDelta;
                displayData.motorPower = motorPower;
                
                // Debug: Log extended temperatures (simplified)
                // Serial.printf("🌡️ [CAN-EXT] BMS1: %.1f°C, BMS2: %.1f°C\n", tempBMS1.temperature, tempBMS2.temperature);
            } else {
                Serial.println("❌ [parseCANMessage] Failed to parse MSG_ID_BATTERY_EXT");
            }
            break;
        }
        
        case MSG_ID_DISTANCE_DATA: {
            success = parseDistanceData(data, length, displayData.odometer, displayData.distance, displayData.tripDistance);
            break;
        }
        
        case MSG_ID_TIME_DATA: {
            success = parseTimeData(data, length, displayData.time);
            break;
        }
        
        default:
            Serial.printf("[CAN] Unknown message ID: 0x%03X\n", id);
            return false;
    }
    
    if (success) {
        // Update calculated values
        displayData.batteryPercent = (displayData.battery1Percent + displayData.battery2Percent) / 2;
        displayData.batteryVoltage = (displayData.battery1Volt + displayData.battery2Volt) / 2.0f;
        displayData.voltage = displayData.batteryVoltage;
        displayData.current = (displayData.battery1Current + displayData.battery2Current) / 2.0f;
    }
    
    return success;
}

