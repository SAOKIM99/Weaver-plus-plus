#include "JbdPrintUtils.h"

namespace JbdPrintUtils {
  void printPackInfo(PackInfo* pack) {
    if (pack->isEmpty()) {
      Serial.println("Pack is EMPTY");
      return;
    }
    Serial.println("--------- printing packinfo start ---------");
    printfln("voltage = ", pack->voltage, "V");
    printfln("current = ", pack->current, "A");
    printfln("remaining capacity = ", pack->remainingCapacity, "Ah");
    printfln("nominal capacity = ", pack->nominalCapacity, "Ah");
    printdln("cycle count = ", pack->cycleCount, "");
    printdln("manufactureDate = ", pack->manufactureDate, "");
    printdln("balancing state = ", pack->balancingState, "");
    printdln("protection state = ", pack->protectionState, "");
    printdln("software version = ", pack->softwareVersion, "");
    printdln("percent = ", pack->percent, "%");
    printxln("isChargingOpen = ", pack->isChargingOpen(), "");
    printxln("isDischargingOpen = ", pack->isDischargingOpen(), "");
    printdln("series count = ", pack->seriesCount, "");

    for (int i = 0; i < pack->ntcCount; i++) {
      printfln((String("ntc") + String(i) + String(" = ")).c_str(), pack->ntc[i], "C");
    }
    
    Serial.println("--------- printing packinfo end ---------");
  }

  void printPackCellInfo(PackCellInfo* packcell) {
    printdln("Number of cell = ", packcell->numCell, "Pcs");
    for(uint8_t i = 0; i < 20; i++) {
      printdln("Voltage Cell = ", packcell->cellVol[i], "mV");
    }

    printfln("Voltage average = ", packcell->avgVol, "mV");
    printdln("Voltage Diff = ", packcell->diffVol, "mV");
    printdln("Voltage max = ", packcell->maxVol, "mV");
    printdln("Voltage min = ", packcell->minVol, "mV");
  }

  void printProtectionCount(ProtectionCount* protectionCount) {
    Serial.printf("shortCircuit %u\n", protectionCount->shortCircuit);
    Serial.printf("chargeOCP %u\n", protectionCount->chargeOCP);
    Serial.printf("dischargeOCP %u\n", protectionCount->dischargeOCP);
    Serial.printf("chargeOVP %u\n", protectionCount->chargeOVP);
    Serial.printf("chargeUVP %u\n", protectionCount->chargeUVP);
    Serial.printf("chargeOTP %u\n", protectionCount->chargeOTP);
    Serial.printf("chargeUTP %u\n", protectionCount->chargeUTP);
    Serial.printf("dischargeOTP %u\n", protectionCount->dischargeOTP);
    Serial.printf("dischargeUTP %u\n", protectionCount->dischargeUTP);
    Serial.printf("packOVP %u\n", protectionCount->packOVP);
    Serial.printf("packUVP %u\n", protectionCount->packUVP);
  }

  void printSettings(BmsSetting* setting) {
    uint16_t* settings = setting->getSettings();
    std::string* longSettings = setting->getLongSettings();

    for (uint8_t i = 0; i < SETTING_COUNT; i++) {
      Serial.printf("%s: %u\n", BmsSetting::SettingMap[i].c_str(), settings[i]);
    }

    for (uint8_t i = 0; i < LONG_SETTING_COUNT; i++) {
      Serial.printf("%s: %s\n", BmsSetting::LongSettingMap[i].c_str(), 
        longSettings[i].c_str());
    }
  }

  void printfln(const char* prefix, float f, const char* suffix) {
    Serial.print(prefix);
    Serial.print(f);
    Serial.print(suffix);
    Serial.println();
  }

  void printdln(const char* prefix, unsigned int f, const char* suffix) {
    Serial.print(prefix);
    Serial.print(f);
    Serial.print(suffix);
    Serial.println();
  }

  void printxln(const char* prefix, bool f, const char* suffix) {
    Serial.print(prefix);
    const char * r = f ? "true" : "false";
    Serial.print(r);
    Serial.print(suffix);
    Serial.println();
  }
}