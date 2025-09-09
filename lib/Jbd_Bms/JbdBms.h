#ifndef JBD_BMS_H_
#define JBD_BMS_H_

#include <Stream.h>
#include "BmsSetting.h"

#define BMS_LEN_RESPONSE 40
#define TIMEOUT          200 // ms
#define FLUSH_TIMEOUT    75 // ms
#define MAX_SERIES_CELLS 20
#define MAX_TRIES        3

struct PackInfo {
  float voltage;
  float current;
  float remainingCapacity;
  float nominalCapacity;
  uint16_t cycleCount;
  uint16_t manufactureDate;
  uint32_t balancingState;
  uint16_t protectionState;
  uint8_t softwareVersion;
  uint8_t percent;
  uint8_t mosfetStatus;
  uint8_t seriesCount;
  uint8_t ntcCount;
  float ntc[4];

  bool isChargingOpen();
  bool isDischargingOpen();
  bool isOffBySoftware();
  bool isEmpty();
};

struct PackCellInfo {
  uint8_t numCell;
  uint16_t cellVol[20];
  uint16_t minVol;
  uint16_t maxVol;
  uint16_t diffVol;
  float avgVol;

  bool isEmpty();
};

struct ProtectionCount {
  uint16_t shortCircuit;
  uint16_t chargeOCP;
  uint16_t dischargeOCP;
  uint16_t chargeOVP;
  uint16_t chargeUVP;
  uint16_t chargeOTP;
  uint16_t chargeUTP;
  uint16_t dischargeOTP;
  uint16_t dischargeUTP;
  uint16_t packOVP;
  uint16_t packUVP;

  uint32_t totalProtection();
  bool isEmpty();
};

enum MosfetState {M_OFF, M_ON};

class JbdBms {
public:
  static PackInfo EMPTY_PACK_INFO;
  static PackCellInfo EMPTY_PACK_CELL_INFO;
  static ProtectionCount EMPTY_PROTECTION_COUNT;

  JbdBms(Stream& bms);

  PackInfo* getPackInfo(bool refresh);
  PackCellInfo* getPackCellInfo(bool refresh);
  ProtectionCount* getProtectionCount(bool refresh);
  
  // 0x00: good 
  // 0xFF: failed to read settings
  // Otherwise: hex code of the unmatched setting
  uint8_t checkSettings();
  
  bool isChargingOpen();
  bool isDischargingOpen();
  bool isBothOpen();

  void lock();
  void unlock();
  void setMosfetState(MosfetState discharge, MosfetState charge);

private:
  Stream& bms;
  uint8_t response[BMS_LEN_RESPONSE];

  PackInfo packInfo;
  PackCellInfo packCellInfo;
  ProtectionCount protectionCount;

  void getPackInfo();
  void getPackCellInfo();
  void getProtectionCount();

  int8_t readSetting(uint8_t* request, size_t len, uint8_t tries = 0);

  int8_t readResponse();
  bool readLen(uint8_t* res, uint8_t len);
  bool checkCheckSum(uint8_t* res, uint8_t len);
  void flush();

  void sendRequest(uint8_t* requestMessage, uint8_t len);
  void startUpdate();
  void endUpdate();

  void parsePackInfo();
  void parseCellInfo();

  uint16_t combine(uint8_t highbyte, uint8_t lowbyte);
};

#endif /* JBD_BMS_H_ */