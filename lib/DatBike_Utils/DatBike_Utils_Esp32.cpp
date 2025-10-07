#include "DatBike_Utils_Esp32.h"

void getStringPref(Preferences& pref, 
  const char* key, char* data, size_t maxLen, const char* defaultVal) {

  size_t len;
  if (!pref.isKey(key)) {
    len = strlen(defaultVal);
    memcpy(data, defaultVal, len);
  } else {
    len = pref.getBytes(key, data, maxLen);
  }

  data[len] = '\0';
}