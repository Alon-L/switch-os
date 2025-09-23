#pragma once

#include <stddef.h>
#include <stdint.h>

#include "error.h"

/**
 * Appends AML code to the end of the DSDT table.
 *
 * This function reallocates a new larger DSDT to make space for the new AML code.
 * It deals with fixing the DSDT's checksum, and fixing the FADT table to point to the new DSDT table.
 */
err_t append_dsdt(const uint8_t* aml, size_t aml_size);
