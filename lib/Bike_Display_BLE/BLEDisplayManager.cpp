#include "BLEDisplayManager.h"

BLEDisplayManager::BLEDisplayManager() :
    pServer(nullptr),
    pAdvertising(nullptr),
    displayInfoService(nullptr),
    connected(false),
    currentState(DISPLAY_STATE_IDLE),
    connectionParam(nullptr) {
}

BLEDisplayManager::~BLEDisplayManager() {
    if (displayInfoService) {
        delete displayInfoService;
    }
}

void BLEDisplayManager::begin() {
    Serial.println("========================================");
    Serial.println("=== BLE Display Manager (Simple) ===");
    Serial.println("========================================");

    // Initialize NimBLE
    NimBLEDevice::init("+ SAO KIM DISPLAY +");
    setPower();

    // Setup server and services
    setupServer();
    setupServices();
    setupAdvertising();

    Serial.println("BLE Display Manager initialized successfully");
    Serial.println("=== Connection Instructions ===");
    Serial.println("1. Device discoverable as '+ SAO KIM DISPLAY +'");
    Serial.println("2. Simple connection - no bonding required");
    Serial.println("3. OTA firmware update available");
    Serial.println("========================================");
}

void BLEDisplayManager::update() {
    // Minimal update - BLE Bonding handles everything automatically
    // No need to check boot button during operation
    // Bonding is managed by NimBLE stack
}

void BLEDisplayManager::setupServer() {
    // Create the BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(this);

    Serial.println("BLE Display Server created");
}

void BLEDisplayManager::setupServices() {
    // Create Info Service
    // Initialize display info service
    displayInfoService = new BLEDisplayInfo(pServer->createService(DISPLAY_INFO_SERVICE_UUID));
    displayInfoService->begin();

    Serial.println("Display Info Service started");
}

void BLEDisplayManager::setupAdvertising() {
    // Get display name for advertising
    const char* displayName = displayInfoService ? displayInfoService->getName() : INIT_DISPLAY_NAME;

    // Setup simple advertising data
    BLEAdvertisementData data;
    data.setName(displayName);

    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(DISPLAY_ADV_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinInterval(48);
    pAdvertising->setMaxInterval(96);
    pAdvertising->setScanResponseData(data);

    Serial.printf("Advertising setup for: %s\n", displayName);
}

void BLEDisplayManager::setPower() {
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_DEFAULT);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_ADV);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_SCAN);
}

void BLEDisplayManager::onConnect(BLEServer* pServer, ble_gap_conn_desc* param) {
    Serial.println("========================================");
    Serial.println("[DISPLAY BLE] Device Connected");

    if (!param) {
        Serial.println("[DISPLAY BLE] Error: No connection info");
        return;
    }

    // Delay để đảm bảo connection ổn định
    delay(100);

    connected = true;
    currentState = DISPLAY_STATE_CONNECTED;

    // Stop advertising
    if (pAdvertising->isAdvertising()) {
        pAdvertising->stop();
    }

    // Set connection parameters
    if (param->role) { // Slave role = 1
        connectionParam = param;

        // Set MTU
        int err = NimBLEDevice::setMTU(517);
        Serial.printf("[DISPLAY BLE] MTU: %d\n", err);

        // Update connection parameters
        pServer->updateConnParams(param->conn_handle, 0x10, 0x20, 0, 400);
    }

    Serial.println("[DISPLAY BLE] Connected successfully");
    Serial.println("========================================");
}

void BLEDisplayManager::onDisconnect(BLEServer* pServer, ble_gap_conn_desc* param) {
    if (param->role) { // Slave role = 1
        connected = false;
        currentState = DISPLAY_STATE_IDLE;
        connectionParam = nullptr;

        Serial.println("[DISPLAY BLE] Device disconnected");

        // Restart advertising
        startAdvertising();
    }
}

bool BLEDisplayManager::isConnected() {
    return connected;
}

void BLEDisplayManager::startAdvertising() {
    // Check if name changed
    if (displayInfoService) {
        // Re-setup advertising if name changed
        setupAdvertising();
    }

    if (pAdvertising && !pAdvertising->isAdvertising()) {
        pAdvertising->start();
        Serial.println("Display advertising started");
    }
}

void BLEDisplayManager::stopAdvertising() {
    if (pAdvertising && pAdvertising->isAdvertising()) {
        pAdvertising->stop();
        Serial.println("Display advertising stopped");
    }
}

void BLEDisplayManager::setDisplayState(DisplayState state) {
    currentState = state;
}

DisplayState BLEDisplayManager::getDisplayState() {
    return currentState;
}

const char* BLEDisplayManager::getDisplayName() {
    return displayInfoService ? displayInfoService->getName() : "+ SAO KIM DISPLAY +";
}

void BLEDisplayManager::setDisplayName(const char* name) {
    if (displayInfoService && name && strlen(name) > 0) {
        // This would trigger the write characteristic
        Serial.printf("Setting display name to: %s\n", name);
    }
}

const char* BLEDisplayManager::getDisplayVersion() {
    return displayInfoService ? displayInfoService->getVersion() : DISPLAY_VERSION;
}

const char* BLEDisplayManager::getDisplayHardware() {
    return displayInfoService ? displayInfoService->getHardware() : INIT_DISPLAY_HARDWARE;
}

void BLEDisplayManager::notifyDisplayStatusChange(const char* status) {
    if (displayInfoService) {
        displayInfoService->notifyStatusChange(status);
    }
}

void BLEDisplayManager::setConnectionPriority(bool isHigh) {
    if (connected && pServer && connectionParam) {
        if (isHigh) {
            Serial.println("Display ConnectionPriority: high");
            // High priority: min_int = 16 (20ms); max_int = 32 (40ms); timeout = 4s;
            pServer->updateConnParams(connectionParam->conn_handle, 0x10, 0x20, 0, 400);
        } else {
            Serial.println("Display ConnectionPriority: low");
            // Low priority: min_int = 80 (100ms); max_int = 200 (250ms); timeout = 4s;
            pServer->updateConnParams(connectionParam->conn_handle, 50, 100, 0, 400);
        }
    }
}