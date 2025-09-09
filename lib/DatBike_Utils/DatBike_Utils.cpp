#include "DatBike_Utils.h"
#include <stdio.h>
#include <stdlib.h>

std::string numberToString(int32_t n) {
  char buffer[11];
  itoa(n, buffer, 10);
  return std::string(buffer);
}