#include "alloc.h"

#include <stdint.h>

#include "mem.h"
#include "trace.h"
#include "utils.h"

// The free blocks are handled via a linked list which is entirely stored in the free blocks.
// Every free block contains this header in its memory.
// The allocator contains a pointer to the list's head.
struct free_block {
  struct free_block* next;
};

struct allocator {
  size_t block_size;

  uint8_t* buf;
  size_t buf_size;

  struct free_block* head;
};

// Define a block allocator.
// This creates a `g_allocator_{bs}` allocator, and a buffer aligned to the given align value.
// The allocator's head is not initialized here, but in `core_init_allocators`.
#define DEFINE_ALLOCATOR(_block_size, _buf_size, _align)                             \
  static __attribute__((aligned(_align))) uint8_t g_buffer_##_block_size[_buf_size]; \
  static struct allocator g_allocator_##_block_size = {                              \
    .block_size = _block_size,                                                       \
    .buf = g_buffer_##_block_size,                                                   \
    .buf_size = _buf_size,                                                           \
  };                                                                                 \
  _Static_assert(_block_size >= sizeof(struct free_block),                           \
                 "Allocator's block size is not large enough to contain the free block header.")

DEFINE_ALLOCATOR(32, 32 * 1024, 32);
DEFINE_ALLOCATOR(64, 64 * 1024, 64);
DEFINE_ALLOCATOR(128, 128 * 1024, 128);
DEFINE_ALLOCATOR(256, 256 * 1024, 256);
DEFINE_ALLOCATOR(8192, 8192 * 32, 4096);

// This array contains all the allocators.
// It is assumed to be ascending relative to the allocators' block size.
static struct allocator* g_allocators[] = {&g_allocator_32, &g_allocator_64, &g_allocator_128, &g_allocator_256,
                                           &g_allocator_8192};

/**
 * Returns the allocator that allocated a given address.
 */
static struct allocator* find_allocator_for_allocation(void* addr) {
  for (size_t i = 0; i < ARRAY_SIZE(g_allocators); i++) {
    struct allocator* allocator = g_allocators[i];
    // Check if the address is contained in the allocator's buffer.
    if ((uintptr_t)allocator->buf <= (uintptr_t)addr &&
        (uintptr_t)allocator->buf + allocator->buf_size >= (uintptr_t)addr) {
      return allocator;
    }
  }
  return NULL;
}

/**
 * Returns the optimal allocator for allocating memory of given size.
 *
 * The optimal allocator is the allocator with the minimal block size that can contain the given size.
 */
static struct allocator* find_allocator_for_size(size_t size) {
  // We assume `g_allocators` is ascending relative to the allocators' block size.
  for (size_t i = 0; i < ARRAY_SIZE(g_allocators); i++) {
    struct allocator* allocator = g_allocators[i];
    if (allocator->block_size >= size) {
      return allocator;
    }
  }
  return NULL;
}

void core_init_allocators(void) {
  for (size_t i = 0; i < ARRAY_SIZE(g_allocators); i++) {
    struct allocator* allocator = g_allocators[i];
    // Iterate over all the blocks in the allocator's buffer, and chain them together into a linked list of free blocks.
    for (void* addr = allocator->buf; (uintptr_t)addr < (uintptr_t)allocator->buf + allocator->buf_size;
         addr += allocator->block_size) {
      struct free_block* prev_head = allocator->head;
      allocator->head = (struct free_block*)addr;
      allocator->head->next = prev_head;
    }
  }
}

void* core_malloc(size_t size) {
  struct allocator* allocator = find_allocator_for_size(size);
  if (allocator == NULL || allocator->head == NULL) {
    TRACE("No allocator head for allocator %lu\n", allocator->block_size);
    return NULL;
  }

  // Pop a free block and return it as the allocation.
  void* ptr = allocator->head;
  allocator->head = allocator->head->next;
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
  struct allocator* allocator = find_allocator_for_allocation(ptr);
  if (allocator) {
    // Insert a free block at the freed address.
    ((struct free_block*)ptr)->next = allocator->head;
    allocator->head = ptr;
  }
}
