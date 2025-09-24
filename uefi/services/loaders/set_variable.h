#ifndef _HOOKS_LOADERS_SET_VARIABLE_H
#define _HOOKS_LOADERS_SET_VARIABLE_H

#include "error.h"

extern struct loaded_hook g_loaded_set_variable;

err_t hook_set_variable(void);

void unhook_set_variable(void);

#endif
