#include "BLEDisplayInfo.h"
#include <esp_ota_ops.h>
#include <esp_partition.h>

BLEDisplayInfo::BLEDisplayInfo(BLEService* service) :
    BLEServiceManager(service), otaHandler(0), packageCounter(0) {
    addReadWrite(DISPLAY_HARDWARE_CHAR_UUID,
        authed([this](BLECharacteristic* p) { onReadHardware(p); }),
        authed([this](BLECharacteristic* p) { onWriteHardware(p); }));

    addReadWrite(DISPLAY_OTA_CHAR_UUID,
        authed([this](BLECharacteristic* p) {onReadOTA(p);}),
        authed([this](BLECharacteristic* p) {onWriteOTA(p);}), true);

    // Add characteristics với bảo mật
    addReadWrite(DISPLAY_NAME_CHAR_UUID,
        authed([this](BLECharacteristic* p) { onReadName(p); }),
        authed([this](BLECharacteristic* p) { onWriteName(p); }));


}

void BLEDisplayInfo::begin() {
    BLEServiceManager::begin();

    pref.begin("display-info", false);

    // Load saved data from preferences
    getStringPref(pref, "name", name, MAX_NAME_LENGTH, INIT_DISPLAY_NAME);
    getStringPref(pref, "hardware", hardware, MAX_HARDWARE_LENGTH, INIT_DISPLAY_HARDWARE);
    strcpy(version, DISPLAY_VERSION);

    Serial.printf("Display Name: %s\n", name);
    Serial.printf("Display Version: %s\n", version);
    Serial.printf("Display Hardware: %s\n", hardware);
}

const char* BLEDisplayInfo::getName() {
    return name;
}

const char* BLEDisplayInfo::getVersion() {
    return version;
}

const char* BLEDisplayInfo::getHardware() {
    return hardware;
}

void BLEDisplayInfo::notifyStatusChange(const char* status) {
    // For display, we can notify status changes if needed
    Serial.printf("Display status: %s\n", status);
}


void BLEDisplayInfo::onReadOTA(BLECharacteristic* pChar) {
	pChar->setValue(String(DISPLAY_VERSION));
}

void BLEDisplayInfo::onWriteOTA(BLECharacteristic* pChar) {
  // OTA save on esp_ota
  std::string rxData = pChar->getValue();
  if (rxData == UPDATE_START_MSG) {
    setOTA();
    Serial.println("Display OTA: Begin OTA");
    const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
    Serial.println("Display OTA: Found partition");
    esp_err_t result = esp_ota_begin(partition, OTA_SIZE_UNKNOWN, &otaHandler);
    if (result == ESP_OK) {
      //Serial.println("OTA operation commenced successfully");
    } else {
      Serial.print("Display OTA: Failed to commence OTA operation, error: ");
      Serial.println(result);
      resetOTA();
      return;
    }
    packageCounter = 0;
    Serial.println("Display OTA: Begin OTA done");
  }
  else if (rxData == UPDATE_END_MSG) {
    Serial.println("Display OTA: Upload completed");
    esp_err_t result = esp_ota_end(otaHandler);
    if (result == ESP_OK) {
      //Serial.println("Newly written OTA app image is valid.");
    } else {
      Serial.print("Display OTA: Failed to validate OTA app image, error: ");
      Serial.println(result);
    }
    if (esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL)) == ESP_OK) {
      delay(1000);
      esp_restart();
    } else {
      Serial.println("Display OTA Error: Invalid boot partition");
      delay(1000);
      resetOTA();
    }
  }
  else {
    Serial.print("Display OTA: Received ");
    Serial.print(rxData.length());
    Serial.print(" bytes\t");

    if (esp_ota_write(otaHandler, rxData.c_str(), rxData.length()) == ESP_OK) {
      packageCounter++;
    }
    else {
      Serial.println("Display OTA: Write failed, please try again!!!");
      packageCounter = UINT16_MAX;
    }
    Serial.println(packageCounter);
    onNotifyOTA(packageCounter);
    // Make the writes much more reliable
    vTaskDelay(1);
  }
}

void BLEDisplayInfo::onNotifyOTA(uint32_t stateOTA) {
  BLECharacteristic* pCharacteristic = service->getCharacteristic(DISPLAY_OTA_CHAR_UUID);
  pCharacteristic->setValue(stateOTA);
  pCharacteristic->notify();
}

void BLEDisplayInfo::setOTA() {
    // Set some flag or state to indicate OTA is in progress
    Serial.println("Display OTA: OTA mode activated");
}

void BLEDisplayInfo::resetOTA() {
    otaHandler = 0;
    packageCounter = 0;
    Serial.println("Display OTA: OTA reset");
}

void BLEDisplayInfo::onReadName(BLECharacteristic* pChar) {
    pChar->setValue(String(name));
}

void BLEDisplayInfo::onWriteName(BLECharacteristic* pChar) {
    const char* newName = pChar->getValue().c_str();
    if (strlen(newName) > 0 && strlen(newName) <= MAX_NAME_LENGTH) {
        memcpy(name, newName, strlen(newName) + 1);
        pref.putBytes("name", newName, strlen(newName));

        Serial.printf("Display name updated: %s\n", name);
    } else {
        Serial.println("Invalid display name length");
    }
}

void BLEDisplayInfo::onReadVersion(BLECharacteristic* pChar) {
    pChar->setValue(String(version));
}

void BLEDisplayInfo::onReadHardware(BLECharacteristic* pChar) {
    pChar->setValue(String(hardware));
}

void BLEDisplayInfo::onWriteHardware(BLECharacteristic* pChar) {
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

                Serial.printf("Display hardware updated: %s\n", hardware);
            } else {
                Serial.println("Display hardware info too long");
            }
        } else {
            Serial.println("Invalid security key for display hardware update");
        }
    } else {
        Serial.println("Input too short for display hardware update");
    }
}

void BLEDisplayInfo::getStringPref(Preferences& p, const char* key, char* data, size_t maxLen, const char* defaultVal) {
    size_t len = p.getBytes(key, data, maxLen);
    if (len == 0) {
        len = strlen(defaultVal);
        memcpy(data, defaultVal, len);
    }
    data[len] = '\0';
}

bool BLEDisplayInfo::validateSecurityKey(const char* input) {
    if (strlen(input) < SECURITY_LOCK_LEN) {
        return false;
    }

    char key[SECURITY_LOCK_LEN + 1];
    strncpy(key, input, SECURITY_LOCK_LEN);
    key[SECURITY_LOCK_LEN] = '\0';

    return strcmp(key, SECURITY_LOCK) == 0;
}