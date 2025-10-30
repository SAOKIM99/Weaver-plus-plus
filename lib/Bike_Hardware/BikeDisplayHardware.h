#ifndef DISPLAY_HARDWARE_H
#define DISPLAY_HARDWARE_H

// ================================================
// HARDWARE PIN CONFIGURATION
// ================================================

// CAN Bus Pins
#define CAN_TX_PIN    25
#define CAN_RX_PIN    26

// COS Pin (Cosine/Encoder Input)
#define COS_PIN       34 // Active HIGH input for passing signal

// ================================================
// DISPLAY PINS (TFT_eSPI Configuration)
// ================================================
// Note: Display pins are configured in User_Setup.h
// This section is for reference only

// ================================================
// OTHER HARDWARE PINS
// ================================================
// Add other hardware pin definitions here as needed
// Examples:
// #define LED_PIN       2
// #define BUTTON_PIN    0
// #define BUZZER_PIN    4

#endif // DISPLAY_HARDWARE_H