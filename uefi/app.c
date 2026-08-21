#include <core/consts.h>
#include <efi.h>
#include <efilib.h>
#include <error.h>

#include "acpi/hook_pts.h"
#include "acpi/tables.h"
#include "core/header.h"
#include "core_header_utils.h"
#include "core_loader.h"
#include "services/hooks_loader.h"

struct core_header* g_core_header = NULL;

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
  err_t err = SUCCESS;

  InitializeLib(ImageHandle, SystemTable);

  TRACE("Loading core...\n");
  CHECK_RETHROW(load_core(&g_core_header));

  TRACE("Locating ACPI tables...\n");
  CHECK_RETHROW(find_acpi_tables());

  TRACE("Hooking UEFI services...\n");
  CHECK_RETHROW(hook_services());

  TRACE("Hooking the _PTS aml method...\n");
  CHECK_RETHROW(create_or_hook_pts());

  TRACE("Filling core header...\n");
  CHECK_RETHROW(fill_core_header());

  TRACE("Initialized!\n");

cleanup:
  if (!IS_SUCCESS(err)) {
    unhook_services();
    return EFI_LOAD_ERROR;
  } else {
    return EFI_SUCCESS;
  }
}
