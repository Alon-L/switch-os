#ifndef _INCLUDE_IO
#define _INCLUDE_IO

#include <stdint.h>

#define BUILDIO(num, type)                                 \
  static inline void out##num(uint16_t port, type value) { \
    asm volatile("out %0, %1" : : "Nd"(port), "a"(value)); \
  }                                                        \
                                                           \
  static inline type in##num(uint16_t port) {              \
    type value;                                            \
    asm volatile("in %0, %1" : "=a"(value) : "Nd"(port));  \
    return value;                                          \
  }

BUILDIO(8, uint8_t)
BUILDIO(16, uint16_t)
BUILDIO(32, uint32_t)
#undef BUILDIO

#endif
