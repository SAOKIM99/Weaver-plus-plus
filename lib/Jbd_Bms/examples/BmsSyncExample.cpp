#include "BmsSync.h"

#define ESP
#define RX1 35
#define TX1 32
#define RX2 33
#define TX2 25

BmsSync bmsSync(RX1, TX1, RX2, TX2);

void setup() {
  Serial.begin(115200);
  bmsSync.begin();
  delay(1000);
}

void loop() {
  bmsSync.sync();
  bmsSync.printInfo();
  delay(100);
}
