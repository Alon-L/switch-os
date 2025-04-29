#include <efi.h>
#include <efiapi.h>
#include <efidef.h>
#include <efierr.h>
#include <efilib.h>

#include "core/consts.h"

#define PAGE_SIZE (4096)

/**
 * Allocate pages of type EfiReservedMemoryType to reserve this memory from usage by the OS.
 */
static EFI_STATUS reserve_memory(EFI_PHYSICAL_ADDRESS start, size_t size) {
  size_t pages = size / PAGE_SIZE;
  return uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiReservedMemoryType, pages, &start);
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
  InitializeLib(ImageHandle, SystemTable);

  // Reserve memory for core.
  if (EFI_ERROR(reserve_memory(CORE_PHYS_ADDR, CORE_MAX_PHYS_MEM_SIZE))) {
    return EFI_LOAD_ERROR;
  }

  CHECK_RETHROW(fill_core_header(core_header));

  if (EFI_ERROR(reserve_memory(CORE_PM_PHYS_ADDR, CORE_MAX_PM_PHYS_MEM_SIZE))) {
    return EFI_LOAD_ERROR;
  }

  return EFI_SUCCESS;
}
