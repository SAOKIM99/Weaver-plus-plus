#pragma once

#include "Arduino.h"
#include <SoftwareSerial.h>
#include "../JKBMSInterface/JKBMSInterface.h"
#include "../Vesc_Uart/src/VescUart.h"
#include "BikeMainHardware.h"
#include "BikeData.h"

class BikeSensorManager {
private:
    BikeStatus bikeStatus;
    JKBMSInterface bms1;
    JKBMSInterface bms2;
    SoftwareSerial vescSerial;
    VescUart vesc;
    
    // Timing
    unsigned long lastSensorUpdate;
    
    // Sensor state
    bool bmsInitialized;
    bool vescInitialized;
    
    // Hall sensor interrupt variables
    static volatile unsigned long hallPulseCount;
    static volatile unsigned long lastHallTime;
    static volatile float hallFrequency;
    static volatile bool hallFrequencyReady;
    
    // Private methods
    void updateBMSData();
    void updateVESCData();
    void updateGPIOSensors();
    void updateHallSensors();
    
    // Hall sensor interrupt handler
    static void IRAM_ATTR hallSensorISR();

public:
    BikeSensorManager();
    ~BikeSensorManager();
    
    // Core functions
    void begin();
    void update();
    
    // Data access
    BikeStatus getBikeStatus() const;
    
    // Bike control
    void setBikeKeyState(bool keyOn);
    
    // Sensor control
    bool initializeBMS();
    bool initializeVESC();
    void setMotorCurrent(float current);
    void setMotorRPM(int rpm);
    
    // Hall sensor utilities
    unsigned long getHallPulseCount() const;
    float calculateBikeSpeed(float hallFreq) const;
};
