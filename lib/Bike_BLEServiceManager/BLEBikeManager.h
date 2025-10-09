#ifndef BLE_BIKE_MANAGER_H
#define BLE_BIKE_MANAGER_H

#include <NimBLEDevice.h>
#include <Arduino.h>
#include <Preferences.h>
#include "BLEBikeInfo.h"
#include "BikeData.h"
#include "BikeMainHardware.h"

// Forward declaration
class BikeRFIDManager;

// Security definitions
#define PAIRING_TIMEOUT_MS 30000  // 30 seconds timeout for pairing
#define MAX_BONDED_DEVICES 5      // Maximum number of bonded devices

// Service UUIDs for Bike System
#define BIKE_INFO_SERVICE_UUID      "12345678-1234-1234-1234-123456789abc"
#define BIKE_CONTROL_SERVICE_UUID   "87654321-4321-4321-4321-cba987654321"

// Bike States
enum BikeState {
    BIKE_STATE_IDLE = 0,
    BIKE_STATE_CONNECTED = 1,
    BIKE_STATE_SECURED = 2,
    BIKE_STATE_RUNNING = 3,
    BIKE_STATE_ERROR = 4
};

class BLEBikeManager : public BLEServerCallbacks {
public:
    BLEBikeManager();
    ~BLEBikeManager();
    
    // Initialization
    void begin();
    void setRFIDManager(BikeRFIDManager* rfid);  // Set RFID manager reference
    void update();
    
    // BLE Server Callbacks
    void onConnect(BLEServer* pServer, ble_gap_conn_desc* param) override;
    void onDisconnect(BLEServer* pServer, ble_gap_conn_desc* param) override;
    void onAuthenticationComplete(ble_gap_conn_desc* desc) override;
    uint32_t onPassKeyRequest() override;
    bool onConfirmPIN(uint32_t pin) override;
    
    // Connection Management
    bool isConnected();
    bool isSecured();
    void startAdvertising();
    void stopAdvertising();
    
    // Bike Control
    void setBikeState(BikeState state);
    BikeState getBikeState();
    
    // Security
    bool isPairingInProgress();
    void clearBondedDevices();
    uint8_t getBondedDeviceCount();
    void printBondedDevices();  // Debug: Show all bonded devices
    
    // Bike Information
    const char* getBikeName();
    void setBikeName(const char* name);
    BikeOperationState getBikeStatus();
    void setBikeStatus(BikeOperationState status);
    
    // Notifications
    void notifyBikeStatusChange(BikeOperationState status);
    void notifyStateChange(BikeState state);

private:
    // Core components
    BLEServer* pServer;
    BLEAdvertising* pAdvertising;
    
    // Custom components
    BLEBikeInfo* bikeInfoService;
    
    // State management
    bool connected;
    bool secured;
    BikeState currentState;
    ble_gap_conn_desc* connectionParam;
    
    // Security management (simplified - no need to store MAC addresses)
    bool pairingInProgress;
    unsigned long pairingStartTime;
    
    // RFID integration
    BikeRFIDManager* rfidManager;
    
    // Internal functions
    void setupServer();
    void setupServices();
    void setupAdvertising();
    void setupSecurity();
    void setPower();
    void setConnectionPriority(bool isHigh);
    
    // Security functions
    bool isBootButtonPressed();
    bool isRFIDAuthenticated();  // Check RFID authentication
    
    // Bonding information retrieval (from NimBLE stack)
    int getBondedDevicesFromStack();
};

#endif
