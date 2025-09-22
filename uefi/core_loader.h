#pragma once

#include "core/header.h"
#include "error.h"

/*
 * Maps all parts of  core onto their respective physical addresses (see `core/consts.h`).
 * Also validates the magic at the beginning of the core header.
 *
 * @core_addr_out - The virtual memory of core mapped. This doesn't necessarily
 *  point to core's {_start} function, but the beginning of core's memory.
 */
err_t load_core(struct core_header** core_addr_out);

/*
 * Frees core's memory that was previously loaded with `load_core`.
 */
void free_core(struct core_header* core_header);
