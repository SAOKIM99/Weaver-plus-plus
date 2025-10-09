#include "BLEBikeManager.h"
#include "BikeRFIDManager.h"

BLEBikeManager::BLEBikeManager() : 
    pServer(nullptr), 
    pAdvertising(nullptr),
    bikeInfoService(nullptr),
    connected(false),
    secured(false),
    currentState(BIKE_STATE_IDLE),
    connectionParam(nullptr),
    pairingInProgress(false),
    pairingStartTime(0),
    rfidManager(nullptr) {
}

BLEBikeManager::~BLEBikeManager() {
    if (bikeInfoService) {
        delete bikeInfoService;
    }
}

void BLEBikeManager::begin() {
    Serial.println("========================================");
    Serial.println("=== BLE Bike Manager with Bonding ===");
    Serial.println("========================================");
    
    // Initialize boot button
    pinMode(MANUAL_AUTHENTICATION_PIN, INPUT_PULLUP);
    
    // Initialize NimBLE
    NimBLEDevice::init("+ SAO KIM +");
    setPower();
    
    // Setup BLE security BEFORE creating server
    setupSecurity();
    
    // Setup server and services
    setupServer();
    setupServices();
    setupAdvertising();
    
    // Start advertising
    startAdvertising();
    
    // Show bonded device count from NimBLE stack
    int bondCount = getBondedDevicesFromStack();
    Serial.print("Found ");
    Serial.print(bondCount);
    Serial.println(" bonded devices in NimBLE stack");
    
    Serial.println("BLE Bike Manager initialized successfully");
    Serial.println("=== Security Instructions ===");
    Serial.println("1. Device discoverable as '+ SAO KIM +'");
    Serial.println("2. FIRST TIME PAIRING:");
    Serial.println("   - Connect from your device");
    Serial.println("   - Check PIN displayed on Serial");
    Serial.println("   - Press BOOT button OR scan RFID to accept");
    Serial.println("   - Timeout: 10 seconds");
    Serial.println("3. BONDED DEVICES:");
    Serial.println("   - Auto-reconnect without confirmation");
    Serial.println("   - Encrypted connection");
    Serial.println("4. CLEAR ALL BONDS:");
    Serial.println("   - Call clearBondedDevices() function");
    Serial.println("========================================");
}

void BLEBikeManager::setRFIDManager(BikeRFIDManager* rfid) {
    rfidManager = rfid;
    Serial.println("RFID Manager linked to BLE Manager");
}

void BLEBikeManager::update() {
    // Minimal update - BLE Bonding handles everything automatically
    // No need to check boot button during operation
    // Bonding is managed by NimBLE stack
}

// Security Functions Implementation  
bool BLEBikeManager::isRFIDAuthenticated() {
    if (rfidManager == nullptr) {
        return false;
    }
    
    // Check if authorized RFID card is present
    return rfidManager->authenticateCard();
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

void BLEBikeManager::setupSecurity() {
    Serial.println("Setting up BLE Security (Bonding)...");
    
    // Use RANDOM address for privacy (RPA - Resolvable Private Address)
    NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
    
    // Enable Secure Connections + MITM + Bonding
    // setSecurityAuth(bond, mitm, sc)
    NimBLEDevice::setSecurityAuth(
        true,   // bonding: Save keys for auto-reconnection
        true,   // mitm: Man-in-the-middle protection  
        true    // sc: Secure Connections (BLE 4.2+)
    );
    
    // I/O Capability: DISPLAY_YESNO
    // - Display PIN on Serial
    // - Require Yes/No confirmation via BOOT button or RFID
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);
    
    // Key Distribution: LTK (encryption) + IRK (identity resolution)
    NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
    NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
    
    // Passkey not used for Just Works mode
    NimBLEDevice::setSecurityPasskey(0);
    
    Serial.println("BLE Security configured:");
    Serial.println("- Bonding: ENABLED (auto-reconnect)");
    Serial.println("- MITM Protection: ENABLED");
    Serial.println("- Secure Connections: ENABLED");
    Serial.println("- I/O: Display + Yes/No (BOOT/RFID)");
    Serial.println("- Privacy: Random Address (RPA)");
}

void BLEBikeManager::setPower() {
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_DEFAULT);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_ADV);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_SCAN);
}

void BLEBikeManager::onConnect(BLEServer* pServer, ble_gap_conn_desc* param) {
    Serial.println("========================================");
    Serial.println("[BLE] Connection Request");
    
    if (!param) {
        Serial.println("[BLE] Error: No connection info");
        return;
    }
    
    // Delay để đảm bảo connection ổn định
    delay(100);
    
    // Kiểm tra bonding status
    if (param->sec_state.bonded) {
        // Thiết bị đã bonded - tự động chấp nhận
        Serial.println("[BLE] ✓ Device ALREADY BONDED - Auto-accept");
        Serial.println("[BLE] Encrypted connection established");
        
        connected = true;
        secured = true;
        currentState = BIKE_STATE_SECURED;
        
        // Stop advertising
        if (pAdvertising->isAdvertising()) {
            pAdvertising->stop();
        }
        
        // Set connection parameters
        if (param->role) { // Slave role = 1
            connectionParam = param;
            
            // Set MTU
            int err = NimBLEDevice::setMTU(517);
            Serial.printf("[BLE] MTU: %d\n", err);

            // Update connection parameters
            pServer->updateConnParams(param->conn_handle, 0x10, 0x20, 0, 400);
            
            // Notify bike status
            if (bikeInfoService) {
                bikeInfoService->notifyStatusChange(BIKE_ON);
            }
        }
        
        Serial.println("[BLE] Connected successfully (bonded device)");
    } else {
        // Thiết bị chưa bonded - bắt đầu pairing
        Serial.println("[BLE] New device - Starting pairing...");
        
        uint16_t connHandle = param->conn_handle;
        
        // Khởi tạo security/pairing process
        if (NimBLEDevice::startSecurity(connHandle)) {
            Serial.println("[BLE] Pairing request sent");
            Serial.println("[BLE] Waiting for PIN confirmation...");
            pairingInProgress = true;
            pairingStartTime = millis();
        } else {
            Serial.println("[BLE] Failed to start pairing - will retry");
        }
    }
    
    Serial.println("========================================");
}

void BLEBikeManager::onDisconnect(BLEServer* pServer, ble_gap_conn_desc* param) {
    if (param->role) { // Slave role = 1
        connected = false;
        secured = false;
        pairingInProgress = false;
        currentState = BIKE_STATE_IDLE;
        connectionParam = nullptr;
        
        Serial.println("[BLE] Device disconnected");
        
        // Notify bike status
        if (bikeInfoService) {
            bikeInfoService->notifyStatusChange(BIKE_OFF);
        }
        
        // Restart advertising
        startAdvertising();
    }
}

void BLEBikeManager::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    if (!desc) return;
    
    Serial.println("========================================");
    if (desc->sec_state.bonded && desc->sec_state.encrypted) {
        Serial.println("[BLE] ✓ PAIRING/BONDING SUCCESS");
        Serial.println("[BLE] - Connection: BONDED");
        Serial.println("[BLE] - Encryption: ENABLED");
        
        // Get peer address
        NimBLEAddress peerAddr(desc->peer_ota_addr);
        Serial.print("[BLE] - Peer Address: ");
        Serial.println(peerAddr.toString().c_str());
        Serial.println("[BLE] Device saved - next time will auto-connect");
        
        // Mark as connected and secured
        connected = true;
        secured = true;
        currentState = BIKE_STATE_SECURED;
        pairingInProgress = false;
        
        // Stop advertising
        if (pAdvertising && pAdvertising->isAdvertising()) {
            pAdvertising->stop();
        }
        
        // Set connection parameters
        if (desc->role) {
            connectionParam = desc;
            
            // Set MTU
            int err = NimBLEDevice::setMTU(517);
            Serial.printf("[BLE] MTU: %d\n", err);

            // Update connection parameters
            pServer->updateConnParams(desc->conn_handle, 0x10, 0x20, 0, 400);
            
            // Notify bike status
            if (bikeInfoService) {
                bikeInfoService->notifyStatusChange(BIKE_ON);
            }
        }
    } else {
        Serial.println("[BLE] ✗ PAIRING FAILED");
        Serial.println("[BLE] - Not bonded or encrypted");
        pairingInProgress = false;
    }
    Serial.println("========================================");
}

uint32_t BLEBikeManager::onPassKeyRequest() {
    Serial.println("[BLE] PassKey request - not used for Yes/No mode");
    return 0;
}

bool BLEBikeManager::onConfirmPIN(uint32_t pin) {
    Serial.println("========================================");
    Serial.println("[BLE] ⚠️  PAIRING CONFIRMATION REQUIRED!");
    Serial.print("[BLE] PIN displayed: ");
    Serial.println(pin);
    Serial.println("[BLE] >> PRESS BOOT BUTTON to accept <<");
    Serial.println("[BLE] >> OR SCAN RFID CARD to accept <<");
    Serial.println("[BLE] Timeout: 10 seconds");
    Serial.println("========================================");
    
    unsigned long startTime = millis();
    bool accepted = false;
    
    while (millis() - startTime < 10000) {  // 10 second timeout
        // Check BOOT button
        if (digitalRead(MANUAL_AUTHENTICATION_PIN) == LOW) {
            Serial.println("[BLE] ✓ BOOT button pressed - PAIRING ACCEPTED");
            accepted = true;
            
            // Wait for button release
            while (digitalRead(MANUAL_AUTHENTICATION_PIN) == LOW) {
                delay(10);
            }
            break;
        }
        
        // Check RFID authentication
        if (isRFIDAuthenticated()) {
            Serial.println("[BLE] ✓ RFID authenticated - PAIRING ACCEPTED");
            accepted = true;
            break;
        }
        
        delay(100);
    }
    
    if (!accepted) {
        Serial.println("[BLE] ✗ Timeout - PAIRING REJECTED");
    }
    
    return accepted;
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
    Serial.println("========================================");
    Serial.println("[BLE] Clearing ALL bonded devices...");
    
    // Sử dụng NimBLE API để xóa tất cả bonded devices
    int count = ble_store_clear();
    
    Serial.printf("[BLE] Cleared %d bonded device(s)\n", count >= 0 ? count : 0);
    Serial.println("[BLE] All bonding information deleted");
    Serial.println("[BLE] Next connection will require pairing");
    Serial.println("========================================");
}

uint8_t BLEBikeManager::getBondedDeviceCount() {
    return getBondedDevicesFromStack();
}

int BLEBikeManager::getBondedDevicesFromStack() {
    // Get bonded device count from NimBLE stack
    int count = 0;
    ble_addr_t peer_id_addrs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
    int num_peers;
    
    // Get all bonded peer addresses
    ble_store_util_bonded_peers(peer_id_addrs, &num_peers, MYNEWT_VAL(BLE_STORE_MAX_BONDS));
    count = num_peers;
    
    return count;
}

void BLEBikeManager::printBondedDevices() {
    Serial.println("========================================");
    Serial.println("[BLE] Bonded Devices List:");
    
    ble_addr_t peer_id_addrs[MYNEWT_VAL(BLE_STORE_MAX_BONDS)];
    int num_peers;
    
    // Get all bonded peer addresses
    ble_store_util_bonded_peers(peer_id_addrs, &num_peers, MYNEWT_VAL(BLE_STORE_MAX_BONDS));
    
    if (num_peers == 0) {
        Serial.println("  (No bonded devices)");
    } else {
        for (int i = 0; i < num_peers; i++) {
            char addr_str[18];
            sprintf(addr_str, "%02x:%02x:%02x:%02x:%02x:%02x",
                    peer_id_addrs[i].val[5], peer_id_addrs[i].val[4],
                    peer_id_addrs[i].val[3], peer_id_addrs[i].val[2],
                    peer_id_addrs[i].val[1], peer_id_addrs[i].val[0]);
            Serial.printf("  %d. %s\n", i + 1, addr_str);
        }
    }
    Serial.printf("Total: %d bonded device(s)\n", num_peers);
    Serial.println("========================================");
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
    return digitalRead(MANUAL_AUTHENTICATION_PIN) == LOW;
}
