#include <string>
#include <Arduino.h>

#define SETTING_START 0x10
#define SETTING_COUNT (0x3F - SETTING_START + 1)
#define LONG_SETTING_START 0xA0
#define LONG_SETTING_COUNT (0xA2 - LONG_SETTING_START + 1)

class BmsSetting {
  public:
    static void initialize();
    static std::string SettingMap[SETTING_COUNT];
    static std::string LongSettingMap[LONG_SETTING_COUNT];
    static BmsSetting GoodSetting;

    uint16_t* getSettings();
    std::string* getLongSettings();
    uint8_t checkAgainstGoodSetting();

  private:
    uint16_t settings[SETTING_COUNT];
    std::string longSettings[LONG_SETTING_COUNT];

    static bool isInitialized;

    static void initializeMap();
    static void initializeGoodSetting();
};