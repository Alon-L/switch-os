#ifndef _INCLUDE_IO
#define _INCLUDE_IO

#include <stdint.h>

#define BUILD_IO_IN(name, type, bwl)                                      \
  static inline __attribute__((always_inline)) type name(uint16_t port) { \
    type value;                                                           \
    asm volatile("in" bwl " %1, %0" : "=a"(value) : "Nd"(port));          \
    return value;                                                         \
  }

#define BUILD_IO_OUT(name, type, bwl)                                                 \
  static inline __attribute__((always_inline)) void name(uint16_t port, type value) { \
    asm volatile("out" bwl " %1, %0" : : "Nd"(port), "a"(value));                     \
  }

#define BUILD_MMIO_READ(name, type, bwl, barrier)                                        \
  static inline __attribute__((always_inline)) type name(volatile void* addr) {          \
    type value;                                                                          \
    asm volatile("mov" bwl " %1, %0" : "=r"(value) : "m"(*(volatile type*)addr)barrier); \
    return value;                                                                        \
  }

#define BUILD_MMIO_WRITE(name, type, bwl, barrier)                                          \
  static inline __attribute__((always_inline)) void name(volatile void* addr, type value) { \
    asm volatile("mov" bwl " %1, %0" : : "m"(*(volatile type*)addr), "r"(value)barrier);    \
  }

BUILD_IO_IN(in8, uint8_t, "b");
BUILD_IO_IN(in16, uint16_t, "w");
BUILD_IO_IN(in32, uint32_t, "l");

BUILD_IO_OUT(out8, uint8_t, "b");
BUILD_IO_OUT(out16, uint16_t, "w");
BUILD_IO_OUT(out32, uint32_t, "l");

BUILD_MMIO_READ(read8, uint8_t, "b", );
BUILD_MMIO_READ(read16, uint16_t, "w", );
BUILD_MMIO_READ(read32, uint32_t, "l", );

BUILD_MMIO_READ(read_mb8, uint8_t, "b", : "memory");
BUILD_MMIO_READ(read_mb16, uint16_t, "w", : "memory");
BUILD_MMIO_READ(read_mb32, uint32_t, "l", : "memory");

BUILD_MMIO_WRITE(write8, uint8_t, "b", );
BUILD_MMIO_WRITE(write16, uint16_t, "w", );
BUILD_MMIO_WRITE(write32, uint32_t, "l", );

BUILD_MMIO_WRITE(write_mb8, uint8_t, "b", : "memory");
BUILD_MMIO_WRITE(write_mb16, uint16_t, "w", : "memory");
BUILD_MMIO_WRITE(write_mb32, uint32_t, "l", : "memory");

#undef BUILD_IO_IN
#undef BUILD_IO_OUT
#undef BUILD_MMIO_READ
#undef BUILD_MMIO_WRITE

#define mb() asm volatile("mfence" : : : "memory")

#endif
