#pragma once

#include <wchar.h>

#include "error.h"
#include "guid.h"

/**
 * Enables all privileges required for calling UEFI services (specificially the `SetVariable` service).
 */
err_t enable_uefi_privileges(void);

/**
 * Enables all privileges required for entering S3 mode.
 */
err_t enable_sleep_privileges(void);

// TODO: Is UEFI supported

/**
 * Calls the `SetVariable` UEFI runtime service with the given `guid`, `name`, `data` and `data_size`.
 */
err_t set_uefi_variable(struct guid* guid, const wchar_t* name, void* data, size_t data_size);

/**
 * Immediately enters S3.
 */
err_t enter_sleep_s3(void);
