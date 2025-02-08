#ifndef _CONFIGURE_CORE_HEADER_H
#define _CONFIGURE_CORE_HEADER_H

#include "error.h"

/**
 * Configure the core header. This should be called by our `sleep_prepare` hook
 * right before entering S3.
 */
err_t configure_core_header(void);

#endif
