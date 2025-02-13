#ifndef _DEVICES_H
#define _DEVICES_H

#include "error.h"

/**
 * Register module's character devices.
 * These devices are used to communicate with this module from usermode, and call core.
 */
err_t register_devices(void);

#endif
