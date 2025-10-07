#ifndef DATBIKE_UTILS_H_
#define DATBIKE_UTILS_H_

#include <string>

#define UTILS_LOW_PASS_FILTER(value, sample, constant)	(value -= (constant) * ((value) - (sample)))
#define UTILS_LOW_PASS_FILTER_2(value, sample, lconstant, hconstant)\
  if (sample < value) {\
    UTILS_LOW_PASS_FILTER(value, sample, lconstant);\
  }\
  else {\
    UTILS_LOW_PASS_FILTER(value, sample, hconstant);\
  }

std::string numberToString(int32_t n);

#endif