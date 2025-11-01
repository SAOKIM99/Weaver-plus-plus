#include "BikeSensorManager.h"

BikeSensorManager::BikeSensorManager() : 
    bms1(&Serial2),
    bms2(&Serial1),
    vescSerial(VESC_RX, VESC_TX),
    vesc(),
    lastSensorUpdate(0),
    bmsInitialized(false),
    vescInitialized(false) {
    
    // Initialize status to safe defaults
    memset(&bikeStatus, 0, sizeof(BikeStatus));
    bikeStatus.operationState = BIKE_OFF;
}

BikeSensorManager::~BikeSensorManager() {
}

void BikeSensorManager::begin() {
    Serial.println("=== Sensor Manager Initialization ===");
    
    // Initialize GPIO pins - using pins from BikeHardware.h
    // KEY_PIN is managed by RFID manager, not set here
    pinMode(BRAKE_PIN, INPUT_PULLUP);
    pinMode(BRAKEL_PIN, OUTPUT);
    pinMode(LEFT_PIN, INPUT_PULLUP);   // INPUT: Read left signal state
    pinMode(RIGHT_PIN, INPUT_PULLUP);  // INPUT: Read right signal state
    
    // Initialize BMS communication
    Serial2.begin(115200, SERIAL_8N1, BMS_RX1, BMS_TX1);  // BMS1: RX=5, TX=17
    Serial1.begin(115200, SERIAL_8N1, BMS_RX2, BMS_TX2);  // BMS2: RX=4, TX=16
    
    // Initialize BMS and VESC
    bmsInitialized = initializeBMS();
    vescInitialized = initializeVESC();
    
    Serial.printf("Sensor Manager initialized:\n");
    Serial.printf("- BMS: %s\n", bmsInitialized ? "OK" : "FAILED");
    Serial.printf("- VESC: %s\n", vescInitialized ? "OK" : "FAILED");
}

void BikeSensorManager::update() {
    unsigned long currentTime = millis();

    if (currentTime - lastSensorUpdate >= 300) {
        updateBMSData();
        updateVESCData();
        updateGPIOSensors();
        
        lastSensorUpdate = currentTime;
    }
}

bool BikeSensorManager::initializeBMS() {
    bms1.begin(115200);
    bms2.begin(115200);
    Serial.println("JK-BMS1 Interface initialized successfully");
    Serial.println("JK-BMS2 Interface initialized successfully");
    return true;
}

bool BikeSensorManager::initializeVESC() {
    // Setup SoftwareSerial for VESC on pins 33(RX) and 27(TX)
    vescSerial.begin(9600);
    
    // Set VESC to use SoftwareSerial
    vesc.setSerialPort(&vescSerial);
    
    Serial.println("VESC initialized on SoftwareSerial pins RX=33, TX=27");
    Serial.println("Serial0 (USB) remains free for debug messages");
    
    // Test communication (don't fail if first attempt fails, VESC might need time)
    delay(100); // Give VESC time to initialize
    if (vesc.getVescValues()) {
        Serial.println("VESC communication test successful");
        return true;
    } else {
        Serial.println("VESC communication test failed, but initialized");
        return true;  // Still consider it initialized, communication might work later
    }
}

void BikeSensorManager::updateBMSData() {
    if (!bmsInitialized) return;
    
    // Update BMS1 data using JKBMSInterface
    bms1.update();
    
    // Check if BMS1 data is valid and update bike status
    if (bms1.isDataValid()) {
        // Basic measurements
        bikeStatus.bms1.voltage = bms1.getVoltage();
        bikeStatus.bms1.current = bms1.getCurrent();
        bikeStatus.bms1.soc = bms1.getSOC();
        bikeStatus.bms1.temperature = bms1.getBatteryTemp();
        bikeStatus.bms1.connected = true;
        
        // Extended data
        bikeStatus.bms1.cycles = bms1.getCycles();
        bikeStatus.bms1.powerTemp = bms1.getPowerTemp();
        bikeStatus.bms1.boxTemp = bms1.getBoxTemp();
        
        // Debug: Print BMS1 temperatures (disabled for cleaner output)
        // Serial.printf("🌡️ [BMS1] Battery: %.1f°C, Power: %.1f°C, Box: %.1f°C\n", 
        //              bikeStatus.bms1.temperature, bikeStatus.bms1.powerTemp, bikeStatus.bms1.boxTemp);
        
        // Cell voltage info
        bikeStatus.bms1.numCells = bms1.getNumCells();
        bikeStatus.bms1.lowestCellVolt = bms1.getLowestCellVoltage();
        bikeStatus.bms1.highestCellVolt = bms1.getHighestCellVoltage();
        bikeStatus.bms1.cellVoltageDelta = (uint16_t)(bms1.getCellVoltageDelta() * 1000); // Convert V to mV
        
        // Status flags
        bikeStatus.bms1.alarmStatus = bms1.getAlarmStatus();
        bikeStatus.bms1.statusInfo = bms1.getStatusInfo();
        bikeStatus.bms1.isCharging = bms1.isCharging();
        bikeStatus.bms1.isDischarging = bms1.isDischarging();
        bikeStatus.bms1.chargingEnabled = bms1.isChargingEnabled();
        bikeStatus.bms1.dischargingEnabled = bms1.isDischargingEnabled();
        
        // Device info
        bikeStatus.bms1.softwareVersion = bms1.getSoftwareVersion();
        bikeStatus.bms1.deviceInfo = bms1.getDeviceInfo();
    } else {
        bikeStatus.bms1.connected = false;
    }
    
    // Update BMS2 data using JKBMSInterface
    bms2.update();
    
    // Check if BMS2 data is valid and update bike status
    if (bms2.isDataValid()) {
        // Basic measurements
        bikeStatus.bms2.voltage = bms2.getVoltage();
        bikeStatus.bms2.current = bms2.getCurrent();
        bikeStatus.bms2.soc = bms2.getSOC();
        bikeStatus.bms2.temperature = bms2.getBatteryTemp();
        bikeStatus.bms2.connected = true;
        
        // Extended data
        bikeStatus.bms2.cycles = bms2.getCycles();
        bikeStatus.bms2.powerTemp = bms2.getPowerTemp();
        bikeStatus.bms2.boxTemp = bms2.getBoxTemp();
        
        // Debug: Print BMS2 temperatures (disabled for cleaner output)
        // Serial.printf("🌡️ [BMS2] Battery: %.1f°C, Power: %.1f°C, Box: %.1f°C\n", 
        //              bikeStatus.bms2.temperature, bikeStatus.bms2.powerTemp, bikeStatus.bms2.boxTemp);
        
        // Cell voltage info
        bikeStatus.bms2.numCells = bms2.getNumCells();
        bikeStatus.bms2.lowestCellVolt = bms2.getLowestCellVoltage();
        bikeStatus.bms2.highestCellVolt = bms2.getHighestCellVoltage();
        bikeStatus.bms2.cellVoltageDelta = (uint16_t)(bms2.getCellVoltageDelta() * 1000); // Convert V to mV
        
        // Status flags
        bikeStatus.bms2.alarmStatus = bms2.getAlarmStatus();
        bikeStatus.bms2.statusInfo = bms2.getStatusInfo();
        bikeStatus.bms2.isCharging = bms2.isCharging();
        bikeStatus.bms2.isDischarging = bms2.isDischarging();
        bikeStatus.bms2.chargingEnabled = bms2.isChargingEnabled();
        bikeStatus.bms2.dischargingEnabled = bms2.isDischargingEnabled();
        
        // Device info
        bikeStatus.bms2.softwareVersion = bms2.getSoftwareVersion();
        bikeStatus.bms2.deviceInfo = bms2.getDeviceInfo();
    } else {
        bikeStatus.bms2.connected = false;
    }
}

void BikeSensorManager::updateVESCData() {
    if (!vescInitialized) return;
    
    if (vesc.getVescValues()) {
        bikeStatus.vesc.motorERPM = vesc.data.erpm;
        bikeStatus.vesc.motorCurrent = vesc.data.avgMotorCurrent;
        bikeStatus.vesc.inputVoltage = vesc.data.inpVoltage;
        bikeStatus.vesc.dutyCycle = vesc.data.dutyCycleNow;
        bikeStatus.vesc.tempFET = vesc.data.tempMosfet;
        bikeStatus.vesc.tempMotor = vesc.data.tempMotor;
        bikeStatus.vesc.connected = true;
        
        // Calculate bike speed from eRPM (electrical RPM)
        // eRPM = actual motor RPM * total poles (48 poles for 24 pole pairs)
        // Formula: eRPM * WHEEL_CIRCUMFERENCE * MOTOR_WHEEL_REDUCTION_RATIO / (60 * MOTOR_TOTAL_POLES) * 3.6
        // Simplified: eRPM * ERPM_TO_SPEED_FACTOR
        bikeStatus.bikeSpeed = bikeStatus.vesc.motorERPM * ERPM_TO_SPEED_FACTOR;
        
        // Serial.printf("🚀 [VESC] eRPM: %.0f (Speed: %.1f km/h), Motor Current: %.1fA, Input Voltage: %.1fV, Temp FET: %.1f°C, Temp Motor: %.1f°C\n", 
        //               bikeStatus.vesc.motorERPM, bikeStatus.bikeSpeed, bikeStatus.vesc.motorCurrent, bikeStatus.vesc.inputVoltage, bikeStatus.vesc.tempFET, bikeStatus.vesc.tempMotor);
    } else {
        bikeStatus.vesc.connected = false;
    }
}

void BikeSensorManager::updateGPIOSensors() {
    // KEY_PIN is OUTPUT controlled by RFID manager, not read here
    // keyOn status is determined by RFID unlock state in main.cpp
    bikeStatus.brakePressed = !digitalRead(BRAKE_PIN);
    bikeStatus.leftSignal = digitalRead(LEFT_PIN);
    bikeStatus.rightSignal = digitalRead(RIGHT_PIN);
}

BikeStatus BikeSensorManager::getBikeStatus() const {
    return bikeStatus;
}

void BikeSensorManager::setBikeKeyState(bool keyOn) {
    bikeStatus.keyOn = keyOn;
}

void BikeSensorManager::setMotorCurrent(float current) {
    if (!vescInitialized) return;
    
    // Limit to safety bounds
    current = constrain(current, -30.0, 30.0);
    
    vesc.setCurrent(current);
    Serial.printf("Motor current set to %.1fA\n", current);
}

void BikeSensorManager::setMotorRPM(int rpm) {
    if (!vescInitialized) return;
    
    // Limit to safety bounds  
    rpm = constrain(rpm, -3000, 3000);
    
    vesc.setRPM(rpm);
    Serial.printf("Motor RPM set to %d\n", rpm);
}

float BikeSensorManager::calculateSpeedFromMotorRPM(float motorRPM) const {
    if (motorRPM == 0.0f) {
        return 0.0f; // Motor stopped
    }
    
    // Convert eRPM (electrical RPM) to bike speed in km/h
    // eRPM = actual motor RPM * total poles (48 poles for 24 pole pairs)
    // Formula derivation:
    // 1. eRPM → Wheel rotations per second: eRPM / (60 * MOTOR_TOTAL_POLES)
    // 2. Wheel rotation per second → Distance per second: rotation/sec * WHEEL_CIRCUMFERENCE
    // 3. Distance per second → km/h: * 3.6
    // 4. Apply reduction ratio: * MOTOR_WHEEL_REDUCTION_RATIO
    // 
    // Combined: motorRPM (as eRPM) * ERPM_TO_SPEED_FACTOR
    return motorRPM * ERPM_TO_SPEED_FACTOR;
}
