#include "BikeRFIDManager.h"

BikeRFIDManager::BikeRFIDManager() : 
    mfrc522(SS_PIN, RST_PIN),
    bikeUnlocked(false),
    lastCardTime(0) {
}

BikeRFIDManager::~BikeRFIDManager() {
}

void BikeRFIDManager::begin() {
    Serial.println("=== RFID Manager Initialization ===");
    
    // Initialize SPI bus
    SPI.begin();
    
    // Initialize MFRC522
    mfrc522.PCD_Init();
    mfrc522.PCD_DumpVersionToSerial();
    
    // Initialize preferences
    preferences.begin("bike-rfid", false);
    
    // Load bike state
    loadBikeState();
    
    Serial.printf("RFID Manager initialized - Bike %s\n", 
                  bikeUnlocked ? "UNLOCKED" : "LOCKED");
}

void BikeRFIDManager::update() {
    // Check for RFID card
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String uid = getCardUID();
        
        // Debounce - ignore same card within 2 seconds
        if (uid != lastCardUID || (millis() - lastCardTime) > 2000) {
            Serial.printf("RFID Card detected: %s\n", uid.c_str());
            processCard(uid);
            lastCardUID = uid;
            lastCardTime = millis();
        }
        
        // Stop reading
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    }
}

void BikeRFIDManager::processCard(String uid) {
    if (isCardAuthorized(uid)) {
        Serial.println("Authorized card detected!");
        toggleBikeLock();
    } else {
        Serial.println("Unauthorized card - Access denied!");
    }
}

bool BikeRFIDManager::isCardAuthorized(String uid) {
    String key = "card_" + uid;
    return preferences.getBool(key.c_str(), false);
}

void BikeRFIDManager::addAuthorizedCard(String uid) {
    String key = "card_" + uid;
    preferences.putBool(key.c_str(), true);
    
    // Also add to list for management
    uint8_t cardCount = preferences.getUChar("card_count", 0);
    String listKey = "cardlist_" + String(cardCount);
    preferences.putString(listKey.c_str(), uid);
    preferences.putUChar("card_count", cardCount + 1);
    
    Serial.printf("Card added: %s\n", uid.c_str());
}

void BikeRFIDManager::removeAuthorizedCard(String uid) {
    String key = "card_" + uid;
    preferences.remove(key.c_str());
    Serial.printf("Card removed: %s\n", uid.c_str());
}

void BikeRFIDManager::clearAllCards() {
    preferences.clear();
    Serial.println("All authorized cards cleared");
}

void BikeRFIDManager::toggleBikeLock() {
    if (bikeUnlocked) {
        lockBike();
    } else {
        unlockBike();
    }
}

void BikeRFIDManager::lockBike() {
    bikeUnlocked = false;
    digitalWrite(KEY_PIN, LOW);
    saveBikeState();
    Serial.println("🔒 BIKE LOCKED");
}

void BikeRFIDManager::unlockBike() {
    bikeUnlocked = true;
    digitalWrite(KEY_PIN, HIGH);
    saveBikeState();
    Serial.println("🔓 BIKE UNLOCKED");
}

bool BikeRFIDManager::isBikeUnlocked() {
    return bikeUnlocked;
}

void BikeRFIDManager::saveBikeState() {
    preferences.putBool("bike_unlocked", bikeUnlocked);
}

void BikeRFIDManager::loadBikeState() {
    bikeUnlocked = preferences.getBool("bike_unlocked", false);
    
    // Set KEY_PIN according to saved state
    pinMode(KEY_PIN, OUTPUT);
    digitalWrite(KEY_PIN, bikeUnlocked ? HIGH : LOW);
}

String BikeRFIDManager::getCardUID() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (i > 0) uid += ":";
        if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    return uid;
}

bool BikeRFIDManager::isCardPresent() {
    return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

bool BikeRFIDManager::authenticateCard() {
    if (isCardPresent()) {
        String uid = getCardUID();
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return isCardAuthorized(uid);
    }
    return false;
}
