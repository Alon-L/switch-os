#ifndef _HOOKS_LOADERS_VIRTUAL_ADDRESS_MAP_EVENT_H
#define _HOOKS_LOADERS_VIRTUAL_ADDRESS_MAP_EVENT_H

#include "error.h"

extern struct loaded_hook g_loaded_virtual_address_map_event;

err_t create_virtual_address_map_event(void);

void remove_virtual_address_map_event(void);

#endif
