#ifndef BIKE_RFID_MANAGER_H
#define BIKE_RFID_MANAGER_H

#include <Preferences.h>
#include <SPI.h>
#include <MFRC522.h>
#include "../Bike_Hardware/BikeHardware.h"

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
    
    void saveBikeState();
    void loadBikeState();
    void processCard(String uid);
    String getCardUID();
};

#endif
