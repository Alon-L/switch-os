#ifndef _SET_VARIABLE_HOOK_COMMANDS_H
#define _SET_VARIABLE_HOOK_COMMANDS_H

#include <efi.h>
#include <stdbool.h>

#include "error.h"

// Arbitrary GUID that the running kernel needs to use in order to communicate with us and send commands.
#define EFI_COMMANDS_GUID {0xe21c66ed, 0xbc2e, 0x4d30, 0xac, 0x5e, 0x1b, 0x96, 0x80, 0x02, 0xc5, 0x41}

/**
 * Returns whether a given guid is equal to `EFI_COMMANDS_GUID`.
 *
 * This function should be called by the `SetVariable` hook to detect whether a command should be executed.
 */
bool is_commands_guid(const EFI_GUID* guid);

/**
 * Receives the command name, and executes its corresponding command handler.
 * Note that `data` may change due by the handler.
 *
 * This function should be called by the `SetVariable` hook with the variable name as the command name, after validating
 * the passed guid matches the special commands guid `EFI_COMMANDS_GUID`, using `is_commands_guid`.
 */
err_t handle_command(const wchar_t* cmd_name, void* data, size_t data_size);

#endif
