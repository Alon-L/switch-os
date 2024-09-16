#ifndef _CORE_LOADER
#define _CORE_LOADER

#include "core/header.h"
#include "error.h"

/*
 * Maps core onto the physical address {CORE_PHYS_ADDR}, and gives it
 * executable permissions.
 * Also validates the magic at the beginning of the core header.
 *
 * @core_addr_out - The virtual memory of core mapped. This doesn't necessarily
 *  point to core's {_start} function, but the beginning of core's memory.
 */
err_t load_core(struct core_header** core_addr_out);

/*
 * Unloads core's memory that was previously loaded with {load_core}.
 */
void unload_core(struct core_header* core_header);

#endif
