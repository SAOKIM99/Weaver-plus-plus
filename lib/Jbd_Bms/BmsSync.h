#ifndef BMS_SYNC
#define BMS_SYNC


#include "JbdBms.h"

#define MAX_PROTECTION 1000

class BmsSync {
  public:
    BmsSync(int rxd1, int txd1, int rxd2, int txd2);
    
    void begin();
    void sync();
    void printInfo();
    void getPack(uint8_t numberpack);
    void getPackcell(uint8_t numberpack);
    void getProtectionCount(uint8_t numberpack);
    PackInfo* packInfo;
    PackCellInfo* packCell;
    ProtectionCount* protectionCount;
  private:
    int rx1, tx1, rx2, tx2;
    JbdBms bms1, bms2;

    void control(JbdBms& bmsOn, JbdBms& bmsOff);
    void checkSetting(JbdBms& bms);
};
#endif
