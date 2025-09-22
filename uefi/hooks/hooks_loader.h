#ifndef _HOOKS_HEADER
#define _HOOKS_HEADER

#include <stdbool.h>
#include <stddef.h>

#include "error.h"

struct loaded_hook {
  /// Whether the hook was loaded to memory. This value does not represent whether the hook is in place.
  bool is_loaded;

  /// The pointer to the header of the hook. This should be the beginning of the hook area.
  void* header;

  /// The pointer to the entry of the hook. This is the entry function that should be called for the hook.
  void* entry;
};

#define DECLARE_HOOK_BINARY(name)                     \
  extern char _binary_build_##name##_raw_bin_start[]; \
  extern char _binary_build_##name##_raw_bin_end[]

#define HOOK_START(name) (_binary_build_##name##_raw_bin_start)

#define HOOK_END(name) (_binary_build_##name##_raw_bin_end)

#define HOOK_SIZE(name) ((uintptr_t)HOOK_END(name) - (uintptr_t)HOOK_START(name))

/**
 * Loads the hook into memory under a `EfiRuntimeServicesCode` descriptor, and initializes `loaded_hook_out` with the
 * pointers to the hook's header and entry.
 *
 * The hook's memory won't be released when the UEFI application quits. This is required for services hooks which must
 * outlive the application.
 */
err_t load_hook(const void* hook_start, size_t hook_size, struct loaded_hook* loaded_hook_out);

/**
 * Frees the memory pages for a hook allocated using `load_hook`.
 *
 * Does nothing if the hook is already unloaded.
 */
void free_hook(struct loaded_hook* loaded_hook, size_t hook_size);

/**
 * Place hooks on the required UEFI runtime and boot services, and register any additional events that are required.
 * See the `loaders` and `raw` directories for a list of all hooks.
 */
err_t hook_services(void);

/**
 * Unhook everything that was hooked with `hook_services`.
 *
 * This only unhooks the hooked services.
 */
void unhook_services(void);

#endif
