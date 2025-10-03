#pragma once

#include <argtable3.h>
#include <wchar.h>

#include "error.h"

typedef int (*cmd_handler)(int argc, char* argv[], arg_dstr_t res, void* ctx);

// This struct describes each of the subcommands of the main application.
struct cmd {
  /// The name of the subcommand.
  const char* name;

  /// The description of the subcommand. Shown when running the `help` subcommand, or when passing `--help` to the
  /// subcommand.
  const char* description;

  /// The function executed when the subcommand is passed. The return value of the subcommand is the exitcode of the
  /// application. Any output should be passed to the `res` dynamic string.
  const cmd_handler handler;
};

/**
 * Sends a command to the UEFI application via its `SetVariable` hook.
 *
 * This calls the `SetVariable` UEFI runtime service, with the magic GUID `EFI_COMMANDS_GUID` which identifies commands
 * to the hook. Note that the value of `data` may change due to this call, depending on the command.
 */
err_t send_uefi_command(const wchar_t* name, void* data, size_t data_size);
