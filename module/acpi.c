#include "acpi.h"

#include <linux/acpi.h>
#include <linux/efi.h>

err_t acpi_find_rsdp(uint64_t* rsdp_out) {
  err_t err = SUCCESS;

  // Taken from the kernel's `cpi_os_get_root_pointer`
  CHECK(efi_enabled(EFI_CONFIG_TABLES));
  CHECK(efi.acpi20 != EFI_INVALID_TABLE_ADDR);

  *rsdp_out = efi.acpi20;

cleanup:
  return err;
}

err_t acpi_find_waking_vector(uint32_t* waking_vector_out) {
  err_t err = SUCCESS;
  struct acpi_table_facs* facs = NULL;

  // The waking vector is stored in the FACS ACPI table.
  // We can access this table by using the FADT ACPI table, as it stores the
  // physical address of the FACS table.

  struct acpi_table_fadt fadt = acpi_gbl_FADT;
  uint32_t facs_phys_addr = fadt.facs;
  CHECK_TRACE(facs_phys_addr != 0, "FACS table physical address is uninitialized by the kernel\n");

  // The ACPI tables must be mapped as write-back.
  facs = ioremap_cache(facs_phys_addr, sizeof(struct acpi_table_facs));
  CHECK_TRACE(facs != NULL, "Failed to map FACS table onto virtual memory\n");

  uint32_t waking_vector = facs->firmware_waking_vector;
  CHECK_TRACE(waking_vector != 0, "Waking vector physical address is uninitialized by the kernel\n");

  *waking_vector_out = waking_vector;

cleanup:
  if (facs != NULL) {
    // Unmap FACS from virtual memory
    iounmap(facs);
  }

  return err;
}
