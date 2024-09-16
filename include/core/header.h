#ifndef _INCLUDE_CORE_HEADER
#define _INCLUDE_CORE_HEADER

#include "core/consts.h"

#if defined(CORE)
#include <stdint.h>
#elif defined(MODULE)
#include <linux/kernel.h>
#endif

typedef int (*core_start_t)(void);

struct core_header {
  uint32_t magic;
  uint32_t start_offset;
};

static inline core_start_t core_header_get_start(
    struct core_header* core_header) {
  return (core_start_t)((void*)core_header + core_header->start_offset);
}

static inline bool is_core_header_magic_valid(struct core_header* core_header) {
  return core_header->magic == CORE_HEADER_MAGIC;
}

#endif
