#ifndef BLE_DISPLAY_INFO_H
#define BLE_DISPLAY_INFO_H

#include "BLEServiceManager.h"
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <BikeData.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

// Service và Characteristic UUIDs cho Display Info
#define DISPLAY_INFO_SERVICE_UUID   "7d790d30-1dff-41a3-833f-5e98f1f70c00"
#define DISPLAY_HARDWARE_CHAR_UUID  "7d790d30-1dff-41a3-833f-5e98f1f70c01"
#define DISPLAY_OTA_CHAR_UUID       "7d790d30-1dff-41a3-833f-5e98f1f70c02"
#define DISPLAY_NAME_CHAR_UUID      "7d790d30-1dff-41a3-833f-5e98f1f70c03"

// Display specific definitions
#define DISPLAY_VERSION             "0.1"
#define INIT_DISPLAY_NAME           "+ UI +"
#define INIT_DISPLAY_HARDWARE       "D-W++"

#define SECURITY_LOCK               "140499"
#define SECURITY_LOCK_LEN           6

#define MAX_VERSION_LENGTH          10
#define MAX_NAME_LENGTH            30
#define MAX_HARDWARE_LENGTH        30

// OTA definitions
#define UPDATE_START_MSG            "START_OTA"
#define UPDATE_END_MSG              "END_OTA"


class BLEDisplayInfo : public BLEServiceManager<3> {
public:
    BLEDisplayInfo(BLEService* service);

    virtual void begin();

    // Getters
    const char* getName();
    const char* getVersion();
    const char* getHardware();

    // Status management (simplified for display)
    void notifyStatusChange(const char* status);

private:
    Preferences pref;

    char version[MAX_VERSION_LENGTH + 1];
    char name[MAX_NAME_LENGTH + 1];
    char hardware[MAX_HARDWARE_LENGTH + 1];

    // OTA variables
    esp_ota_handle_t otaHandler;
    uint16_t packageCounter;

    // Characteristic callbacks
    void onReadName(BLECharacteristic* pChar);
    void onWriteName(BLECharacteristic* pChar);

    void onReadVersion(BLECharacteristic* pChar);

    void onReadHardware(BLECharacteristic* pChar);
    void onWriteHardware(BLECharacteristic* pChar);

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