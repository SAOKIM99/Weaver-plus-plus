#ifndef BIKE_RFID_MANAGER_H
#define BIKE_RFID_MANAGER_H

#include <Preferences.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BikeMainHardware.h>

class BikeRFIDManager {
public:
    BikeRFIDManager();
    ~BikeRFIDManager();
    
    void begin();
    void update();
    
    // RFID Management
    bool isCardPresent();
    bool authenticateCard();
    void addAuthorizedCard(String uid);
    void removeAuthorizedCard(String uid);
    bool isCardAuthorized(String uid);
    void clearAllCards();
    void setMasterCard(String uid);  // Set master card from main
    
    // Bike Control
    bool isBikeUnlocked();
    void toggleBikeLock();
    void lockBike();
    void unlockBike();
    
private:
    MFRC522 mfrc522;
    Preferences preferences;
    
    bool bikeUnlocked;
    String lastCardUID;
    unsigned long lastCardTime;
    String masterCardUID;  // Master card set from main
    
    void saveBikeState();
    void loadBikeState();
    void processCard(String uid);
    String getCardUID();
};

#endif
