#pragma once

#include "error.h"

/**
 * Places a hook on the `_PTS` method if it exists, or creates a new `_PTS` with the hook body if it doesn't exist.
 *
 * If the `_PTS` method already exists, it is renamed to `SPTS` and our hook calls it when it's done.
 *
 * The hook content is taken from `pts_hook.asl`. Notice it always tries to call `SPTS`. We explicitly remove this call
 * if the `_PTS` doesn't exist (in this case the hook becomes the actual `_PTS` method, and the `SPTS` method does not
 * exist).
 */
err_t create_or_hook_pts(void);
