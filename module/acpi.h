#ifndef _ACPI_H
#define _ACPI_H

#include <linux/types.h>

#include "error.h"

/**
 * Returns the physical address of the RSDP table.
 */
err_t acpi_find_rsdp(uint64_t* rsdp_out);

/**
 * Returns the physical address of the currently set waking vector.
 */
err_t acpi_find_waking_vector(uint32_t* waking_vector_out);

#endif
