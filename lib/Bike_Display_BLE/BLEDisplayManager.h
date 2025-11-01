#ifndef BLE_DISPLAY_MANAGER_H
#define BLE_DISPLAY_MANAGER_H

#include <NimBLEDevice.h>
#include <Arduino.h>
#include <Preferences.h>
#include "BLEDisplayInfo.h"

// Service UUIDs for Display System
#define DISPLAY_ADV_UUID  "610c3edd-0a09-439a-9274-33702080fa5c"

// Display States
enum DisplayState {
    DISPLAY_STATE_IDLE = 0,
    DISPLAY_STATE_CONNECTED = 1,
    DISPLAY_STATE_ACTIVE = 2,
    DISPLAY_STATE_ERROR = 3
};

class BLEDisplayManager : public BLEServerCallbacks {
public:
    BLEDisplayManager();
    ~BLEDisplayManager();

    // Initialization
    void begin();
    void update();

    // BLE Server Callbacks
    void onConnect(BLEServer* pServer, ble_gap_conn_desc* param) override;
    void onDisconnect(BLEServer* pServer, ble_gap_conn_desc* param) override;

    // Connection Management
    bool isConnected();
    void startAdvertising();
    void stopAdvertising();

    // Display Control
    void setDisplayState(DisplayState state);
    DisplayState getDisplayState();

    // Display Information
    const char* getDisplayName();
    void setDisplayName(const char* name);
    const char* getDisplayVersion();
    const char* getDisplayHardware();

    // Notifications
    void notifyDisplayStatusChange(const char* status);

private:
    // Core components
    BLEServer* pServer;
    BLEAdvertising* pAdvertising;

    // Custom components
    BLEDisplayInfo* displayInfoService;

    // State management
    bool connected;
    DisplayState currentState;
    ble_gap_conn_desc* connectionParam;

    // Internal functions
    void setupServer();
    void setupServices();
    void setupAdvertising();
    void setPower();
    void setConnectionPriority(bool isHigh);
};

#endif