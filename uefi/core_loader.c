#include "core_loader.h"

#include <core/consts.h>
#include <core/header.h>
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>
#include <efierr.h>
#include <efilib.h>
#include <error.h>

extern char _binary_build_core_bin_trimmed_start[];
extern char _binary_build_core_bin_trimmed_end[];

extern char _binary_build_rm_bin_trimmed_start[];
extern char _binary_build_rm_bin_trimmed_end[];

extern char _binary_build_pm_bin_trimmed_start[];
extern char _binary_build_pm_bin_trimmed_end[];

#define CORE_START (_binary_build_core_bin_trimmed_start)
#define CORE_SIZE ((uintptr_t)_binary_build_core_bin_trimmed_end - (uintptr_t)_binary_build_core_bin_trimmed_start)

#define CORE_RM_START (_binary_build_rm_bin_trimmed_start)
#define CORE_RM_SIZE ((uintptr_t)_binary_build_rm_bin_trimmed_end - (uintptr_t)_binary_build_rm_bin_trimmed_start)

#define CORE_PM_START (_binary_build_pm_bin_trimmed_start)
#define CORE_PM_SIZE ((uintptr_t)_binary_build_pm_bin_trimmed_end - (uintptr_t)_binary_build_pm_bin_trimmed_start)

/**
 * Allocate pages of type EfiReservedMemoryType to reserve this memory from usage by the OS.
 */
static err_t reserve_memory(uintptr_t start, size_t size) {
  err_t err = SUCCESS;
  size_t pages = ALIGN_UP(size, EFI_PAGE_SIZE) / EFI_PAGE_SIZE;

  CHECK(uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiReservedMemoryType, pages, &start) == EFI_SUCCESS);

cleanup:
  return err;
}

/**
 * Writes a given buffer to a specified physical address.
 * @param addr  - The physical address to write to.
 * @param buf   - The buffer to write.
 * @param size  - The size of the buffer.
 */
static err_t load_memory(uintptr_t addr, const void* buf, size_t size) {
  err_t err = SUCCESS;

  CHECK_RETHROW(reserve_memory(addr, size));
  __builtin_memcpy((void*)addr, buf, size);

cleanup:
  return err;
}

err_t load_core(struct core_header** core_header_out) {
  err_t err = SUCCESS;

  CHECK_TRACE(CORE_SIZE <= CORE_MAX_PHYS_MEM_SIZE, "Not enough reserved RAM for core\n");
  CHECK_TRACE(CORE_RM_SIZE <= CORE_MAX_RM_PHYS_MEM_SIZE, "Not enough reserved RAM for core rm\n");
  CHECK_TRACE(CORE_PM_SIZE <= CORE_MAX_PM_PHYS_MEM_SIZE, "Not enough reserved RAM for core pm\n");

  CHECK_RETHROW(load_memory(CORE_PHYS_ADDR, CORE_START, CORE_SIZE));
  CHECK_RETHROW(load_memory(CORE_RM_PHYS_ADDR, CORE_RM_START, CORE_RM_SIZE));
  CHECK_RETHROW(load_memory(CORE_PM_PHYS_ADDR, CORE_PM_START, CORE_PM_SIZE));

  struct core_header* core_header = (struct core_header*)CORE_PHYS_ADDR;
  CHECK(is_core_header_magic_valid(core_header));

  *core_header_out = core_header;

cleanup:
  return err;
}

/**
 * Frees pages allocated using `reserve_memory`.
 */
static err_t free_memory(uintptr_t start, size_t size) {
  err_t err = SUCCESS;
  size_t pages = ALIGN_UP(size, EFI_PAGE_SIZE) / EFI_PAGE_SIZE;

  CHECK(uefi_call_wrapper(BS->FreePages, 4, start, pages) == EFI_SUCCESS);

cleanup:
  return err;
}

void unload_core(struct core_header* core_header) {
  free_memory(CORE_PHYS_ADDR, CORE_SIZE);
  free_memory(CORE_RM_PHYS_ADDR, CORE_RM_SIZE);
  free_memory(CORE_PM_PHYS_ADDR, CORE_PM_SIZE);
}
