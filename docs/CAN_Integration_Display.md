# CAN Integration for Display Controller

## Overview
The TFT Display Controller (`main_display.cpp`) has been upgraded to receive real-time bike data via CAN bus from the main controller instead of using simulation data.

## Key Changes Made

### 1. **PlatformIO Configuration**
- Added `Bike_CAN` and `Bike_Data` libraries to `tft_display` environment
- CAN library already included for hardware support

### 2. **Hardware Configuration**
- **Display Board CAN Pins**: TX=25, RX=26 (from BikeDisplayHardware.h)
- **Main Board CAN Pins**: TX=25, RX=26 (from BikeMainHardware.h)
- **CAN Speed**: 500 kbps
- **Transceiver**: VP230 (both boards)

### 3. **Software Architecture**

#### **CAN Manager Integration**
```cpp
BikeCANManager canManager;
canManager.begin(25, 26); // Display board pins
canManager.setReceiveCallback(onCANMessage);
```

#### **Message Parsing**
- **Automatic Parsing**: `parseCANMessage()` updates `BikeDataDisplay` directly
- **Real-time Updates**: Callback-driven data updates
- **Connection Monitoring**: Fallback to demo mode if CAN connection lost

#### **Supported Messages**
```cpp
MSG_ID_BIKE_STATUS (0x100)     → Speed, Bluetooth, Turn Signals
MSG_ID_BMS_DATA + 1 (0x201)   → Battery 1 Data
MSG_ID_BMS_DATA + 2 (0x202)   → Battery 2 Data  
MSG_ID_VESC_DATA (0x300)       → Motor Controller Data
MSG_ID_BATTERY_EXT (0x400)     → Extended Battery Info
MSG_ID_DISTANCE_DATA (0x500)   → Distance & Odometer
MSG_ID_TIME_DATA (0x600)       → Operating Time
```

## Features

### ✅ **Real-time Data Reception**
- Live bike data from main controller
- Automatic parsing into dashboard format
- Seamless UI updates

### ✅ **Connection Management** 
- CAN connection status monitoring
- Fallback to demo mode when disconnected
- Connection status logging

### ✅ **Debug & Monitoring**
- CAN message logging with parsed values
- Statistics: messages received count
- Connection status indicators

### ✅ **Backwards Compatibility**
- Falls back to demo data if no CAN connection
- Maintains same UI behavior
- No breaking changes to existing display code

## Usage

### **Main Controller (Sender)**
Run with `esp32dev` environment - sends bike data via CAN

### **Display Controller (Receiver)**
Run with `tft_display` environment - receives and displays data

### **Testing Scenarios**

1. **Full System**: Both controllers connected via CAN bus
   - Real-time data display
   - All dashboard elements updated from live data

2. **Display Only**: Display controller without main controller
   - Automatic fallback to demo mode after 5 seconds
   - Simulated data for UI testing

3. **Development**: Individual controller testing
   - Each board can run independently
   - Proper error handling and logging

## Build Commands

```bash
# Build main controller
platformio run --environment esp32dev

# Build display controller  
platformio run --environment tft_display

# Upload to display controller
platformio run --environment tft_display --target upload

# Monitor display controller
platformio device monitor --environment tft_display
```

## Expected Serial Output

### **Display Controller (Receiver)**
```
=== SAO KIM Display Controller ===
✅ CAN Manager initialized successfully
✅ CAN Receive callback registered
🚴‍♂️ LVGL Electric Bike Dashboard with CAN Support initialized!
📡 Waiting for CAN messages from main controller...

[CAN] Status: Speed=25.3 km/h, BT=ON, L=OFF, R=OFF
[CAN] BMS1: 48.20V, 85%, 28.0°C
[CAN] Motor: 4.30A, Motor=45.0°C, ECU=38.0°C
CAN:OK Speed:25.3 km/h Bat:85% Motor:45.0°C BT:ON
```

### **Connection Lost (Demo Mode)**
```
[CAN] Connection lost - switching to demo mode
CAN:DEMO Speed:25.0 km/h Bat:75% Motor:45.0°C BT:OFF
```

## System Integration

The display controller now operates as a true **CAN receiver node**, processing real-time data from the main bike controller. This creates a distributed system where:

- **Main Controller**: Handles sensors, BLE, RFID, motor control, and broadcasts data
- **Display Controller**: Focuses solely on dashboard display and receives data via CAN

This architecture improves system reliability, reduces wiring complexity, and allows independent development of display and control systems.