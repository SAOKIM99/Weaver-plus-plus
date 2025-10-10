#ifndef BIKE_CAN_MANAGER_H
#define BIKE_CAN_MANAGER_H

#include <Arduino.h>
#include <CAN.h>
#include "BikeData.h"
#include "BikeMainHardware.h"

// CAN Configuration
#define CAN_SPEED     500E3  // 500 kbps

// CAN Message IDs
#define MSG_ID_BIKE_STATUS    0x100  // Speed, gear, signals
#define MSG_ID_BMS_DATA       0x200  // +1 for BMS1, +2 for BMS2
#define MSG_ID_VESC_DATA      0x300  // Motor data
#define MSG_ID_BATTERY_EXT    0x400  // Extended battery data
#define MSG_ID_DISTANCE_DATA  0x500  // Distance & trip data
#define MSG_ID_TIME_DATA      0x600  // Time data
#define MSG_ID_DISPLAY_CMD    0x800  // Commands from display

// CAN Message Types
enum CANMessageType {
    CAN_MSG_BIKE_STATUS = 0,    // Speed, turn signals
    CAN_MSG_BMS1_DATA = 1,      // BMS1 basic data
    CAN_MSG_BMS2_DATA = 2,      // BMS2 basic data
    CAN_MSG_VESC_DATA = 3,      // Motor data
    CAN_MSG_BATTERY_EXT = 4,    // Extended battery info
    CAN_MSG_DISTANCE_DATA = 5,  // Distance & odometer
    CAN_MSG_TIME_DATA = 6,      // Time data
    CAN_MSG_COUNT = 7
};

// CAN receive callback function type
typedef void (*CANReceiveCallback)(uint32_t id, uint8_t* data, uint8_t length);

class BikeCANManager {
public:
    BikeCANManager();
    ~BikeCANManager();
    
    // Initialization
    bool begin(int txPin, int rxPin); // Override pins for display board
    void update();
    
    // Data transmission
    bool sendBikeStatus(const BikeStatus& status, bool bikeUnlocked, bool bleConnected);
    bool sendBMSData(const BMSData& bms, uint8_t bmsId);
    bool sendVESCData(const VESCData& vesc);
    bool sendBatteryExtended(const BMSData& bms1, const BMSData& bms2);
    bool sendDistanceData(float odometer, float distance, float tripDistance);
    bool sendTimeData(int time);
    
    // Data reception
    void setReceiveCallback(CANReceiveCallback callback);
    
    // Data parsing functions
    bool parseBikeStatus(uint8_t* data, uint8_t length, BikeStatus& status, bool& bikeUnlocked, bool& bleConnected);
    bool parseBMSData(uint8_t* data, uint8_t length, BMSData& bms);
    bool parseVESCData(uint8_t* data, uint8_t length, VESCData& vesc);
    bool parseBatteryExtended(uint8_t* data, uint8_t length, BMSData& bms1, BMSData& bms2, int& motorPower);
    bool parseDistanceData(uint8_t* data, uint8_t length, float& odometer, float& distance, float& tripDistance);
    bool parseTimeData(uint8_t* data, uint8_t length, int& time);
    
    // Convenience function to parse any message
    bool parseCANMessage(uint32_t id, uint8_t* data, uint8_t length, BikeDataDisplay& displayData);
    
    // Status
    bool isInitialized();
    uint32_t getMessagesSent();
    uint32_t getMessagesReceived();
    
    // Sequence sending (for RTOS task)
    void sendNextInSequence(const SharedBikeData& sharedData);
    
private:
    bool initialized;
    uint32_t messagesSent;
    uint32_t messagesReceived;
    uint8_t sendSequence;
    CANReceiveCallback receiveCallback;
    

};

#endif