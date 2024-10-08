#ifndef _INCLUDE_CORE_HEADER
#define _INCLUDE_CORE_HEADER

#include "core/consts.h"

#if defined(CORE)
#include <stdint.h>
#include <stdbool.h>
#elif defined(MODULE)
#include <linux/kernel.h>
#endif

typedef int (*core_start_t)(void);

struct core_header {
  const uint32_t magic;
  uint32_t original_waking_vector;
} __attribute__((packed));

static inline bool is_core_header_magic_valid(struct core_header* core_header) {
  return core_header->magic == CORE_HEADER_MAGIC;
}

#endif
