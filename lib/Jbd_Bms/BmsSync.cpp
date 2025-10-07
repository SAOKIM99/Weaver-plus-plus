#include <string>

#include "BmsSync.h"
#include "JbdPrintUtils.h"

// CANBus canbusTx(GPIO_NUM_25, GPIO_NUM_26, 65, 1);

BmsSync::BmsSync(int rxd1, int txd1, int rxd2, int txd2):
  rx1(rxd1),
  tx1(txd1),
  rx2(rxd2),
  tx2(txd2),
  bms1(Serial1),
  bms2(Serial2) {
}

void BmsSync::begin() {
  Serial1.begin(9600, SERIAL_8N1, rx1, tx1);
  Serial2.begin(9600, SERIAL_8N1, rx2, tx2);

  BmsSetting::initialize();
  checkSetting(bms1);
  checkSetting(bms2);

  //canbusTx.begin();
}

void BmsSync::checkSetting(JbdBms& bms) {
  uint8_t res = bms.checkSettings();
  if (res == 0xFF) {
    Serial.println("BMS Error: Read setting failed");
  } else if (res > 0x00) {
    Serial.printf("BMS Error: Bad setting %s\n", BmsSetting::SettingMap[res].c_str());
  }

  ProtectionCount* protectionCount = bms.getProtectionCount(true);
  if (protectionCount->isEmpty()) {
    Serial.println("BMS Error: Read protection failed");
  } else {
    uint32_t total = protectionCount->totalProtection();
    Serial.printf("Protection count: %u\n", total);

    if (total > MAX_PROTECTION) {
      Serial.println("BMS Error: Too many protections");
    }
  }
}

void BmsSync::sync() {
  PackInfo* pack1 = bms1.getPackInfo(true);
  PackInfo* pack2 = bms2.getPackInfo(true);

  // Comment out to hot fix for EECS-33. Will properly fix later
  // PackCellInfo* cell1 = bms1.getPackCellInfo(true);
  // PackCellInfo* cell2 = bms2.getPackCellInfo(true);
  // canbusTx.sendPackInfo(pack1, pack2);
  // canbusTx.sendCellInfo(cell1, cell2);

  // skip sync if reading errors on either pack
  if (pack1->isEmpty()) {
    //Serial.println("Pack1: Reading error, skip sync");
    return;
  }
  
  if (pack2->isEmpty()) {
    //Serial.println("Pack2: Reading error, skip sync");
    return;
  }

  // Sometimes noises can make reading incorrect, or delay
  // can happen between setMosfetState() and the state actually 
  // being set, which in turn causes us to accidentally 
  // disable both BMSs one by one. In that case, we need to turn 
  // them back on. 
  if (pack1->isOffBySoftware() && pack2->isOffBySoftware()) {
    bms1.setMosfetState(M_ON, M_ON);
    bms2.setMosfetState(M_ON, M_ON);
    return;
  }

  // Only sync if one and only one of the BMSs is unlocked.
  // This fails in the case where bms1 is discharge locked and 
  // bms2 is charge locked. In which case, we should software-lock 
  // both BMSs. But this case is unlikely to ever happens, so we ignore
  // it here.
  if (bms1.isBothOpen() && (!bms2.isBothOpen())) {
    Serial.println("Control BMS1 BMS2");
    control(bms1, bms2);
  } else if (bms2.isBothOpen() && (!bms1.isBothOpen())) {
    Serial.println("Control BMS2 BMS1");
    control(bms2, bms1);
  }
}

void BmsSync::control(JbdBms& bmsOn, JbdBms& bmsOff) {
  Serial.print("BMS_ON: ");
  Serial.print(bmsOn.getPackInfo(false)->isChargingOpen());
  Serial.print(bmsOn.getPackInfo(false)->isDischargingOpen());
  Serial.println(bmsOn.getPackInfo(false)->isOffBySoftware());

  Serial.print("BMS_OFF: ");
  Serial.print(bmsOff.getPackInfo(false)->isChargingOpen());
  Serial.print(bmsOff.getPackInfo(false)->isDischargingOpen());
  Serial.println(bmsOff.getPackInfo(false)->isOffBySoftware());

  PackInfo* packOff = bmsOff.getPackInfo(false);

  if (packOff->isOffBySoftware()) {
    bmsOff.setMosfetState(M_ON, M_ON);
    return;
  }

  MosfetState charging = packOff->isChargingOpen() ? M_ON : M_OFF;
  MosfetState discharging = packOff->isDischargingOpen() ? M_ON : M_OFF;
  if (charging == M_ON && discharging == M_ON) {
    return;
  }

  Serial.print("Set Mosfet State: ");
  Serial.print(discharging);
  Serial.println(charging);
  bmsOn.setMosfetState(discharging, charging);
}

void BmsSync::printInfo() {
  PackInfo* pack1 = bms1.getPackInfo(false);
  PackInfo* pack2 = bms2.getPackInfo(false);

  Serial.println("PACK 1:");
  JbdPrintUtils::printPackInfo(pack1);

  Serial.println("PACK 2:");
  JbdPrintUtils::printPackInfo(pack2);
}

void BmsSync::getPack(uint8_t numberpack) {
  if (numberpack==1) packInfo = bms1.getPackInfo(true);
  if (numberpack==2) packInfo = bms2.getPackInfo(true);
}

void BmsSync::getPackcell(uint8_t numberpack) {
  if (numberpack==1) packCell = bms1.getPackCellInfo(true);
  if (numberpack==2) packCell = bms2.getPackCellInfo(true);
}

void BmsSync::getProtectionCount(uint8_t numberpack) {
  if (numberpack==1) protectionCount = bms1.getProtectionCount(true);
  if (numberpack==2) protectionCount = bms2.getProtectionCount(true);
}

