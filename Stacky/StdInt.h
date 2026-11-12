#pragma once

#ifdef _MSC_VER
#include <cstdint>
#else
using int8_t   = signed char;
using uint8_t  = unsigned char;
using int16_t  = short;
using uint16_t = unsigned short;
using int32_t  = long;
using uint32_t = unsigned long;
#endif
