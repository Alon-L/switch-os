#ifndef _INCLUDE_MEM_AREA_H
#define _INCLUDE_MEM_AREA_H

#include <stdbool.h>
#include <stdint.h>

struct mem_area {
  uint64_t start;
  uint64_t size;
};

static inline bool is_mem_area_contained_in_range(const struct mem_area* area, uint64_t start, uint64_t end) {
  return start <= area->start && area->start + area->size <= end;
}

#endif
