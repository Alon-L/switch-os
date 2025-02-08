#include "alloc.h"

#include <stdint.h>

#include "mem.h"

#define ALIGN_DOWN(x, align_to) ((x) & ~((align_to) - 1))

static __attribute__((aligned(4096))) uint8_t g_buffer[4096 * 64];
static void* g_next_free = g_buffer;

/**
 * Returns the size left for allocation.
 */
static inline size_t size_left() {
  return (uintptr_t)g_buffer + sizeof(g_buffer) - (uintptr_t)(g_next_free);
}

void* core_malloc(size_t size) {
  size_t aligned_size_left = ALIGN_DOWN(size_left(), size);
  if (aligned_size_left < size) {
    return NULL;
  }

  // Make sure the allocated memory is aligned
  size_t align_off = size_left() - aligned_size_left;
  g_next_free += align_off;

  void* ptr = g_next_free;

  g_next_free += size;

  return ptr;
}

void* core_calloc(size_t count, size_t size) {
  size_t total_size = count * size;

  void* ptr = core_malloc(total_size);
  if (ptr == NULL) {
    return NULL;
  }

  memset(ptr, 0, total_size);

  return ptr;
}

void core_free(void* ptr) {
  // Free is currently a NOOP for this simple extremely inefficient allocator.
  (void)ptr;
}
