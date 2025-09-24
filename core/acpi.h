#ifndef _ACPI_H
#define _ACPI_H

#include <stdint.h>

#include "error.h"

/**
 * Initializes uACPI and enters ACPI mode.
 * This is required to return back to the kernel using `acpi_return_kernel`.
 */
err_t acpi_setup(void);

/**
 * Reset the state of the ACPI initialization done through `acpi_setup`.
 * This deallocates all the used memory, and resets the internal uACPI's context struct.
 */
void acpi_destroy(void);

/**
 * Returns back to the original kernel.
 * Sets the ACPI waking vector to the original kernel's waking vector and enters S3 suspend.
 * The original kernel will remain normal operation when the machine wakes back.
 */
void acpi_return_kernel(uint32_t waking_vector);

#endif
