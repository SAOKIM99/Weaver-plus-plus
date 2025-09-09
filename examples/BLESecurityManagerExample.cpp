#include <Arduino.h>
#include "BLESecurityManager.h"

// Create BLE Security Manager instance
BLESecurityManager bleManager("ESP32-SecureDevice", 
                             "12345678-1234-1234-1234-123456789abc",
                             "87654321-4321-4321-4321-cba987654321");

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32 BLE Security Manager Example ===");
    
    // Initialize BLE Security Manager
    bleManager.begin();
    
    // Optional: Customize settings
    // bleManager.setBootButtonPin(0);           // Change boot button pin
    // bleManager.setPairingTimeout(60000);      // 60 seconds timeout
    // bleManager.setMaxBondedDevices(10);       // Allow 10 bonded devices
}

void loop() {
    // Update BLE Security Manager (handles button presses and timeouts)
    bleManager.update();
    
    // Status update every 10 seconds
    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate > 10000) {
        Serial.print("Status: ");
        if (bleManager.isConnected()) {
            Serial.print("Connected");
        } else if (bleManager.isPairingInProgress()) {
            Serial.print("PAIRING IN PROGRESS - Press BOOT button!");
        } else {
            Serial.print("Advertising - Ready for connections");
        }
        
        Serial.print(" | Bonded devices: ");
        Serial.println(bleManager.getBondedDeviceCount());
        
        lastStatusUpdate = millis();
    }
    
    // Your application logic here
    if (bleManager.isConnected()) {
        // Handle BLE communication
        // You can get the characteristic: bleManager.getCharacteristic()
        // Example: Send data, handle received data, etc.
    }
    
    delay(100);
}

// Example function to demonstrate BLE data handling
void handleBLEData() {
    if (bleManager.isConnected()) {
        // Get the characteristic for communication
        BLECharacteristic* pChar = bleManager.getCharacteristic();
        
        // Example: Read data
        std::string receivedData = pChar->getValue();
        if (receivedData.length() > 0) {
            Serial.print("Received: ");
            Serial.println(receivedData.c_str());
        }
        
        // Example: Send data
        // pChar->setValue("Hello from ESP32");
        // pChar->notify();
    }
}
