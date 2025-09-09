#include "BLEBikeInfo.h"

BLEBikeInfo::BLEBikeInfo(BLEService* service) : 
    BLEServiceManager(service), currentStatus(BIKE_OFF) {
    
    // Add characteristics với bảo mật
    addReadWrite(BIKE_NAME_CHAR_UUID, 
        authed([this](BLECharacteristic* p) { onReadName(p); }), 
        authed([this](BLECharacteristic* p) { onWriteName(p); }));

    addReadWrite(BIKE_HARDWARE_CHAR_UUID, 
        authed([this](BLECharacteristic* p) { onReadHardware(p); }), 
        authed([this](BLECharacteristic* p) { onWriteHardware(p); }));

    addReadWrite(BIKE_STATUS_CHAR_UUID, 
        authed([this](BLECharacteristic* p) { onReadStatus(p); }), 
        NULL, true); // Chỉ read và notify
}

void BLEBikeInfo::begin() {
    BLEServiceManager::begin();
    
    pref.begin("bike-info", false);
    
    // Load saved data from preferences
    getStringPref(pref, "name", name, MAX_NAME_LENGTH, INIT_BIKE_NAME);
    getStringPref(pref, "hardware", hardware, MAX_HARDWARE_LENGTH, INIT_BIKE_HARDWARE);
    strcpy(version, BIKE_VERSION);
    
    Serial.printf("Bike Name: %s\n", name);
    Serial.printf("Bike Version: %s\n", version);
    Serial.printf("Bike Hardware: %s\n", hardware);
    Serial.printf("Bike Status: %d\n", currentStatus);
}

const char* BLEBikeInfo::getName() {
    return name;
}

const char* BLEBikeInfo::getVersion() {
    return version;
}

const char* BLEBikeInfo::getHardware() {
    return hardware;
}

BikeOperationState BLEBikeInfo::getStatus() {
    return currentStatus;
}

void BLEBikeInfo::setStatus(BikeOperationState status) {
    currentStatus = status;
    notifyStatusChange(status);
}

void BLEBikeInfo::notifyStatusChange(BikeOperationState status) {
    BLECharacteristic* pChar = service->getCharacteristic(BIKE_STATUS_CHAR_UUID);
    if (pChar != nullptr) {
        uint8_t statusValue = (uint8_t)status;
        pChar->setValue(&statusValue, sizeof(statusValue));
        pChar->notify();
    }
}

void BLEBikeInfo::onReadName(BLECharacteristic* pChar) {
    pChar->setValue(String(name));
}

void BLEBikeInfo::onWriteName(BLECharacteristic* pChar) {
    const char* newName = pChar->getValue().c_str();
    if (strlen(newName) > 0 && strlen(newName) <= MAX_NAME_LENGTH) {
        memcpy(name, newName, strlen(newName) + 1);
        pref.putBytes("name", newName, strlen(newName));
        
        Serial.printf("Bike name updated: %s\n", name);
    } else {
        Serial.println("Invalid bike name length");
    }
}

void BLEBikeInfo::onReadHardware(BLECharacteristic* pChar) {
    pChar->setValue(String(hardware));
}

void BLEBikeInfo::onWriteHardware(BLECharacteristic* pChar) {
    const char* input = pChar->getValue().c_str();
    
    if (strlen(input) > SECURITY_LOCK_LEN) {
        char key[SECURITY_LOCK_LEN + 1];
        strncpy(key, input, SECURITY_LOCK_LEN);
        key[SECURITY_LOCK_LEN] = '\0';

        if (strcmp(key, SECURITY_LOCK) == 0) {
            // Security key is valid, update hardware info
            const char* newHardware = input + SECURITY_LOCK_LEN;
            size_t hardwareLen = strlen(input) - SECURITY_LOCK_LEN;
            
            if (hardwareLen <= MAX_HARDWARE_LENGTH) {
                memcpy(hardware, newHardware, hardwareLen);
                hardware[hardwareLen] = '\0';
                pref.putBytes("hardware", hardware, hardwareLen);
                
                Serial.printf("Hardware updated: %s\n", hardware);
            } else {
                Serial.println("Hardware info too long");
            }
        } else {
            Serial.println("Invalid security key for hardware update");
        }
    } else {
        Serial.println("Input too short for hardware update");
    }
}

void BLEBikeInfo::onReadStatus(BLECharacteristic* pChar) {
    uint8_t statusValue = (uint8_t)currentStatus;
    pChar->setValue(&statusValue, sizeof(statusValue));
}

void BLEBikeInfo::getStringPref(Preferences& p, const char* key, char* data, size_t maxLen, const char* defaultVal) {
    size_t len = p.getBytes(key, data, maxLen);
    if (len == 0) {
        len = strlen(defaultVal);
        memcpy(data, defaultVal, len);
    }
    data[len] = '\0';
}

bool BLEBikeInfo::validateSecurityKey(const char* input) {
    if (strlen(input) < SECURITY_LOCK_LEN) {
        return false;
    }
    
    char key[SECURITY_LOCK_LEN + 1];
    strncpy(key, input, SECURITY_LOCK_LEN);
    key[SECURITY_LOCK_LEN] = '\0';
    
    return strcmp(key, SECURITY_LOCK) == 0;
}
