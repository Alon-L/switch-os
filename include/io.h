#ifndef _INCLUDE_IO
#define _INCLUDE_IO

#include <stdint.h>

#define BUILD_IO_IN(name, type)                                           \
  static inline __attribute__((always_inline)) type name(uint16_t port) { \
    type value;                                                           \
    asm volatile("in %0, %1" : "=a"(value) : "Nd"(port));                 \
    return value;                                                         \
  }

#define BUILD_IO_OUT(name, type)                                                      \
  static inline __attribute__((always_inline)) void name(uint16_t port, type value) { \
    asm volatile("out %0, %1" : : "Nd"(port), "a"(value));                            \
  }

#define BUILD_MMIO_READ(name, type, barrier)                                      \
  static inline __attribute__((always_inline)) type name(volatile void* addr) {   \
    type value;                                                                   \
    asm volatile("mov %0, %1" : "=r"(value) : "m"(*(volatile type*)addr)barrier); \
    return value;                                                                 \
  }

#define BUILD_MMIO_WRITE(name, type, barrier)                                               \
  static inline __attribute__((always_inline)) void name(volatile void* addr, type value) { \
    asm volatile("mov %0, %1" : : "m"(*(volatile type*)addr), "r"(value)barrier);           \
  }

BUILD_IO_IN(in8, uint8_t);
BUILD_IO_IN(in16, uint16_t);
BUILD_IO_IN(in32, uint32_t);

BUILD_IO_OUT(out8, uint8_t);
BUILD_IO_OUT(out16, uint16_t);
BUILD_IO_OUT(out32, uint32_t);

BUILD_MMIO_READ(read8, uint8_t, );
BUILD_MMIO_READ(read16, uint16_t, );
BUILD_MMIO_READ(read32, uint32_t, );

BUILD_MMIO_READ(read_mb8, uint8_t, : "memory");
BUILD_MMIO_READ(read_mb16, uint16_t, : "memory");
BUILD_MMIO_READ(read_mb32, uint32_t, : "memory");

BUILD_MMIO_WRITE(write8, uint8_t, );
BUILD_MMIO_WRITE(write16, uint16_t, );
BUILD_MMIO_WRITE(write32, uint32_t, );

BUILD_MMIO_WRITE(write_mb8, uint8_t, : "memory");
BUILD_MMIO_WRITE(write_mb16, uint16_t, : "memory");
BUILD_MMIO_WRITE(write_mb32, uint32_t, : "memory");

#undef BUILD_IO_IN
#undef BUILD_IO_OUT
#undef BUILD_MMIO_READ
#undef BUILD_MMIO_WRITE

#define mb() asm volatile("mfence" : : : "memory")

#endif
