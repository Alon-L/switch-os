#include <core/consts.h>
#include <efi.h>
#include <efilib.h>
#include <error.h>

#include "acpi/tables.h"
#include "core_header_utils.h"
#include "core_loader.h"
#include "hooks/hooks_loader.h"

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
  err_t err = SUCCESS;

  InitializeLib(ImageHandle, SystemTable);

  TRACE("Loading core...\n");
  struct core_header* core_header = NULL;
  CHECK_RETHROW(load_core(&core_header));

  TRACE("Locating ACPI tables...\n");
  CHECK_RETHROW(find_acpi_tables());

  TRACE("Hooking UEFI services...\n");
  CHECK_RETHROW(hook_services());

  TRACE("Filling core header...\n");
  CHECK_RETHROW(fill_core_header(core_header));

cleanup:
  if (IS_ERROR(err)) {
    unhook_services();
    return EFI_LOAD_ERROR;
  } else {
    return EFI_SUCCESS;
  }
}
