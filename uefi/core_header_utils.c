#include "core_header_utils.h"

#include <efidef.h>
#include <efilib.h>

#include "pci.h"

// TODO: This is currently hard coded to a virtio blk device. Make this
// configurable.
#define DISK_PCI_VENDOR_ID 0x1AF4
#define DISK_PCI_DEVICE_ID 0x1001

#define QEMU_NVME_VENDOR_ID 0x1b36
#define QEMU_NVME_DEVICE_ID 0x0010

static const struct pci_dev_id g_disk_pci_id = {.vendor_id = QEMU_NVME_VENDOR_ID, .device_id = QEMU_NVME_DEVICE_ID};

/**
 * Locates the RSDP in the EFI SystemTable, and fills `core_header.rsdp`.
 */
static err_t fill_rsdp(struct core_header* core_header) {
  err_t err = SUCCESS;
  static EFI_GUID acpi_20_table_guid = ACPI_20_TABLE_GUID;
  void* rsdp = NULL;

  CHECK(ST->ConfigurationTable != NULL);

  for (size_t i = 0; i < ST->NumberOfTableEntries; i++) {
    EFI_CONFIGURATION_TABLE* table = &ST->ConfigurationTable[i];

    if (CompareGuid(&table->VendorGuid, &acpi_20_table_guid) == 0) {
      // The RSDP should only appear once.
      CHECK(rsdp == NULL);
      rsdp = table->VendorTable;
    }
  }

  CHECK_TRACE(rsdp != NULL, "Unable to find the RSDP\n");
  core_header->rsdp = (uint64_t)(uintptr_t)rsdp;

cleanup:
  return err;
}

/**
 * Locates the disk by enumerating the PCI bus, and fills `core_header->disk_pci`
 * with the disk's PCI location, and the BIOS-initialized BARs.
 */
static err_t fill_disk_pci(struct core_header* core_header) {
  err_t err = SUCCESS;

  struct pci_dev disk_pci_dev = {0};
  CHECK_RETHROW(lookup_pci_dev(&disk_pci_dev, &g_disk_pci_id));

  // Fill the disk's pci bus information.
  core_header->disk_pci.addr.bus = disk_pci_dev.addr.bus;
  core_header->disk_pci.addr.device = disk_pci_dev.addr.device;
  core_header->disk_pci.addr.function = disk_pci_dev.addr.function;

  // Fill the disk's bars.
  struct pci_bar pci_bar = {0};
  for (size_t i = 0; i < 6; i++) {
    CHECK_RETHROW(pci_get_bar(&disk_pci_dev, i, &pci_bar));
    core_header->disk_pci.bars[i] = pci_bar.addr;
  }

cleanup:
  return err;
}

/**
 * According to the UEFI specs (table 7.6 in section 7.2), all of the following memory types can be used
 * by the OS as RAM after full initialization.
 */
static const EFI_MEMORY_TYPE g_usable_memory_types[] = {
  EfiLoaderCode,         EfiLoaderData,        EfiBootServicesCode, EfiBootServicesData,
  EfiConventionalMemory, EfiACPIReclaimMemory, EfiPersistentMemory,
};

/**
 * Returns whether a given memory descriptor can be used by the OS as RAM.
 */
static bool is_memory_desc_usable(const EFI_MEMORY_DESCRIPTOR* desc) {
  for (size_t i = 0; i < ARRAY_SIZE(g_usable_memory_types); i++) {
    if (desc->Type == g_usable_memory_types[i]) {
      return true;
    }
  }
  return false;
}

/**
 * Inserts a memory descriptor into `core_header->ram_areas`.
 *
 * This also defragments consecutive descriptors so the table is as small as possible.
 */
static err_t insert_desc(struct core_header* core_header, const EFI_MEMORY_DESCRIPTOR* desc) {
  err_t err = SUCCESS;

  CHECK_TRACE(core_header->ram_areas_size < ARRAY_SIZE(core_header->ram_areas),
              "No space left for additional RAM areas!\n");

  size_t desc_size = desc->NumberOfPages * EFI_PAGE_SIZE;
  size_t desc_end = desc->PhysicalStart + desc_size;

  // Try to defragment the new descriptor into an existing consecutive descriptor.
  for (size_t i = 0; i < core_header->ram_areas_size; i++) {
    struct mem_area* area = &core_header->ram_areas[i];

    if (area->start == desc_end) {
      // An existing descriptor begins where the new descriptor ends.
      area->start = desc->PhysicalStart;
      goto cleanup;
    } else if (area->start + area->size == desc->PhysicalStart) {
      // An existing descriptor ends where the new descriptor starts.
      area->size += desc_size;
      goto cleanup;
    }
  }

  core_header->ram_areas[core_header->ram_areas_size].start = desc->PhysicalStart;
  core_header->ram_areas[core_header->ram_areas_size].size = desc_size;

  core_header->ram_areas_size++;

cleanup:
  return err;
}

/**
 * Locates all the usable memory RAM areas using the `GetMemoryMap` boot service, and fills `core_header->ram_areas`.
 */
static err_t fill_ram_areas(struct core_header* core_header) {
  err_t err = SUCCESS;
  EFI_MEMORY_DESCRIPTOR* memory_map = NULL;

  size_t memory_map_size = 0;
  size_t map_key = 0;
  size_t desc_size = 0;
  uint32_t desc_version = 0;

  // Pass a zero `memory_map_size` to obtain the required memory map size.
  CHECK(uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size, memory_map, &map_key, &desc_size, &desc_version) ==
        EFI_BUFFER_TOO_SMALL);

  // The memory map size might have changed after calling `GetMemoryMap`. Extend the buffer to make sure it fits.
  memory_map_size += EFI_PAGE_SIZE;
  memory_map = AllocatePool(memory_map_size);
  CHECK(memory_map != NULL);

  // Obtain the memory map.
  CHECK(uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size, memory_map, &map_key, &desc_size, &desc_version) ==
        EFI_SUCCESS);

  // Iterate over the memory map and find all the usable RAM areas.
  size_t ram_areas_size = 0;
  for (size_t i = 0; i + desc_size <= memory_map_size; i += desc_size) {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((void*)memory_map + i);
    if (!is_memory_desc_usable(desc)) {
      continue;
    }

    CHECK_RETHROW(insert_desc(core_header, desc));
  }

cleanup:
  if (memory_map != NULL) {
    FreePool(memory_map);
  }
  return err;
}

err_t fill_core_header(struct core_header* core_header) {
  err_t err = SUCCESS;

  CHECK_RETHROW(fill_rsdp(core_header));
  CHECK_RETHROW(fill_disk_pci(core_header));
  CHECK_RETHROW(fill_ram_areas(core_header));

cleanup:
  return err;
}
