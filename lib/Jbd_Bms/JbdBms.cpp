#include <Arduino.h>

#include "JbdBms.h"
#include "JbdPrintUtils.h"

#define MAX_VOL 0
#define MIN_VOL 10000
#define MAX_CELL 20

PackInfo JbdBms::EMPTY_PACK_INFO = {};
PackCellInfo JbdBms::EMPTY_PACK_CELL_INFO = {};
ProtectionCount JbdBms::EMPTY_PROTECTION_COUNT = {
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

JbdBms::JbdBms(Stream& bmsStream): bms(bmsStream) {
}

PackInfo* JbdBms::getPackInfo(bool refresh) {
  if (refresh) {
    getPackInfo();
  }
  return &packInfo;
}

PackCellInfo* JbdBms::getPackCellInfo(bool refresh) {
  if (refresh) {
    getPackCellInfo();
  }
  return &packCellInfo;
}

ProtectionCount* JbdBms::getProtectionCount(bool refresh) {
  if (refresh) {
    getProtectionCount();
  }
  return &protectionCount;
}

bool JbdBms::isChargingOpen() {
  return packInfo.isChargingOpen();
}

bool JbdBms::isDischargingOpen() {
  return packInfo.isDischargingOpen();
}

bool JbdBms::isBothOpen() {
  return isChargingOpen() && isDischargingOpen();
}

bool PackInfo::isChargingOpen() {
  return mosfetStatus & 1;
}

bool PackInfo::isDischargingOpen() {
  return (mosfetStatus >> 1) & 1;
}

bool PackInfo::isOffBySoftware() {
  return (protectionState >> 12) & 1;
}

bool PackInfo::isEmpty() {
  return (voltage == 0) && (ntcCount == 0) && (manufactureDate == 0) && (softwareVersion == 0);
}

void JbdBms::unlock() {
  setMosfetState(M_ON, M_ON);
}

void JbdBms::lock() {
  setMosfetState(M_OFF, M_OFF);
}

void JbdBms::setMosfetState(MosfetState discharge, MosfetState charge) {
  uint8_t state = discharge << 1 | charge;
  uint8_t byte5 = 0x03 - state;
  uint8_t byte7 = 0x1D - byte5;
  uint8_t request[] = {0xDD, 0x5A,  0xE1, 0x02, 0x00,  byte5,  0xFF, byte7,  0x77};
      
  startUpdate();
  sendRequest(request, 9);
  endUpdate();    
}

void JbdBms::startUpdate() {
  uint8_t request[] = {0xDD, 0x5A, 0x00, 0x02, 0x56, 0x78, 0xFF, 0x30, 0x77};
  sendRequest(request, sizeof(request));
}

void JbdBms::endUpdate() {
  uint8_t request[] = {0xDD, 0x5A, 0x01, 0x02, 0x00, 0x00, 0xFF, 0xFD, 0x77};
  sendRequest(request, sizeof(request));
}

void JbdBms::getPackInfo() {
  uint8_t request[] = {0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77};
  sendRequest(request, sizeof(request));
  
  if (readResponse() == -1) {
    // error while reading
    packInfo = EMPTY_PACK_INFO;
    return;
  }

  parsePackInfo();
}

void JbdBms::getPackCellInfo() {
  uint8_t request[] = { 0xDD, 0xA5, 0x04, 0x00, 0xFF, 0xFC, 0x77 };
  sendRequest(request, sizeof(request));

  if (readResponse() == -1) {
    // error while reading
    packCellInfo = EMPTY_PACK_CELL_INFO;
    return; 
  }

  parseCellInfo();
}

int8_t JbdBms::readSetting(uint8_t* request, size_t len, uint8_t tries) {
  sendRequest(request, len);
  int8_t res = readResponse();
  if (res != -1) {
    return res;
  } else if (tries < MAX_TRIES) {
    delay(200);
    return readSetting(request, len, tries + 1);
  }

  return -1;
}

uint8_t JbdBms::checkSettings() {
  Serial.println("Checking settings...");
  BmsSetting settings;

  // make it more reliable
  startUpdate();
  endUpdate();

  startUpdate();
  uint8_t request[] = { 0xDD, 0xA5, 0x17, 0x00, 0xFF, 0xE9, 0x77 };

  for (uint8_t i = 0; i < SETTING_COUNT; i++) {  
    request[2] = i + SETTING_START;
    request[5] = 0x100 - request[2];
    
    if (readSetting(request, sizeof(request)) == -1) {
      Serial.printf("Failed to read setting for %04x\n", i + SETTING_START);
      endUpdate();
      return 0xFF;
    }
    settings.getSettings()[i] = combine(response[0], response[1]);
  }

  for (uint8_t i = 0; i < LONG_SETTING_COUNT; i++) {  
    request[2] = i + LONG_SETTING_START;
    request[5] = 0x100 - request[2];
    
    int8_t res = readSetting(request, sizeof(request));
    if (res == -1) {
      Serial.printf("Failed to read setting for %04x\n", i + LONG_SETTING_START);
      endUpdate();
      return 0xFF;
    }
    settings.getLongSettings()[i].append((char*) response, res);
  }

  endUpdate();
  JbdPrintUtils::printSettings(&settings);
  return settings.checkAgainstGoodSetting();
}

void JbdBms::getProtectionCount() {
  Serial.println("Read protection count...");

  // make it more reliable
  startUpdate();
  endUpdate();

  startUpdate();
  uint8_t request[] = { 0xDD, 0xA5, 0xAA, 0x00, 0xFF, 0x100 - 0xAA, 0x77 };
  sendRequest(request, sizeof(request));

  if (readResponse() == -1) {
    protectionCount = EMPTY_PROTECTION_COUNT;  
  } else {
    protectionCount.shortCircuit  = combine(response[0],  response[1]);
    protectionCount.chargeOCP     = combine(response[2],  response[3]);
    protectionCount.dischargeOCP  = combine(response[4],  response[5]);
    protectionCount.chargeOVP     = combine(response[6],  response[7]);
    protectionCount.chargeUVP     = combine(response[8],  response[9]);
    protectionCount.chargeOTP     = combine(response[10], response[11]);
    protectionCount.chargeUTP     = combine(response[12], response[13]);
    protectionCount.dischargeOTP  = combine(response[14], response[15]);
    protectionCount.dischargeUTP  = combine(response[16], response[17]);
    protectionCount.packOVP       = combine(response[18], response[19]);
    protectionCount.packUVP       = combine(response[20], response[21]);
  }

  JbdPrintUtils::printProtectionCount(&protectionCount);
  endUpdate();
}

bool PackCellInfo::isEmpty() {
  return (minVol == 0) && (maxVol == 0) && (avgVol == 0);
}

bool ProtectionCount::isEmpty() {
  return totalProtection() == JbdBms::EMPTY_PROTECTION_COUNT.totalProtection();
}

void JbdBms::sendRequest(uint8_t* requestMessage, uint8_t len) {
  flush();
  bms.write(requestMessage, len);
}

int8_t JbdBms::readResponse() {  
  uint8_t header[4];
  if (!readLen(header, 4)) {
    return -1;
  }

  uint8_t dataLen = header[3];

  // include 2 checksum bytes
  if (!readLen(response, dataLen + 2)) {
    return -1;
  }

  bool checkSum = checkCheckSum(response, dataLen);
  return checkSum ? dataLen : -1;
}

bool JbdBms::checkCheckSum(uint8_t* res, uint8_t len) {
  uint16_t received = (res[len] << 8) | res[len + 1];

  //guard against weird edge cases
  if (received == 0) {
    return false;
  }

  uint16_t sum = len;
  for (int i = 0; i < len; i++) {
    sum += res[i];
  }

  sum = (sum - 1) ^ 0xFFFF;
  return sum == received;
}

void JbdBms::flush() { 
  uint32_t last = millis();
  while (millis() - last < FLUSH_TIMEOUT) {
    if (bms.available() > 0) {
      bms.read();
    } else {
      delay(5);
    }
  }
}

bool JbdBms::readLen(uint8_t * res, uint8_t len) {
  uint32_t startTime = millis();
  for (uint8_t i = 0; i < len; i++) {
    // wait till data is available
    while ((millis() - startTime < TIMEOUT) && (bms.available() <= 0)) {
      delay(5);
    }

    if (bms.available() <= 0) {
      return false;
    }
    res[i] = bms.read();
  }
  return true;
}

void JbdBms::parsePackInfo() {
  packInfo.voltage = combine(response[0], response[1])/100.0f;
  packInfo.current = ((int16_t)combine(response[2], response[3]))/100.0f;
  packInfo.remainingCapacity = combine(response[4], response[5])/100.0f;
  packInfo.nominalCapacity = combine(response[6], response[7])/100.0f;
  packInfo.cycleCount = combine(response[8], response[9]);
  packInfo.manufactureDate = combine(response[10], response[11]);
  packInfo.balancingState = ((uint32_t) combine(response[12], response[13])) << 16 
                        | combine(response[14], response[15]);
  packInfo.protectionState = combine(response[16], response[17]);
  packInfo.softwareVersion = response[18];
  packInfo.percent = response[19];
  packInfo.mosfetStatus = response[20];
  packInfo.seriesCount = response[21];
  packInfo.ntcCount = response[22];
  for (int i = 0; i < packInfo.ntcCount; i++) {
    packInfo.ntc[i] = (combine(response[23+i*2], response[24+i*2]) - 2731)/10.0f;
  }
}

void JbdBms::parseCellInfo() {
  float sumVol = 0;

  packCellInfo.maxVol = MAX_VOL;
  packCellInfo.minVol = MIN_VOL;

  packCellInfo.numCell = MAX_CELL;

  for (uint8_t i = 0; i < packCellInfo.numCell; i++) {
    packCellInfo.cellVol[i] = combine(response[i * 2], response[i * 2 + 1]);
    sumVol += packCellInfo.cellVol[i];

    if (packCellInfo.maxVol < packCellInfo.cellVol[i]) {
      packCellInfo.maxVol = packCellInfo.cellVol[i];
    }
    if (packCellInfo.minVol > packCellInfo.cellVol[i]) {
      packCellInfo.minVol = packCellInfo.cellVol[i];
    }
  }

  packCellInfo.avgVol = sumVol / packCellInfo.numCell;
  packCellInfo.diffVol = packCellInfo.maxVol - packCellInfo.minVol;
}

/**
 * Build one uint16_t out of two uint8_t
 */
uint16_t JbdBms::combine(uint8_t highbyte, uint8_t lowbyte)
{
  return (((uint16_t)highbyte) << 8) | lowbyte;
}

uint32_t ProtectionCount::totalProtection() {
  return  ((uint32_t)shortCircuit) + chargeOCP + dischargeOCP
    + chargeOVP + chargeUVP + chargeOTP + chargeUTP
    + dischargeOTP + dischargeUTP + packOVP + packUVP;
}