#ifndef BLE_BIKE_INFO_H
#define BLE_BIKE_INFO_H

#include "BLEServiceManager.h"
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <BikeData.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

// Service và Characteristic UUIDs cho Bike Info
#define BIKE_INFO_SERVICE_UUID      "3082d992-772b-494f-99fa-4c270159ca00"
#define BIKE_HARDWARE_CHAR_UUID     "3082d992-772b-494f-99fa-4c270159ca01"
#define BIKE_OTA_CHAR_UUID          "3082d992-772b-494f-99fa-4c270159ca02"
#define BIKE_NAME_CHAR_UUID         "3082d992-772b-494f-99fa-4c270159ca03"
#define BIKE_STATUS_CHAR_UUID       "3082d992-772b-494f-99fa-4c270159ca04"

// Bike specific definitions
#define BIKE_VERSION               "0.1"
#define INIT_BIKE_NAME             "+ SAO KIM +"
#define INIT_BIKE_HARDWARE         "M-W++"

#define SECURITY_LOCK              "140499"
#define SECURITY_LOCK_LEN          6

#define MAX_VERSION_LENGTH         10
#define MAX_NAME_LENGTH           30
#define MAX_HARDWARE_LENGTH       30

// OTA definitions
#define UPDATE_START_MSG           "START_OTA"
#define UPDATE_END_MSG             "END_OTA"


class BLEBikeInfo : public BLEServiceManager<4> {
public:
    BLEBikeInfo(BLEService* service);
    
    virtual void begin();
    
    // Getters
    const char* getName();
    const char* getVersion();
    const char* getHardware();
    BikeOperationState getStatus();
    
    // Status management
    void setStatus(BikeOperationState status);
    void notifyStatusChange(BikeOperationState status);

private:
    Preferences pref;
    
    char version[MAX_VERSION_LENGTH + 1];
    char name[MAX_NAME_LENGTH + 1];
    char hardware[MAX_HARDWARE_LENGTH + 1];
    BikeOperationState currentStatus;
    
    // OTA variables
    esp_ota_handle_t otaHandler;
    uint16_t packageCounter;

    // Characteristic callbacks
    void onReadName(BLECharacteristic* pChar);
    void onWriteName(BLECharacteristic* pChar);
    
    void onReadHardware(BLECharacteristic* pChar);
    void onWriteHardware(BLECharacteristic* pChar);
    
    void onReadStatus(BLECharacteristic* pChar);
    
    // OTA callbacks
    void onReadOTA(BLECharacteristic* pChar);
    void onWriteOTA(BLECharacteristic* pChar);
    void onNotifyOTA(uint32_t stateOTA);
    
    // OTA utility functions
    void setOTA();
    void resetOTA();
    
    // Utility functions
    void getStringPref(Preferences& p, const char* key, char* data, size_t maxLen, const char* defaultVal);
    bool validateSecurityKey(const char* input);
};

#endif
