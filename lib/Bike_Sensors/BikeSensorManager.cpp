#include "BikeSensorManager.h"

// Static variables for Hall sensor interrupt
volatile unsigned long BikeSensorManager::hallPulseCount = 0;
volatile unsigned long BikeSensorManager::lastHallTime = 0;
volatile float BikeSensorManager::hallFrequency = 0.0;
volatile bool BikeSensorManager::hallFrequencyReady = false;

// Hall sensor interrupt service routine
void IRAM_ATTR BikeSensorManager::hallSensorISR() {
    unsigned long currentTime = micros();
    
    if (lastHallTime > 0) {
        unsigned long pulsePeriod = currentTime - lastHallTime;
        
        // Calculate frequency only if period is reasonable (avoid noise)
        if (pulsePeriod > 1000 && pulsePeriod < 1000000) { // Between 1ms and 1s
            hallFrequency = 1000000.0 / pulsePeriod; // Convert to Hz
            hallFrequencyReady = true;
        }
    }
    
    lastHallTime = currentTime;
    hallPulseCount++;
}

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
    pinMode(HALL_PIN, INPUT_PULLUP);
    
    // Setup Hall sensor interrupt
    attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallSensorISR, RISING);
    Serial.println("Hall sensor interrupt attached (RISING edge)");
    
    // Initialize BMS communication
    Serial2.begin(115200, SERIAL_8N1, BMS_RX1, BMS_TX1);  // BMS1: RX=5, TX=17
    Serial1.begin(115200, SERIAL_8N1, BMS_RX2, BMS_TX2);  // BMS2: RX=4, TX=16
    
    // Initialize BMS and VESC
    bmsInitialized = initializeBMS();
    vescInitialized = initializeVESC();
    
    Serial.printf("Sensor Manager initialized:\n");
    Serial.printf("- BMS: %s\n", bmsInitialized ? "OK" : "FAILED");
    Serial.printf("- VESC: %s\n", vescInitialized ? "OK" : "FAILED");
    Serial.printf("- Hall Interrupt: %s\n", "ENABLED");
}

void BikeSensorManager::update() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastSensorUpdate >= 1000) { // Update every 1 second
        updateBMSData();
        updateVESCData();
        updateGPIOSensors();
        updateHallSensors();
        
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
    vescSerial.begin(115200);
    
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
        bikeStatus.vesc.motorRPM = vesc.data.rpm;
        bikeStatus.vesc.motorCurrent = vesc.data.avgMotorCurrent;
        bikeStatus.vesc.inputVoltage = vesc.data.inpVoltage;
        bikeStatus.vesc.dutyCycle = vesc.data.dutyCycleNow;
        bikeStatus.vesc.tempFET = vesc.data.tempMosfet;
        bikeStatus.vesc.tempMotor = vesc.data.tempMotor;
        bikeStatus.vesc.connected = true;
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

void BikeSensorManager::updateHallSensors() {
    // Get Hall sensor frequency from interrupt
    noInterrupts(); // Disable interrupts briefly for thread safety
    
    if (hallFrequencyReady) {
        bikeStatus.hallFrequency = hallFrequency;
        hallFrequencyReady = false; // Mark as read
        
        // Calculate bike speed from Hall frequency
        bikeStatus.bikeSpeed = calculateBikeSpeed(bikeStatus.hallFrequency);
        
    } else {
        // Check if Hall sensor has been inactive (no pulses for >2 seconds)
        unsigned long currentTime = micros();
        if (lastHallTime > 0 && (currentTime - lastHallTime) > 2000000) {
            bikeStatus.hallFrequency = 0.0; // Bike stopped
            bikeStatus.bikeSpeed = 0.0;     // Speed = 0
        }
    }
    
    interrupts(); // Re-enable interrupts
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

unsigned long BikeSensorManager::getHallPulseCount() const {
    noInterrupts();
    unsigned long count = hallPulseCount;
    interrupts();
    return count;
}

float BikeSensorManager::calculateBikeSpeed(float hallFreq) const {
    if (hallFreq <= 0.0) {
        return 0.0; // Bike stopped
    }
    
    // Hall frequency = Wheel rotation frequency (assuming 1 magnet per wheel)
    float wheelRotationsPerSecond = hallFreq / MAGNETS_PER_WHEEL_REVOLUTION;
    
    // Distance per second = rotations/s × wheel circumference
    float metersPerSecond = wheelRotationsPerSecond * WHEEL_CIRCUMFERENCE_M;
    
    // Convert m/s to km/h
    float kmPerHour = metersPerSecond * MS_TO_KMH_FACTOR;
    
    return kmPerHour;
}
