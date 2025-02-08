#ifndef _INCLUDE_ALLOC_H
#define _INCLUDE_ALLOC_H

#include <stddef.h>

/**
 * Initialize all the block allocators.
 * This must be called prior to any (de)allocations.
 */
void core_init_allocators(void);

/**
 * A malloc implementation for core.
 * All memory is aligned to the requested size.
 */
void* core_malloc(size_t size);

/**
 * A calloc implementation for core.
 */
void* core_calloc(size_t count, size_t size);

/**
 * A free implementation for core.
 */
void core_free(void* ptr);

#endif
