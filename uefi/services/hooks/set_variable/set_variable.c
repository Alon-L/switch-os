#include <efi.h>

#include "../headers.h"
#include "core/consts.h"
#include "mem.h"
#include "trace.h"

#define WINDOWS_SLEEP_CHECKPOINT_VARIABLE_NAME (L"SystemSleepCheckpoint")

__attribute__((section(".header"))) struct set_variable_hook_header g_hook_header = {};

EFI_STATUS EFIAPI _start(IN CHAR16* VariableName, IN EFI_GUID* VendorGuid, IN UINT32 Attributes, IN UINTN DataSize,
                         IN VOID* Data) {
  // The Windows kernel implements a mechanism called "Sleep Checkpoints", which traces the progress of entering sleep
  // by updating a UEFI variable every few steps in its sleep process.
  // Some of the writes of this variable occur after updating the waking vector, so we can override it right after.
  if (wstrcmp(VariableName, WINDOWS_SLEEP_CHECKPOINT_VARIABLE_NAME) == 0 &&
      *g_hook_header.waking_vector_addr != CORE_RM_PHYS_ADDR) {
    TRACE("Updating the waking vector from the set_variable hook\n");

    *g_hook_header.original_waking_vector_addr = *g_hook_header.waking_vector_addr;
    *g_hook_header.waking_vector_addr = CORE_RM_PHYS_ADDR;
  }

  // Call the original SetVariable function.
  return g_hook_header.original_set_variable(VariableName, VendorGuid, Attributes, DataSize, Data);
}
