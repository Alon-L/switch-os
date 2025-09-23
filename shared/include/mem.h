#ifndef _INCLUDE_MEM
#define _INCLUDE_MEM

#include <stddef.h>
#include <stdint.h>

static inline void memcpy(void* dst, const void* src, size_t size) {
  uint8_t* dst_bytes = (uint8_t*)dst;
  const uint8_t* src_bytes = (uint8_t*)src;

  for (size_t i = 0; i < size; i++) {
    dst_bytes[i] = src_bytes[i];
  }
}

static inline void memset(void* buf, uint8_t c, size_t n) {
  uint8_t* buf_bytes = (uint8_t*)buf;

  for (size_t i = 0; i < n; i++) {
    buf_bytes[i] = c;
  }
}

static inline int memcmp(const void* buf1, const void* buf2, size_t size) {
  const uint8_t* buf1_bytes = (uint8_t*)buf1;
  const uint8_t* buf2_bytes = (uint8_t*)buf2;

  for (size_t i = 0; i < size; i++) {
    if (buf1_bytes[i] != buf2_bytes[i]) {
      return buf1_bytes[i] - buf2_bytes[i];
    }
  }

  return 0;
}

static inline int strlen(const char* str) {
  size_t len = 0;
  while (str[len] != 0) len++;
  return len;
}

static inline int strcmp(const char* str1, const char* str2) {
  size_t i = 0;

  while (str1[i] && str2[i]) {
    if (str1[i] != str2[i]) {
      break;
    }

    i++;
  }

  return str1[i] - str2[i];
}

static inline int wstrcmp(const wchar_t* str1, const wchar_t* str2) {
  size_t i = 0;

  while (str1[i] && str2[i]) {
    if (str1[i] != str2[i]) {
      break;
    }

    i++;
  }

  return str1[i] - str2[i];
}

#endif
