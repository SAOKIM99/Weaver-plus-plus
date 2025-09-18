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
    bmsSync(BMS_RX1, BMS_TX1, BMS_RX2, BMS_TX2),
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
    pinMode(LEFT_PIN, OUTPUT);
    pinMode(RIGHT_PIN, OUTPUT);
    pinMode(HALL_PIN, INPUT_PULLUP);
    
    // Setup Hall sensor interrupt
    attachInterrupt(digitalPinToInterrupt(HALL_PIN), hallSensorISR, RISING);
    Serial.println("Hall sensor interrupt attached (RISING edge)");
    
    // Initialize VESC communication
    Serial2.begin(115200, SERIAL_8N1, VESC_RX, VESC_TX);
    
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
    bmsSync.begin();
    Serial.println("BMS initialized successfully");
    return true;
}

bool BikeSensorManager::initializeVESC() {
    vesc.setSerialPort(&Serial2);
    
    // Test communication
    if (!vesc.getVescValues()) {
        Serial.println("VESC communication failed");
        return false;
    }
    
    Serial.println("VESC initialized successfully");
    return true;
}

void BikeSensorManager::updateBMSData() {
    if (!bmsInitialized) return;
    
    // Sync BMS data using BmsSync
    bmsSync.sync();
    
    // Get pack info for both BMS
    bmsSync.getPack(1); // BMS1
    
    // Update bike status with BMS data
    if (bmsSync.packInfo != nullptr && !bmsSync.packInfo->isEmpty()) {
        bikeStatus.bms1.voltage = bmsSync.packInfo->voltage;
        bikeStatus.bms1.current = bmsSync.packInfo->current;
        bikeStatus.bms1.soc = bmsSync.packInfo->percent;
        bikeStatus.bms1.temperature = bmsSync.packInfo->ntc[0]; // Use first NTC sensor
        bikeStatus.bms1.connected = true;
    } else {
        bikeStatus.bms1.connected = false;
    }
    
    // Get pack info for BMS2
    bmsSync.getPack(2);
    if (bmsSync.packInfo != nullptr && !bmsSync.packInfo->isEmpty()) {
        bikeStatus.bms2.voltage = bmsSync.packInfo->voltage;
        bikeStatus.bms2.current = bmsSync.packInfo->current;
        bikeStatus.bms2.soc = bmsSync.packInfo->percent;
        bikeStatus.bms2.temperature = bmsSync.packInfo->ntc[0]; // Use first NTC sensor
        bikeStatus.bms2.connected = true;
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
