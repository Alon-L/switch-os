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
  uint32_t magic;
  uint32_t original_waking_vector;
  // The offset of core as a waking vector from the beginning of the header
  // TODO: this needs to be a uint16 - the address must be accessible from realmode.
  uint32_t core_waking_vector_offset;
};

static inline void core_header_set_original_waking_vector(
    struct core_header* core_header, uint32_t original_waking_vector) {
  core_header->original_waking_vector = original_waking_vector;
}

static inline uint32_t core_header_get_core_waking_vector_addr(
    struct core_header* core_header) {
  return *(uint32_t*)((void*)core_header + core_header->core_waking_vector_offset);
}

static inline bool is_core_header_magic_valid(struct core_header* core_header) {
  return core_header->magic == CORE_HEADER_MAGIC;
}

#endif
