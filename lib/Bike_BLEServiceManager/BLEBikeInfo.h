#ifndef BLE_BIKE_INFO_H
#define BLE_BIKE_INFO_H

#include "BLEServiceManager.h"
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <BikeData.h>

// Service và Characteristic UUIDs cho Bike Info
#define BIKE_INFO_SERVICE_UUID      "12345678-1234-1234-1234-123456789abc"
#define BIKE_NAME_CHAR_UUID         "f2c513b7-6b51-4363-b6aa-1ef8bd08c56a"
#define BIKE_HARDWARE_CHAR_UUID     "dc3c0fd6-c7e1-4f81-8e06-dbdbef8058bb"
#define BIKE_STATUS_CHAR_UUID       "8b8f9b38-9af4-11ee-b9d1-0242ac120002"

// Bike specific definitions
#define BIKE_VERSION                "1.0"
#define INIT_BIKE_NAME             "+ SAO KIM +"
#define INIT_BIKE_HARDWARE         "W++"

#define SECURITY_LOCK              "140499"
#define SECURITY_LOCK_LEN          6

#define MAX_VERSION_LENGTH         10
#define MAX_NAME_LENGTH           30
#define MAX_HARDWARE_LENGTH       30


class BLEBikeInfo : public BLEServiceManager<3> {
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

    // Characteristic callbacks
    void onReadName(BLECharacteristic* pChar);
    void onWriteName(BLECharacteristic* pChar);
    
    void onReadHardware(BLECharacteristic* pChar);
    void onWriteHardware(BLECharacteristic* pChar);
    
    void onReadStatus(BLECharacteristic* pChar);
    
    // Utility functions
    void getStringPref(Preferences& p, const char* key, char* data, size_t maxLen, const char* defaultVal);
    bool validateSecurityKey(const char* input);
};

#endif
