#ifndef _INCLUDE_IO
#define _INCLUDE_IO

#include <stdint.h>

#define BUILDIO(suffix, type)                                         \
  static inline void out##suffix(type value, uint16_t port) {         \
    asm volatile("out %0, %1" : : "Nd"(port), "a"(value) : "memory"); \
  }                                                                   \
                                                                      \
  static inline type in##suffix(uint16_t port) {                      \
    type value;                                                       \
    asm volatile("in %0, %1" : "=a"(value) : "Nd"(port) : "memory");  \
    return value;                                                     \
  }

BUILDIO(b, uint8_t)
BUILDIO(w, uint16_t)
BUILDIO(l, uint32_t)
#undef BUILDIO

#endif
