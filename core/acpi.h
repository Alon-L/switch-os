#ifndef _ACPI_H
#define _ACPI_H

#include "error.h"

/**
 * Initializes uACPI and enters ACPI mode.
 * This is required to return back to the kernel using `acpi_return_kernel`.
 */
err_t acpi_setup(void);

/**
 * Returns back to the original kernel.
 * Sets the ACPI waking vector to the original kernel's waking vector and enters S3 suspend.
 * The original kernel will remain normal operation when the machine wakes back.
 */
void acpi_return_kernel(void);

#endif
