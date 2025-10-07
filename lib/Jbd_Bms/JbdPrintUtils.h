#ifndef JBD_PRINT_UTILS
#define JBD_PRINT_UTILS
#endif

#include "JbdBms.h"
#include <Arduino.h>

namespace JbdPrintUtils {
  void printPackInfo(PackInfo* pack);
  void printPackCellInfo(PackCellInfo* packcell);
  void printProtectionCount(ProtectionCount* protectionCount);
  void printSettings(BmsSetting* setting);
  void printfln(const char* prefix, float f, const char* suffix);
  void printdln(const char* prefix, unsigned int f, const char* suffix);
  void printxln(const char* prefix, bool f, const char* suffix);
}