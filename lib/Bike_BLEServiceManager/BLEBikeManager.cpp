#include "BLEBikeManager.h"

BLEBikeManager::BLEBikeManager() : 
    pServer(nullptr), 
    pAdvertising(nullptr),
    bikeInfoService(nullptr),
    connected(false),
    secured(false),
    currentState(BIKE_STATE_IDLE),
    connectionParam(nullptr),
    pairingInProgress(false),
    bootButtonPressed(false),
    lastBootButtonCheck(0),
    pairingStartTime(0) {
}

BLEBikeManager::~BLEBikeManager() {
    if (bikeInfoService) {
        delete bikeInfoService;
    }
}

void BLEBikeManager::begin() {
    Serial.println("=== BLE Bike Manager Initialization ===");
    
    // Initialize boot button
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    
    // Initialize preferences for storing bonded devices
    preferences.begin("bike-bonds", false);
    
    // Show current bonded devices
    uint8_t bondCount = preferences.getUChar("bond_count", 0);
    Serial.print("Found ");
    Serial.print(bondCount);
    Serial.println(" bonded devices");
    
    // Initialize NimBLE
    NimBLEDevice::init("+ SAO KIM +");
    setPower();
    
    // Setup server and services
    setupServer();
    setupServices();
    setupAdvertising();
    
    // Start advertising
    startAdvertising();
    
    Serial.println("BLE Bike Manager initialized successfully");
    Serial.println("=== Security Instructions ===");
    Serial.println("1. Device discoverable as '+ SAO KIM +'");
    Serial.println("2. FIRST TIME: Press BOOT button to accept connection");
    Serial.println("3. SUBSEQUENT: Auto-accepted for bonded devices");
    Serial.println("4. Hold BOOT 5+ seconds to clear all bonds");
    Serial.println("====================================");
}

void BLEBikeManager::update() {
    // Update boot button handling
    updateBootButton();
    
    // Update connection state
    if (connected && !secured) {
        secured = true;
        currentState = BIKE_STATE_SECURED;
        Serial.println("Bike connection secured!");
    }
}

void BLEBikeManager::setupServer() {
    // Create the BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(this);
    
    Serial.println("BLE Server created");
}

void BLEBikeManager::setupServices() {
    // Create Info Service    
    // Initialize bike info service
    bikeInfoService = new BLEBikeInfo(pServer->createService(BIKE_INFO_SERVICE_UUID));
    bikeInfoService->begin();
    
    // Start the service
    // pInfoService->start();
    
    Serial.println("Bike Info Service started");
}

void BLEBikeManager::setupAdvertising() {
    // Get bike name for advertising
    const char* bikeName = bikeInfoService ? bikeInfoService->getName() : "+ SAO KIM +";
    
    // Setup MAC ID for service data
    char macID[7] = {0};
    const uint8_t* pAddrID = NimBLEDevice::getAddress().getNative();
    uint16_t uuidServiceData = 0xFFFF;

    // MAC address value inversion (tương tự BLEHandManager)
    macID[5] = *(pAddrID + 0);
    macID[4] = *(pAddrID + 1);
    macID[3] = *(pAddrID + 2);
    macID[2] = *(pAddrID + 3);
    macID[1] = *(pAddrID + 4);
    macID[0] = *(pAddrID + 5);

    // Setup advertising data
    BLEAdvertisementData data;
    data.setName(bikeName);
    data.setServiceData(BLEUUID(uuidServiceData), macID);

    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BIKE_INFO_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinInterval(48);
    pAdvertising->setMaxInterval(96);
    pAdvertising->setScanResponseData(data);
    
    Serial.printf("Advertising setup for: %s\n", bikeName);
}

void BLEBikeManager::setPower() {
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_DEFAULT);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_ADV);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_SCAN);
}

void BLEBikeManager::onConnect(BLEServer* pServer, ble_gap_conn_desc* param) {
    Serial.println("=== BIKE CONNECTION REQUEST ===");
    Serial.println("Device attempting to connect...");
    
    // Lấy địa chỉ của thiết bị đang kết nối
    std::vector<uint16_t> connIds = pServer->getPeerDevices();
    BLEAddress deviceAddress("");
    bool deviceFound = false;
    
    if (!connIds.empty()) {
        auto peerInfo = pServer->getPeerInfo(connIds[0]);
        deviceAddress = peerInfo.getAddress();
        deviceFound = true;
        Serial.print("Connecting device: ");
        Serial.println(deviceAddress.toString().c_str());
    }
    
    // Kiểm tra xem thiết bị đã được bonded trước đó chưa
    bool alreadyBonded = deviceFound && isDeviceBonded(deviceAddress);
    
    if (alreadyBonded) {
        // Thiết bị đã bonded - cho phép kết nối ngay lập tức
        Serial.println("Device already bonded - AUTO ACCEPT connection!");
        connected = true;
        secured = true;
        currentState = BIKE_STATE_SECURED;
        Serial.println("Bike connected successfully (auto-accepted)");
        
        // Stop advertising sau khi kết nối
        if (pAdvertising->isAdvertising()) {
            pAdvertising->stop();
        }
        
        // Set connection parameters
        if (param->role) { // Slave role = 1
            connectionParam = param;
            
            // Set MTU
            int err = NimBLEDevice::setMTU(517);
            Serial.printf("setMTU: %d\n", err);

            // Update connection parameters
            pServer->updateConnParams(param->conn_handle, 0x10, 0x20, 0, 400);
            
            // Notify bike status
            if (bikeInfoService) {
                bikeInfoService->notifyStatusChange(BIKE_ON);
            }
        }
    } else {
        // Thiết bị mới - yêu cầu nhấn BOOT button
        Serial.println("New device - BOOT BUTTON REQUIRED!");
        
        // Bắt đầu quá trình kiểm tra pairing
        pairingInProgress = true;
        pairingStartTime = millis();
        
        Serial.println("PRESS AND HOLD BOOT BUTTON NOW to accept connection!");
        Serial.println("You have 30 seconds...");
        
        // Đợi người dùng nhấn nút BOOT trong 30 giây
        unsigned long startTime = millis();
        bool accepted = false;
        
        while (millis() - startTime < PAIRING_TIMEOUT_MS) {
            // Kiểm tra nút BOOT
            if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
                Serial.println("BOOT button pressed - CONNECTION ACCEPTED!");
                accepted = true;
                break;
            }
            delay(100);
        }
        
        pairingInProgress = false;
        
        if (accepted) {
            connected = true;
            secured = true;
            currentState = BIKE_STATE_SECURED;
            Serial.println("Bike connected successfully");
            
            // Lưu thiết bị mới
            if (deviceFound) {
                saveBondedDevice(deviceAddress);
            }
            
            // Stop advertising sau khi kết nối
            if (pAdvertising->isAdvertising()) {
                pAdvertising->stop();
            }
            
            // Set connection parameters
            if (param->role) { // Slave role = 1
                connectionParam = param;
                
                // Set MTU
                int err = NimBLEDevice::setMTU(517);
                Serial.printf("setMTU: %d\n", err);

                // Update connection parameters
                pServer->updateConnParams(param->conn_handle, 0x10, 0x20, 0, 400);
                
                // Notify bike status
                if (bikeInfoService) {
                    bikeInfoService->notifyStatusChange(BIKE_ON);
                }
            }
        } else {
            Serial.println("Timeout - CONNECTION REJECTED!");
            
            // Ngắt kết nối
            for (uint16_t connId : connIds) {
                pServer->disconnect(connId);
            }
            connected = false;
            secured = false;
        }
    }
}

void BLEBikeManager::onDisconnect(BLEServer* pServer, ble_gap_conn_desc* param) {
    if (param->role) { // Slave role = 1
        connected = false;
        secured = false;
        pairingInProgress = false;
        currentState = BIKE_STATE_IDLE;
        connectionParam = nullptr;
        
        Serial.println("Bike disconnected");
        
        // Notify bike status
        if (bikeInfoService) {
            bikeInfoService->notifyStatusChange(BIKE_OFF);
        }
        
        // Restart advertising
        startAdvertising();
    }
}

void BLEBikeManager::handleSecureConnection(BLEServer* pServer, ble_gap_conn_desc* param) {
    // Simple connection without separate security manager
    Serial.println("Direct connection - no separate security service");
}

void BLEBikeManager::handleDisconnection(BLEServer* pServer, ble_gap_conn_desc* param) {
    // Handle cleanup
    Serial.println("Connection cleanup");
}

bool BLEBikeManager::isConnected() {
    return connected;
}

bool BLEBikeManager::isSecured() {
    return secured;
}

bool BLEBikeManager::isPairingInProgress() {
    return pairingInProgress;
}

void BLEBikeManager::startAdvertising() {
    // Check if name changed
    if (bikeInfoService) {
        const char* currentName = bikeInfoService->getName();
        // Re-setup advertising if name changed
        setupAdvertising();
    }
    
    if (pAdvertising && !pAdvertising->isAdvertising()) {
        pAdvertising->start();
        Serial.println("Bike advertising started");
    }
}

void BLEBikeManager::stopAdvertising() {
    if (pAdvertising && pAdvertising->isAdvertising()) {
        pAdvertising->stop();
        Serial.println("Bike advertising stopped");
    }
}

void BLEBikeManager::setBikeState(BikeState state) {
    currentState = state;
    notifyStateChange(state);
}

BikeState BLEBikeManager::getBikeState() {
    return currentState;
}

void BLEBikeManager::clearBondedDevices() {
    clearBondedDevicesInternal();
}

uint8_t BLEBikeManager::getBondedDeviceCount() {
    return preferences.getUChar("bond_count", 0);
}

const char* BLEBikeManager::getBikeName() {
    return bikeInfoService ? bikeInfoService->getName() : "+ SAO KIM +";
}

void BLEBikeManager::setBikeName(const char* name) {
    if (bikeInfoService && name && strlen(name) > 0) {
        // This would trigger the write characteristic
        Serial.printf("Setting bike name to: %s\n", name);
    }
}

BikeOperationState BLEBikeManager::getBikeStatus() {
    return bikeInfoService ? bikeInfoService->getStatus() : BIKE_OFF;
}

void BLEBikeManager::setBikeStatus(BikeOperationState status) {
    if (bikeInfoService) {
        bikeInfoService->setStatus(status);
    }
}

void BLEBikeManager::notifyBikeStatusChange(BikeOperationState status) {
    if (bikeInfoService) {
        bikeInfoService->notifyStatusChange(status);
    }
}

void BLEBikeManager::notifyStateChange(BikeState state) {
    Serial.printf("Bike state changed to: %d\n", state);
    // Additional notifications can be implemented here
}

void BLEBikeManager::setConnectionPriority(bool isHigh) {
    if (connected && pServer && connectionParam) {
        if (isHigh) {
            Serial.println("Bike ConnectionPriority: high");
            // High priority: min_int = 16 (20ms); max_int = 32 (40ms); timeout = 4s;
            pServer->updateConnParams(connectionParam->conn_handle, 0x10, 0x20, 0, 400);
        } else {
            Serial.println("Bike ConnectionPriority: low");
            // Low priority: min_int = 80 (100ms); max_int = 200 (250ms); timeout = 4s;
            pServer->updateConnParams(connectionParam->conn_handle, 50, 100, 0, 400);
        }
    }
}

// Security Functions Implementation
bool BLEBikeManager::isBootButtonPressed() {
    return digitalRead(BOOT_BUTTON_PIN) == LOW;
}

void BLEBikeManager::updateBootButton() {
    static unsigned long bootButtonPressStart = 0;
    unsigned long currentTime = millis();
    
    // Check boot button every 50ms to debounce
    if (currentTime - lastBootButtonCheck > 50) {
        bool buttonPressed = isBootButtonPressed();
        
        // Detect button press (falling edge)
        if (buttonPressed && !bootButtonPressed) {
            bootButtonPressStart = currentTime;
        }
        
        // Detect button release (rising edge) - chỉ khi không đang pairing
        if (!buttonPressed && bootButtonPressed && !pairingInProgress) {
            unsigned long pressDuration = currentTime - bootButtonPressStart;
            
            if (pressDuration >= 5000) { // 5 seconds or more
                Serial.println("Long press detected - Clearing all bonded devices");
                clearBondedDevicesInternal();
            }
        }
        
        bootButtonPressed = buttonPressed;
        lastBootButtonCheck = currentTime;
    }
}

void BLEBikeManager::saveBondedDevice(BLEAddress address) {
    // Check if device is already bonded
    if (isDeviceBonded(address)) {
        Serial.println("Device already bonded");
        return;
    }
    
    uint8_t bondCount = preferences.getUChar("bond_count", 0);
    
    if (bondCount < MAX_BONDED_DEVICES) {
        String key = "bond_" + String(bondCount);
        preferences.putString(key.c_str(), address.toString().c_str());
        preferences.putUChar("bond_count", bondCount + 1);
        
        Serial.print("New device bonded: ");
        Serial.println(address.toString().c_str());
        Serial.print("Total bonded devices: ");
        Serial.println(bondCount + 1);
    } else {
        Serial.println("Maximum bonded devices reached!");
    }
}

bool BLEBikeManager::isDeviceBonded(BLEAddress address) {
    uint8_t bondCount = preferences.getUChar("bond_count", 0);
    
    for (uint8_t i = 0; i < bondCount; i++) {
        String key = "bond_" + String(i);
        String bondedAddress = preferences.getString(key.c_str(), "");
        
        if (bondedAddress == address.toString().c_str()) {
            return true;
        }
    }
    
    return false;
}

void BLEBikeManager::clearBondedDevicesInternal() {
    preferences.clear();
    Serial.println("=== ALL BONDED DEVICES CLEARED ===");
    Serial.println("All previous pairings removed");
    
    // Disconnect current connection if any
    if (connected && pServer) {
        std::vector<uint16_t> connIds = pServer->getPeerDevices();
        for (uint16_t connId : connIds) {
            pServer->disconnect(connId);
        }
    }
}
