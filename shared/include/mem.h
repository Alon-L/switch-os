#ifndef _INCLUDE_MEM
#define _INCLUDE_MEM

#include <stddef.h>
#include <stdint.h>

static inline void memcpy(void* dst, const void* src, size_t size) {
  for (size_t i = 0; i < size; i++) {
    *((uint8_t*)dst + i) = *((uint8_t*)src + i);
  }
}

static inline void memset(void* buf, uint8_t c, size_t n) {
  for (size_t i = 0; i < n; i++) {
    *((uint8_t*)buf + i) = c;
  }
}

#endif
