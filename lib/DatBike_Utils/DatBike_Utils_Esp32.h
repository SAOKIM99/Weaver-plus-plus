#ifndef DATBIKE_UTILS_ESP32_H_
#define DATBIKE_UTILS_ESP32_H_

#include <Preferences.h>

void getStringPref(Preferences& p, 
  const char* key, char* data, size_t maxLen, const char* defaultVal);

#endif