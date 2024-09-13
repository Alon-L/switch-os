#ifndef _HOOK_SLEEP_PREPARE
#define _HOOK_SLEEP_PREPARE

#include <linux/kernel.h>
#include "error.h"

err_t hook_sleep_prepare(void);

void unhook_sleep_prepare(void);

bool is_sleep_prepare_hooked(void);

#endif
