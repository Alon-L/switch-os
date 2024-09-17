#ifndef _HOOK_SLEEP_PREPARE
#define _HOOK_SLEEP_PREPARE

#include <linux/kernel.h>
#include "error.h"

/*
 * `acpi_sleep_prepare` is called right before the kernel enters suspend mode,
 * and sets the waking vector. Therefore, we have to interrupt it and override the
 * waking vector to core's waking vector.
 */
err_t hook_sleep_prepare(void);

/*
 * Removes the hook placed by `hook_sleep_prepare`.
 */
void unhook_sleep_prepare(void);

bool is_sleep_prepare_hooked(void);

#endif
