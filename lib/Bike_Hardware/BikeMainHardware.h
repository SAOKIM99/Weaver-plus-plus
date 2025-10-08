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

// CAN Bus pins (Main Board - VP230 transceiver)
#define MAIN_CAN_TX                     25      // GPIO25 -> VP230 TXD
#define MAIN_CAN_RX                     26      // GPIO26 -> VP230 RXD

// Speed Calculation Constants
#define WHEEL_DIAMETER_M                0.7        // 700mm wheel diameter (meters)
#define WHEEL_CIRCUMFERENCE_M           (PI * WHEEL_DIAMETER_M)  // ~2.2m
#define MAGNETS_PER_WHEEL_REVOLUTION    1          // 1 magnet per wheel revolution
#define MS_TO_KMH_FACTOR               3.6        // m/s to km/h conversion

#endif
