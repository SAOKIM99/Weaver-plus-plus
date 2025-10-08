# Bike CAN Manager Library

This library provides CAN bus communication functionality for the SAO KIM Smart Bike System.

## Features

- **Structured CAN Protocol**: Organized message IDs and data formats
- **Multiple Data Types**: Bike status, BMS data, VESC motor data
- **Automatic Sequencing**: Built-in rotation for different message types
- **Thread-Safe**: Designed for RTOS environments
- **Data Packing**: Efficient 16-bit float and integer packing
- **Callback Support**: Custom receive message handlers

## Message Protocol

### MSG_ID_BIKE_STATUS (0x100)
8 bytes containing basic bike status:
- Byte 0: Operation state
- Byte 1: Unlock status
- Byte 2: BLE connection
- Byte 3: Speed (km/h)
- Byte 4: Status flags (bit 0: key, bit 1: brake, bit 2: charging, bit 3: left signal, bit 4: right signal)
- Bytes 5-7: Reserved

### MSG_ID_BMS_DATA (0x201/0x202)
8 bytes of BMS data:
- Bytes 0-1: Voltage * 100
- Bytes 2-3: Current * 100 (signed)
- Byte 4: SOC percentage
- Byte 5: Temperature + 50
- Byte 6: Status flags
- Byte 7: Cell count

### MSG_ID_VESC_DATA (0x300)
8 bytes of motor controller data:
- Bytes 0-1: Motor RPM / 10
- Bytes 2-3: Input voltage * 100
- Bytes 4-5: Motor current * 100
- Byte 6: Motor temperature + 50
- Byte 7: FET temperature + 50

### MSG_ID_BATTERY_EXT (0x400)
8 bytes of extended battery data:
- Byte 0: BMS1 cell voltage delta * 100 + 128
- Byte 1: BMS2 cell voltage delta * 100 + 128
- Byte 2: BMS1 temperature + 40
- Byte 3: BMS2 temperature + 40
- Bytes 4-5: Total motor power (W)
- Byte 6: Status flags (charging, connections)
- Byte 7: Reserved

### MSG_ID_DISTANCE_DATA (0x500)
8 bytes of distance information:
- Bytes 0-3: Odometer * 100 (km)
- Bytes 4-5: Current distance * 100 (km)
- Bytes 6-7: Trip distance * 100 (km)

### MSG_ID_TIME_DATA (0x600)
8 bytes of time information:
- Bytes 0-3: Total time (seconds)
- Bytes 4-7: Reserved for future expansion

## Usage Examples

### Sender (Main Controller)
```cpp
#include "BikeCANManager.h"

BikeCANManager canManager;

void setup() {
    Serial.begin(115200);
    
    // Initialize CAN
    if (canManager.begin()) {
        Serial.println("CAN initialized successfully");
    }
}

void loop() {
    // Send bike status
    canManager.sendBikeStatus(bikeStatus, unlocked, bleConnected);
    
    // Update (handle incoming messages)
    canManager.update();
    
    delay(500);
}
```

### Receiver (Display Controller)
```cpp
#include "BikeCANManager.h"

BikeCANManager canManager;
BikeDataDisplay displayData;

// CAN receive callback
void onCANMessage(uint32_t id, uint8_t* data, uint8_t length) {
    // Parse and update display data
    if (canManager.parseCANMessage(id, data, length, displayData)) {
        Serial.printf("[CAN] Updated: Speed=%.1f km/h, Battery=%d%%\n", 
                      displayData.speed, displayData.batteryPercent);
        
        // Update your display UI here
        updateDisplayUI(displayData);
    }
}

void setup() {
    Serial.begin(115200);
    
    // Initialize CAN
    if (canManager.begin()) {
        Serial.println("CAN initialized successfully");
        
        // Set callback for incoming messages
        canManager.setReceiveCallback(onCANMessage);
    }
}

void loop() {
    // Process incoming CAN messages
    canManager.update();
    
    delay(10);
}
```

### Individual Message Parsing
```cpp
// Parse specific message types
BMSData bms1;
if (canManager.parseBMSData(data, length, bms1)) {
    Serial.printf("BMS1: %.2fV, %d%%, %.1f°C\n", 
                  bms1.voltage, bms1.soc, bms1.temperature);
}

VESCData vesc;
if (canManager.parseVESCData(data, length, vesc)) {
    Serial.printf("Motor: %.0f RPM, %.2fA, %.1f°C\n", 
                  vesc.motorRPM, vesc.motorCurrent, vesc.tempMotor);
}
```

## Hardware Requirements

- ESP32 with CAN transceiver (VP230 or similar)
- CAN TX: GPIO25
- CAN RX: GPIO26
- CAN bus speed: 500 kbps