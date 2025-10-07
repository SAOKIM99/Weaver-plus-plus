#ifndef BIKE_HARDWARE_H
#define BIKE_HARDWARE_H

#include <Arduino.h>

// GPIO Pin Definitions
#define KEY_PIN                         13  // OUTPUT: High = bike ON
#define BRAKE_PIN                       14  // INPUT: Low = brake pressed  
#define BRAKEL_PIN                      12  // OUTPUT: High = brake light ON
#define HALL_PIN                        34  // INPUT: Motor speed pulse
#define LEFT_PIN                        35  // INPUT: High = left signal detected
#define RIGHT_PIN                       32  // INPUT: High = right signal detected

// Boot button for manual BLE authentication
#define MANUAL_AUTHENTICATION_PIN       0

// RFID Reader pins
#define RST_PIN                         22     
#define SS_PIN                          21

// BMS UART pins (JK-BMS Interface)
#define BMS_RX1                         5
#define BMS_TX1                         17
#define BMS_RX2                         4
#define BMS_TX2                         16

// VESC UART pins
#define VESC_RX                         33
#define VESC_TX                         27

// Speed Calculation Constants
#define WHEEL_DIAMETER_M                0.7        // 700mm wheel diameter (meters)
#define WHEEL_CIRCUMFERENCE_M           (PI * WHEEL_DIAMETER_M)  // ~2.2m
#define MAGNETS_PER_WHEEL_REVOLUTION    1          // 1 magnet per wheel revolution
#define MS_TO_KMH_FACTOR               3.6        // m/s to km/h conversion

// Bike States
enum BikeOperationState {
    BIKE_OFF = 0,
    BIKE_ON = 1,
    BIKE_LOCKED = 2,
    BIKE_UNLOCKED = 3
};

// Sensor Data Structures
struct BMSData {
    float voltage;
    float current;
    float temperature;
    uint8_t soc; // State of charge %
    bool connected;
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

#endif
