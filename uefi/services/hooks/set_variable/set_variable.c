#include <efi.h>

#include "core/consts.h"
#include "mem.h"
#include "services/headers.h"
#include "set_variable/commands.h"
#include "trace.h"

#define WINDOWS_SLEEP_CHECKPOINT_VARIABLE_NAME (L"SystemSleepCheckpoint")

__attribute__((section(".header"))) struct set_variable_hook_header g_hook_header = {};

EFI_STATUS EFIAPI _start(IN CHAR16* VariableName, IN EFI_GUID* VendorGuid, IN UINT32 Attributes, IN UINTN DataSize,
                         IN VOID* Data) {
  if (is_commands_guid(VendorGuid)) {
    if (DataSize == 0) {
      // Some implementations of kernel `SetVariable` wrappers delete the variable prior to updating it, which is done
      // by passing 0 size.
      return EFI_SUCCESS;
    }

    // The running kernel is trying to communicate with us and send us commands.
    // Our command handler may change the value behind the `Data` pointer even though it's `IN`.
    // We shouldn't call the original SetVariable, since this isn't a real call to this function.
    err_t cmd_res = handle_command(VariableName, Data, DataSize);
    if (IS_SUCCESS(cmd_res)) {
      return EFI_SUCCESS;
    } else {
      return EFIERR(cmd_res);
    }
  }

  // The Windows kernel implements a mechanism called "Sleep Checkpoints", which traces the progress of entering sleep
  // by updating a UEFI variable every few steps in its sleep process.
  // Some of the writes of this variable occur after updating the waking vector, so we can override it right after.
  if (wstrcmp(VariableName, WINDOWS_SLEEP_CHECKPOINT_VARIABLE_NAME) == 0 &&
      *g_hook_header.waking_vector_addr != CORE_WAKEUP_PHYS_ADDR) {
    TRACE("Sleep checkpoint caught. Updating the waking vector from the set_variable hook\n");

    *g_hook_header.original_waking_vector_addr = *g_hook_header.waking_vector_addr;
    *g_hook_header.waking_vector_addr = CORE_WAKEUP_PHYS_ADDR;
  }

  // Call the original SetVariable function.
  return g_hook_header.original_set_variable(VariableName, VendorGuid, Attributes, DataSize, Data);
}
