#include "tables.h"

#include <efi.h>
#include <efiapi.h>
#include <efidef.h>
#include <efilib.h>

#include "mem.h"

struct acpi_rsdp* g_rsdp = NULL;
struct acpi_rxsdt* g_rxsdt = NULL;
struct acpi_fadt* g_fadt = NULL;
struct acpi_facs* g_facs = NULL;
struct acpi_dsdt* g_dsdt = NULL;

/**
 * Locates the RSDP table by scanning the UEFI system table, and initializes `g_rsdp`.
 *
 * NOTE: This implementation currently only works for ACPI 2.0.
 */
static err_t find_rsdp(void) {
  err_t err = SUCCESS;
  // TODO: Support `ACPI_TABLE_GUID` as well.
  static EFI_GUID acpi_20_table_guid = ACPI_20_TABLE_GUID;

  CHECK(ST->ConfigurationTable != NULL);

  for (size_t i = 0; i < ST->NumberOfTableEntries; i++) {
    EFI_CONFIGURATION_TABLE* table = &ST->ConfigurationTable[i];

    if (CompareGuid(&table->VendorGuid, &acpi_20_table_guid) == 0) {
      // The RSDP should only appear once.
      CHECK_TRACE(g_rsdp == NULL, "The RSDP was found twice inside the SystemTable\n");
      g_rsdp = table->VendorTable;
    }
  }

  CHECK(g_rsdp != NULL);

cleanup:
  return err;
}

/**
 * Uses the RSDP table to find either the XSDT or RSDT table, and initializes `g_rxsdt`.
 */
static err_t find_rxsdt(const struct acpi_rsdp* rsdp) {
  err_t err = SUCCESS;

  if (rsdp->revision > 1 && rsdp->xsdt_addr) {
    g_rxsdt = (struct acpi_rxsdt*)(uintptr_t)rsdp->xsdt_addr;
  } else {
    g_rxsdt = (struct acpi_rxsdt*)(uintptr_t)rsdp->rsdt_addr;
  }

  CHECK(g_rxsdt != NULL);

cleanup:
  return err;
}

/**
 * Returns whether the given RXSDT table is XSDT (otherwise it is the RSDT).
 */
static bool is_xsdt(const struct acpi_rxsdt* rxsdt) {
  return memcmp(rxsdt->hdr.signature, "XSDT", sizeof(rxsdt->hdr.signature)) == 0;
}

/**
 * Scans the RXSDT and locates additional tables, and initialzies their corresponding globals.
 * The RXSDT table has a dynamic size and contains pointers to additional tables at its end.
 *
 * Currently initializes the FADT table in `g_fadt`.
 */
static err_t scan_rxsdt(const struct acpi_rxsdt* rxsdt) {
  err_t err = SUCCESS;

  size_t entry_size = is_xsdt(rxsdt) ? sizeof(uint64_t) : sizeof(uint32_t);
  size_t entries_bytes = rxsdt->hdr.length - sizeof(*rxsdt);

  for (size_t i = 0; i + entry_size <= entries_bytes; i += entry_size) {
    uint64_t entry_addr = 0;
    memcpy(&entry_addr, &rxsdt->ptr_bytes[i], entry_size);
    struct acpi_table_header* entry = (struct acpi_table_header*)(uintptr_t)entry_addr;
    CHECK(entry != NULL);

    if (memcmp(entry->signature, "FACP", sizeof(entry->signature)) == 0) {
      CHECK_TRACE(g_fadt == NULL, "The FADT was found twice inside the RXSDT\n");
      g_fadt = (struct acpi_fadt*)entry;
    }
  }

cleanup:
  return err;
}

/**
 * Locates the FACS table in the FADT table and initializes `g_facs`.
 */
static err_t find_facs(const struct acpi_fadt* fadt) {
  err_t err = SUCCESS;

  if (fadt->x_firmware_ctrl != 0) {
    g_facs = (struct acpi_facs*)(uintptr_t)fadt->x_firmware_ctrl;
  } else {
    g_facs = (struct acpi_facs*)(uintptr_t)fadt->firmware_ctrl;
  }

cleanup:
  return err;
}

/**
 * Locates the DSDT table in the FADT table and initializes `g_dsdt`.
 */
static err_t find_dsdt(const struct acpi_fadt* fadt) {
  err_t err = SUCCESS;

  if (fadt->x_dsdt != 0) {
    g_dsdt = (struct acpi_dsdt*)(uintptr_t)fadt->x_dsdt;
  } else {
    g_dsdt = (struct acpi_dsdt*)(uintptr_t)fadt->dsdt;
  }

cleanup:
  return err;
}

err_t find_acpi_tables(void) {
  err_t err = SUCCESS;

  CHECK_RETHROW(find_rsdp());

  CHECK_RETHROW(find_rxsdt(g_rsdp));

  CHECK_RETHROW(scan_rxsdt(g_rxsdt));
  CHECK(g_fadt != NULL);

  CHECK_RETHROW(find_facs(g_fadt));

  CHECK_RETHROW(find_dsdt(g_fadt));

cleanup:
  return err;
}
