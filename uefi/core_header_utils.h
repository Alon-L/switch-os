#pragma once

#include <efi.h>
#include <efiapi.h>

#include "core/header.h"
#include "error.h"

err_t fill_core_header(struct core_header* core_header);
