#include "JbdBms.h"
#include "JbdPrintUtils.h"

#include <SoftwareSerial.h>

SoftwareSerial bmsStream(35, 32);
JbdBms bms(bmsStream);

void setup() {
  Serial.begin(115200);
  bmsStream.begin(9600);

  delay(1000);
}

void loop() {
  PackInfo* pack = bms.getPackInfo(true);
  JbdPrintUtils::printPackInfo(pack);
  delay(1000);
}
