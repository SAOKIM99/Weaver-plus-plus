#ifndef BLE_BIKE_MANAGER_H
#define BLE_BIKE_MANAGER_H

#include <NimBLEDevice.h>
#include <Arduino.h>
#include <Preferences.h>
#include "BLEBikeInfo.h"
#include "BikeData.h"

// Boot button and security definitions
#define BOOT_BUTTON_PIN 0
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
    void update();
    
    // BLE Server Callbacks (kế thừa từ BLESecurityManager)
    void onConnect(BLEServer* pServer, ble_gap_conn_desc* param) override;
    void onDisconnect(BLEServer* pServer, ble_gap_conn_desc* param) override;
    
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
    // BLEService* pInfoService;
    BLEAdvertising* pAdvertising;
    
    // Custom components
    BLEBikeInfo* bikeInfoService;
    
    // State management
    bool connected;
    bool secured;
    BikeState currentState;
    ble_gap_conn_desc* connectionParam;
    
    // Security management
    Preferences preferences;
    bool pairingInProgress;
    bool bootButtonPressed;
    unsigned long lastBootButtonCheck;
    unsigned long pairingStartTime;
    
    // Internal functions
    void setupServer();
    void setupServices();
    void setupAdvertising();
    void setPower();
    void setConnectionPriority(bool isHigh);
    
    // Security functions
    bool isBootButtonPressed();
    void updateBootButton();
    void saveBondedDevice(BLEAddress address);
    bool isDeviceBonded(BLEAddress address);
    void clearBondedDevicesInternal();
    
    // Security integration
    void handleSecureConnection(BLEServer* pServer, ble_gap_conn_desc* param);
    void handleDisconnection(BLEServer* pServer, ble_gap_conn_desc* param);
};

#endif
