#ifndef BIKE_HARDWARE_H
#define BIKE_HARDWARE_H

#include <Arduino.h>

// GPIO Pin Definitions
#define KEY_PIN                         13  // OUTPUT: High = bike ON
#define BRAKE_PIN                       14  // INPUT: High = brake pressed
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

// CAN Bus pins (Main Board - VP230 transceiver)
#define MAIN_CAN_TX                     25      // GPIO25 -> VP230 TXD
#define MAIN_CAN_RX                     26      // GPIO26 -> VP230 RXD

// Speed Calculation Constants
// Tire: 120/80-16 (width-ratio-diameter in inches)
// Tire height = 80% × 120mm = 96mm
// Wheel diameter = 16 inches + 2 × tire_height = 406.4mm + 192mm = 598.4mm = 0.5984m
#define WHEEL_DIAMETER_M                0.5984     // Tire 120/80-16 wheel diameter (meters)
#define WHEEL_CIRCUMFERENCE_M           (PI * WHEEL_DIAMETER_M)  // ~1.88m

// Motor Speed to Wheel Speed (VESC eRPM to bike speed)
// Motor: 24 pole pairs (48 poles total)
// eRPM = Electrical RPM = actual motor RPM * number_of_poles
// Motor drive wheel 1:1 directly (or set reduction ratio if using gearbox)
#define MOTOR_POLE_PAIRS               24         // 24 pole pairs
#define MOTOR_TOTAL_POLES              (MOTOR_POLE_PAIRS * 2)  // 48 poles
#define MOTOR_WHEEL_REDUCTION_RATIO    1.0f       // 1:1 direct drive (adjust if using gearbox)

// Derived constants for eRPM to speed calculation
// eRPM to wheel rotation frequency: eRPM / (60 * total_poles) = wheel_rotation_per_second
// Wheel speed = rotation_per_sec * circumference * reduction_ratio * 3.6 (m/s to km/h)
// Formula: speed_kmh = eRPM * WHEEL_CIRCUMFERENCE_M * MOTOR_WHEEL_REDUCTION_RATIO * 3.6 / (60 * total_poles)
#define ERPM_TO_SPEED_FACTOR           ((WHEEL_CIRCUMFERENCE_M * MOTOR_WHEEL_REDUCTION_RATIO * 3.6f) / (60.0f * MOTOR_POLE_PAIRS))

// Hall sensor constants (for reference, not used if using VESC eRPM)
#define MAGNETS_PER_WHEEL_REVOLUTION    1          // 1 magnet per wheel revolution
#define MS_TO_KMH_FACTOR               3.6        // m/s to km/h conversion

#endif
