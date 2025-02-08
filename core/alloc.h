#ifndef _INCLUDE_ALLOC_H
#define _INCLUDE_ALLOC_H

#include <stddef.h>

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
