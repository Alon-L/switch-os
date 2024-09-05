#ifndef _CORE_LOADER
#define _CORE_LOADER

#define CORE_PHYS_ADDR (0x40000000)
#define CORE_MAX_PHYS_MEM_SIZE (0x4000000)

/*
 * Maps core onto the physical address {CORE_PHYS_ADDR}, and gives it
 * executable permissions.
 * Also handles instruction cache invalidation.
 *
 * @core_addr_out - The virtual memory of core mapped. This doesn't necessarily
 *  point to core's {_start} function, but the beginning of core's memory.
 */
int load_core(void** core_addr_out);

/*
 * Unloads core's memory that was previously loaded with {load_core}.
 */
void unload_core(void* core_addr);

#endif
