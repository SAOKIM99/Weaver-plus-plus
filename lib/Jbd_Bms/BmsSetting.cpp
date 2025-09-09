#include "BmsSetting.h"

std::string BmsSetting::SettingMap[SETTING_COUNT] = {};
std::string BmsSetting::LongSettingMap[LONG_SETTING_COUNT] = {};
BmsSetting BmsSetting::GoodSetting;
bool BmsSetting::isInitialized = false;

void BmsSetting::initialize() {
  if (!isInitialized) {
    Serial.println("BmsSetting initialized");
    initializeMap();
    initializeGoodSetting();
    isInitialized = true;
  }
}
  
void BmsSetting::initializeMap() {  
  SettingMap[0x10 - SETTING_START] = "FullCapacity";
  SettingMap[0x11 - SETTING_START] = "CycleCapacity";
  SettingMap[0x12 - SETTING_START] = "CellFullVoltage";
  SettingMap[0x13 - SETTING_START] = "CellEmptyVoltage";
  SettingMap[0x14 - SETTING_START] = "RateDischarge";
  SettingMap[0x15 - SETTING_START] = "ProdDate";
  SettingMap[0x16 - SETTING_START] = "Unknown0";
  SettingMap[0x17 - SETTING_START] = "CycleCount";
  SettingMap[0x18 - SETTING_START] = "ChargeOTPTrigger";
  SettingMap[0x19 - SETTING_START] = "ChargeOTPRelease";
  SettingMap[0x1a - SETTING_START] = "ChargeUTPTrigger";
  SettingMap[0x1b - SETTING_START] = "ChargeUTPRelease";
  SettingMap[0x1c - SETTING_START] = "DischargeOTPTrigger";
  SettingMap[0x1d - SETTING_START] = "DischargeOTPRelease";
  SettingMap[0x1e - SETTING_START] = "DischargeUTPTrigger";
  SettingMap[0x1f - SETTING_START] = "DischargeUTPRelease";
  SettingMap[0x20 - SETTING_START] = "PackOVPTrigger";
  SettingMap[0x21 - SETTING_START] = "PackOVPRelease";
  SettingMap[0x22 - SETTING_START] = "PackUVPTrigger";
  SettingMap[0x23 - SETTING_START] = "PackUVPRelease";
  SettingMap[0x24 - SETTING_START] = "CellOVPTrigger";
  SettingMap[0x25 - SETTING_START] = "CellOVPRelease";
  SettingMap[0x26 - SETTING_START] = "CellUVPTrigger";
  SettingMap[0x27 - SETTING_START] = "CellUVPRelease";
  SettingMap[0x28 - SETTING_START] = "ChargeOCP";
  SettingMap[0x29 - SETTING_START] = "DischargeOCP";
  SettingMap[0x2a - SETTING_START] = "BalanceStartVoltage";
  SettingMap[0x2b - SETTING_START] = "BalanceVoltageDelta";
  SettingMap[0x2c - SETTING_START] = "Unknown1";
  SettingMap[0x2d - SETTING_START] = "BalanceEnable";
  SettingMap[0x2e - SETTING_START] = "NTCSensorEnable";
  SettingMap[0x2f - SETTING_START] = "CellCount";
  SettingMap[0x30 - SETTING_START] = "Unknown2";
  SettingMap[0x31 - SETTING_START] = "Unknown3";
  SettingMap[0x32 - SETTING_START] = "Capacity80Percent";
  SettingMap[0x33 - SETTING_START] = "Capacity60Percent";
  SettingMap[0x34 - SETTING_START] = "Capacity40Percent";
  SettingMap[0x35 - SETTING_START] = "Capacity20Percent";
  SettingMap[0x36 - SETTING_START] = "HardwareCellOVP";
  SettingMap[0x37 - SETTING_START] = "HardwareCellUVP";
  SettingMap[0x38 - SETTING_START] = "Unknown4";
  SettingMap[0x39 - SETTING_START] = "Unknown5";
  SettingMap[0x3a - SETTING_START] = "ChargeUTPOTPDelay";
  SettingMap[0x3b - SETTING_START] = "DischargeUTPOTPDelay";
  SettingMap[0x3c - SETTING_START] = "PackUVPOVPDelay";
  SettingMap[0x3d - SETTING_START] = "CellUVOOVPDelay";
  SettingMap[0x3e - SETTING_START] = "ChargeOCPDelayRelease";
  SettingMap[0x3f - SETTING_START] = "DischargeOCPDelayRelease";

  LongSettingMap[0xa0 - LONG_SETTING_START] = "SerialNumber";
  LongSettingMap[0xa1 - LONG_SETTING_START] = "Model";
  LongSettingMap[0xa2 - LONG_SETTING_START] = "Barcode";
}

void BmsSetting::initializeGoodSetting() {
  uint16_t* settings = GoodSetting.getSettings();
  settings[0x10 - SETTING_START] = 0;     // "FullCapacity";
  settings[0x11 - SETTING_START] = 0;     // "CycleCapacity";
  settings[0x12 - SETTING_START] = 0;     // "CellFullVoltage";
  settings[0x13 - SETTING_START] = 0;     // "CellEmptyVoltage";
  settings[0x14 - SETTING_START] = 0;     // "RateDischarge";
  settings[0x15 - SETTING_START] = 0;     // "ProdDate";
  settings[0x16 - SETTING_START] = 0;     // "Unknown0";
  settings[0x17 - SETTING_START] = 0;     // "CycleCount";
  settings[0x18 - SETTING_START] = 3381;  // "ChargeOTPTrigger";
  settings[0x19 - SETTING_START] = 3281;  // "ChargeOTPRelease";
  settings[0x1a - SETTING_START] = 2581;  // "ChargeUTPTrigger";
  settings[0x1b - SETTING_START] = 2631;  // "ChargeUTPRelease";
  settings[0x1c - SETTING_START] = 3431;  // "DischargeOTPTrigger";
  settings[0x1d - SETTING_START] = 3331;  // "DischargeOTPRelease";
  settings[0x1e - SETTING_START] = 2581;  // "DischargeUTPTrigger";
  settings[0x1f - SETTING_START] = 2631;  // "DischargeUTPRelease";
  settings[0x20 - SETTING_START] = 8350;  // "PackOVPTrigger";
  settings[0x21 - SETTING_START] = 8200;  // "PackOVPRelease";
  settings[0x22 - SETTING_START] = 5950;  // "PackUVPTrigger";
  settings[0x23 - SETTING_START] = 6000;  // "PackUVPRelease";
  settings[0x24 - SETTING_START] = 4180;  // "CellOVPTrigger";
  settings[0x25 - SETTING_START] = 4120;  // "CellOVPRelease";
  settings[0x26 - SETTING_START] = 2950;  // "CellUVPTrigger";
  settings[0x27 - SETTING_START] = 3050;  // "CellUVPRelease";
  settings[0x28 - SETTING_START] = 5000;  // "ChargeOCP";
  settings[0x29 - SETTING_START] = 58036; // "DischargeOCP";
  settings[0x2a - SETTING_START] = 3800;  // "BalanceStartVoltage";
  settings[0x2b - SETTING_START] = 15;    // "BalanceVoltageDelta";
  settings[0x2c - SETTING_START] = 0;     // "Unknown1";
  settings[0x2d - SETTING_START] = 6;     // "BalanceEnable";
  settings[0x2e - SETTING_START] = 15;    // "NTCSensorEnable";
  settings[0x2f - SETTING_START] = 20;    // "CellCount";
  settings[0x30 - SETTING_START] = 0;     // "Unknown2";
  settings[0x31 - SETTING_START] = 0;     // "Unknown3";
  settings[0x32 - SETTING_START] = 0;     // "Capacity80Percent";
  settings[0x33 - SETTING_START] = 0;     // "Capacity60Percent";
  settings[0x34 - SETTING_START] = 0;     // "Capacity40Percent";
  settings[0x35 - SETTING_START] = 0;     // "Capacity20Percent";
  settings[0x36 - SETTING_START] = 4250;  // "HardwareCellOVP";
  settings[0x37 - SETTING_START] = 2700;  // "HardwareCellUVP";
  settings[0x38 - SETTING_START] = 0;     // "Unknown4";
  settings[0x39 - SETTING_START] = 0;     // "Unknown5";
  settings[0x3a - SETTING_START] = 1285;  // "ChargeUTPOTPDelay";
  settings[0x3b - SETTING_START] = 1285;  // "DischargeUTPOTPDelay";
  settings[0x3c - SETTING_START] = 1285;  // "PackUVPOVPDelay";
  settings[0x3d - SETTING_START] = 259;   // "CellUVOOVPDelay";
  settings[0x3e - SETTING_START] = 1312;  // "ChargeOCPDelayRelease";
  settings[0x3f - SETTING_START] = 2080;  // "DischargeOCPDelayRelease";
}

uint8_t BmsSetting::checkAgainstGoodSetting() {
  uint16_t* good = GoodSetting.getSettings();

  for (uint8_t i = 0; i < SETTING_COUNT; i++) {
    if (good[i] != 0 && good[i] != settings[i]) {
      Serial.printf("Setting unmatched: %s. Good: %u Ours: %u\n", 
        SettingMap[i].c_str(), good[i], settings[i]);
      return i;
    }
  }

  return 0x00;
}

uint16_t* BmsSetting::getSettings() {
  return settings;
}

std::string* BmsSetting::getLongSettings() {
  return longSettings;
}

