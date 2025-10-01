#include "commands.h"

#include <efi.h>

#include "error.h"
#include "guid.h"
#include "handlers/set_core_action.h"
#include "mem.h"

static const EFI_GUID g_efi_commands_guid = EFI_COMMANDS_GUID;

bool is_commands_guid(const EFI_GUID* guid) {
  return memcmp(guid, &g_efi_commands_guid, sizeof(EFI_GUID)) == 0;
}

err_t handle_command(const wchar_t* cmd_name, void* data, size_t data_size) {
  err_t err = SUCCESS;

  // This macro creates an if clause for every supported command.
  //
  // The idea approach here would be to create an array of pairs of commands and their handles, and search for the
  // command through it. However, this would require a relocation (since we're passing a function pointer), which is not
  // supported for our services hooks. Instead, this method uses rip-relative code which does not require relocations.
#define HANDLE_COMMAND(name, handler) \
  if (wstrcmp(cmd_name, name) == 0) CHECK_RETHROW(handler(data, data_size))

  HANDLE_COMMAND(L"SET_CORE_ACTION", set_core_action_handler);

cleanup:
  return err;
}
