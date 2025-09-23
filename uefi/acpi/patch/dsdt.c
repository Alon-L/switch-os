#include "dsdt.h"

#include <efi.h>
#include <efiapi.h>
#include <efidef.h>
#include <efilib.h>

#include "acpi/tables.h"
#include "mem.h"
#include "patch_utils.h"
#include "utils.h"

/// Whether the DSDT behind `g_dsdt` is a patched one (which was allocated by `alloc_patched_dsdt`).
static bool g_is_dsdt_patched = false;

/**
 * Allocates a new DSDT table in an EFI reserved memory area.
 *
 * The DSDT table is allocated in the lower 32 bit physical memory, to ensure `FADT->dsdt` can be used instead of
 * `FADT->x-dsdt`.
 */
static err_t alloc_patched_dsdt(size_t size, struct acpi_dsdt** patched_dsdt_out) {
  err_t err = SUCCESS;

  struct acpi_dsdt* patched_dsdt = (struct acpi_dsdt*)UINT32_MAX;
  CHECK(uefi_call_wrapper(BS->AllocatePages, 4, AllocateMaxAddress, EfiReservedMemoryType,
                          ALIGN_UP(size, EFI_PAGE_SIZE), (uintptr_t*)&patched_dsdt) == EFI_SUCCESS);
  CHECK(patched_dsdt != NULL);

  *patched_dsdt_out = patched_dsdt;

cleanup:
  return err;
}

/**
 * Frees the DSDT table behind `g_dsdt` which was allocated with `alloc_patched_dsdt`.
 * The caller must ensure that the DSDT was allocated with `alloc_patched_dsdt`, and isn't the original DSDT.
 */
static err_t free_patched_dsdt(void) {
  err_t err = SUCCESS;

  CHECK_TRACE(g_is_dsdt_patched, "The current g_dsdt table is not patched\n");

  size_t patched_dsdt_size = g_dsdt->hdr.length;
  CHECK(uefi_call_wrapper(BS->FreePages, 2, (uintptr_t)g_dsdt, ALIGN_UP(patched_dsdt_size, EFI_PAGE_SIZE)) ==
        EFI_SUCCESS);

cleanup:
  return err;
}

/**
 * Fixes the FADT table to point to a new allocated DSDT table.
 */
static err_t redirect_dsdt(struct acpi_dsdt* new_dsdt) {
  err_t err = SUCCESS;

  CHECK(g_fadt != NULL);

  g_fadt->x_dsdt = (uint64_t)(uintptr_t)new_dsdt;
  g_fadt->dsdt = (uint32_t)(uintptr_t)new_dsdt;

  fix_table_checksum((struct acpi_table_header*)g_fadt);

cleanup:
  return err;
}

err_t append_dsdt(const uint8_t* aml, size_t aml_size) {
  err_t err = SUCCESS;

  // Allocate a larger DSDT.
  size_t patched_dsdt_size = g_dsdt->hdr.length + aml_size;
  struct acpi_dsdt* patched_dsdt = NULL;
  CHECK_RETHROW(alloc_patched_dsdt(patched_dsdt_size, &patched_dsdt));

  // Copy the DSDT's original content, and update its length.
  memcpy(patched_dsdt, g_dsdt, g_dsdt->hdr.length);
  patched_dsdt->hdr.length = patched_dsdt_size;

  // Append the AML to the end of the new DSDT.
  memcpy(patched_dsdt->definition_block + DSDT_AML_SIZE(*g_dsdt), aml, aml_size);

  // Fix the table's checksum after modifying it.
  fix_table_checksum((struct acpi_table_header*)patched_dsdt);

  // Override the FADT to point to the newly allocated DSDT.
  CHECK_RETHROW(redirect_dsdt(patched_dsdt));

  if (g_is_dsdt_patched) {
    // Free the previously patched DSDT, which was allocated the same way.
    CHECK_RETHROW(free_patched_dsdt());
  }

  g_dsdt = patched_dsdt;
  g_is_dsdt_patched = true;

cleanup:
  return err;
}
