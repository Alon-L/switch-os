#include <core/consts.h>
#include <efi.h>
#include <efilib.h>
#include <error.h>

#include "core_header_utils.h"
#include "core_loader.h"

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
  err_t err = SUCCESS;

  InitializeLib(ImageHandle, SystemTable);

  TRACE("Loading core...\n");

  struct core_header* core_header = NULL;
  CHECK_RETHROW(load_core(&core_header));

  TRACE("Filling core header...\n");

  CHECK_RETHROW(fill_core_header(core_header));

cleanup:
  return IS_SUCCESS(err) ? EFI_SUCCESS : EFI_LOAD_ERROR;
}
