#ifndef _HOOKS_LOADERS_GET_MEMORY_MAP_H
#define _HOOKS_LOADERS_GET_MEMORY_MAP_H

#include "error.h"

extern struct loaded_hook g_loaded_get_memory_map;

err_t hook_get_memory_map(void);

void unhook_get_memory_map(void);

#endif
