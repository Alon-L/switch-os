#pragma once

#include <efi.h>
#include <efiapi.h>

#include "error.h"

/**
 * Fills the core header with all the required information.
 */
err_t fill_core_header();
