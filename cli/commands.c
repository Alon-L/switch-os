#include "commands.h"

#include <argtable3.h>

#include "guid.h"
#include "osl.h"

err_t send_uefi_command(const wchar_t* name, void* data, size_t data_size) {
  err_t err = SUCCESS;

  struct guid commands_guid = EFI_COMMANDS_GUID;
  CHECK_RETHROW_TRACE(set_uefi_variable(&commands_guid, name, data, data_size), "Failed to set UEFI variable %ls\n",
                      name);

cleanup:
  return err;
}
