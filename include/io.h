#ifndef _INCLUDE_IO
#define _INCLUDE_IO

#include <stdint.h>

#define BUILDIO(num, type)                                            \
  static inline void out##num(type value, uint16_t port) {            \
    asm volatile("out %0, %1" : : "Nd"(port), "a"(value) : "memory"); \
  }                                                                   \
                                                                      \
  static inline type in##num(uint16_t port) {                         \
    type value;                                                       \
    asm volatile("in %0, %1" : "=a"(value) : "Nd"(port) : "memory");  \
    return value;                                                     \
  }

BUILDIO(8, uint8_t)
BUILDIO(16, uint16_t)
BUILDIO(32, uint32_t)
#undef BUILDIO

#endif
