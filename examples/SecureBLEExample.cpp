#include <Arduino.h>
#include "SecureBLEManager.h"

// Create BLE Manager instance
SecureBLEManager bleManager("MyESP32Device");

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== ESP32 Secure BLE Example ===");
  Serial.println("This example demonstrates secure BLE pairing with ESP32");
  Serial.println();
  
  // Initialize BLE Manager
  bleManager.begin();
  
  Serial.println("=== Usage Instructions ===");
  Serial.println("1. Press and hold BOOT button to enter pairing mode");
  Serial.println("2. Pairing mode lasts 30 seconds");
  Serial.println("3. Use BLE scanner app to find and pair with device");
  Serial.println("4. Once paired, only this device can connect in future");
  Serial.println("5. To pair new device, press BOOT button again");
  Serial.println();
  Serial.println("=== Status ===");
}

void loop() {
  static unsigned long lastStatusUpdate = 0;
  static bool lastPairingState = false;
  static bool lastConnectionState = false;
  
  // Update BLE Manager
  bleManager.update();
  
  // Print status updates every 5 seconds or when state changes
  unsigned long currentTime = millis();
  bool currentPairingState = bleManager.isPairingMode();
  bool currentConnectionState = bleManager.isConnected();
  
  if (currentTime - lastStatusUpdate > 5000 || 
      currentPairingState != lastPairingState ||
      currentConnectionState != lastConnectionState) {
    
    Serial.print("Time: ");
    Serial.print(currentTime / 1000);
    Serial.print("s | Pairing Mode: ");
    Serial.print(currentPairingState ? "ON" : "OFF");
    Serial.print(" | Connected: ");
    Serial.print(currentConnectionState ? "YES" : "NO");
    
    if (currentPairingState) {
      // Calculate remaining pairing time
      unsigned long remainingTime = (30000 - (currentTime % 30000)) / 1000;
      Serial.print(" | Remaining: ");
      Serial.print(remainingTime);
      Serial.print("s");
    }
    
    Serial.println();
    
    lastStatusUpdate = currentTime;
    lastPairingState = currentPairingState;
    lastConnectionState = currentConnectionState;
  }
  
  // Your application logic here
  // For example, you could:
  // - Send sensor data via BLE when connected
  // - Handle BLE commands from mobile app
  // - Control device based on BLE input
  
  delay(100);
}

// Example function to demonstrate BLE data handling
void handleBLEData() {
  if (bleManager.isConnected()) {
    // Handle incoming BLE data
    // Send outgoing BLE data
    Serial.println("Handling BLE communication...");
  }
}
