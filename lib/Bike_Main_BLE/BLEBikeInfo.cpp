#include "BLEBikeInfo.h"
#include <esp_ota_ops.h>
#include <esp_partition.h>

BLEBikeInfo::BLEBikeInfo(BLEService* service) : 
    BLEServiceManager(service), currentStatus(BIKE_OFF), otaHandler(0), packageCounter(0) {
    addReadWrite(BIKE_HARDWARE_CHAR_UUID, 
        authed([this](BLECharacteristic* p) { onReadHardware(p); }), 
        authed([this](BLECharacteristic* p) { onWriteHardware(p); }));

    addReadWrite(BIKE_OTA_CHAR_UUID,
        authed([this](BLECharacteristic* p) {onReadOTA(p);}),
        authed([this](BLECharacteristic* p) {onWriteOTA(p);}), true);
    
    addReadWrite(BIKE_NAME_CHAR_UUID, 
        authed([this](BLECharacteristic* p) { onReadName(p); }), 
        authed([this](BLECharacteristic* p) { onWriteName(p); }));

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


void BLEBikeInfo::onReadOTA(BLECharacteristic* pChar) {
	pChar->setValue(String(BIKE_VERSION));
}

void BLEBikeInfo::onWriteOTA(BLECharacteristic* pChar) {
  // OTA save on esp_ota
  std::string rxData = pChar->getValue();
  if (rxData == UPDATE_START_MSG) {
    setOTA();
    Serial.println("Begin OTA");
    const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
    Serial.println("Found partition");
    esp_err_t result = esp_ota_begin(partition, OTA_SIZE_UNKNOWN, &otaHandler);
    if (result == ESP_OK) {
      //Serial.println("OTA operation commenced successfully");
    } else {
      Serial.print("Failed to commence OTA operation, error: ");
      Serial.println(result);
      resetOTA();
      return;
    }
    packageCounter = 0;
    Serial.println("Begin OTA done");
  }
  else if (rxData == UPDATE_END_MSG) {
    Serial.println("OTA: Upload completed");
    esp_err_t result = esp_ota_end(otaHandler);
    if (result == ESP_OK) {
      //Serial.println("Newly written OTA app image is valid.");
    } else {
      Serial.print("Failed to validate OTA app image, error: ");
      Serial.println(result);
    }
    if (esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL)) == ESP_OK) {
      delay(1000);
      esp_restart();
    } else {
      Serial.println("OTA Error: Invalid boot partition");
      delay(1000);
      resetOTA();
    }
  }
  else {
    Serial.print(rxData.length()); 
    Serial.print("\t");

    if (esp_ota_write(otaHandler, rxData.c_str(), rxData.length()) == ESP_OK) {
      packageCounter++;
    }
    else {
      Serial.println("OTA is Fail, please try again!!!");
      packageCounter = UINT16_MAX;
    }
    Serial.println(packageCounter);
    onNotifyOTA(packageCounter);
    // Make the writes much more reliable
    vTaskDelay(1);
  }
}

void BLEBikeInfo::onNotifyOTA(uint32_t stateOTA) {
  BLECharacteristic* pCharacteristic = service->getCharacteristic(BIKE_OTA_CHAR_UUID);
  pCharacteristic->setValue(stateOTA);
  pCharacteristic->notify();
}

void BLEBikeInfo::setOTA() {
    // Set some flag or state to indicate OTA is in progress
    Serial.println("OTA mode activated");
}

void BLEBikeInfo::resetOTA() {
    otaHandler = 0;
    packageCounter = 0;
    Serial.println("OTA reset");
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
