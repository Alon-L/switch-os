#include "core_header_utils.h"

#include <efidef.h>
#include <efilib.h>
#include <stddef.h>

#include "acpi/tables.h"
#include "core/header.h"
#include "pci.h"
#include "utils.h"

// TODO: This is currently hard coded to a virtio blk device. Make this
// configurable.
#define DISK_PCI_VENDOR_ID 0x1AF4
#define DISK_PCI_DEVICE_ID 0x1001

static const struct pci_dev_id g_disk_pci_id = {.vendor_id = DISK_PCI_VENDOR_ID, .device_id = DISK_PCI_DEVICE_ID};

extern struct core_header* g_core_header;

/**
 * Fills `g_core_header.rsdp` with `g_rsdp`.
 */
static void fill_rsdp(void) {
  g_core_header->rsdp = (uint64_t)(uintptr_t)g_rsdp;
}

/**
 * Fills `g_core_header.facs` with `g_facs`.
 */
static void fill_facs(void) {
  g_core_header->facs = (uint64_t)(uintptr_t)g_facs;
}

/**
 * Locates the disk by enumerating the PCI bus, and fills `g_core_header->disk_pci`
 * with the disk's PCI location, and the BIOS-initialized BARs.
 */
static err_t fill_disk_pci(void) {
  err_t err = SUCCESS;

  struct pci_dev disk_pci_dev = {0};
  CHECK_RETHROW(lookup_pci_dev(&disk_pci_dev, &g_disk_pci_id));

  // Fill the disk's pci bus information.
  g_core_header->disk_pci.addr.bus = disk_pci_dev.addr.bus;
  g_core_header->disk_pci.addr.device = disk_pci_dev.addr.device;
  g_core_header->disk_pci.addr.function = disk_pci_dev.addr.function;

  // Fill the disk's bars.
  struct pci_bar pci_bar = {0};
  for (size_t i = 0; i < 6; i++) {
    CHECK_RETHROW(pci_get_bar(&disk_pci_dev, i, &pci_bar));
    g_core_header->disk_pci.bars[i] = pci_bar.addr;
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
 * Inserts a memory descriptor into `g_core_header->ram_areas`.
 *
 * This also defragments consecutive descriptors so the table is as small as possible.
 */
static err_t insert_desc(const EFI_MEMORY_DESCRIPTOR* desc) {
  err_t err = SUCCESS;

  CHECK_TRACE(g_core_header->ram_areas_size < ARRAY_SIZE(g_core_header->ram_areas),
              "No space left for additional RAM areas!\n");

  size_t desc_size = desc->NumberOfPages * EFI_PAGE_SIZE;
  size_t desc_end = desc->PhysicalStart + desc_size;

  // Try to defragment the new descriptor into an existing consecutive descriptor.
  for (size_t i = 0; i < g_core_header->ram_areas_size; i++) {
    struct mem_area* area = &g_core_header->ram_areas[i];

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

  g_core_header->ram_areas[g_core_header->ram_areas_size].start = desc->PhysicalStart;
  g_core_header->ram_areas[g_core_header->ram_areas_size].size = desc_size;

  g_core_header->ram_areas_size++;

cleanup:
  return err;
}

/**
 * Locates all the usable memory RAM areas using the `GetMemoryMap` boot service, and fills `g_core_header->ram_areas`.
 */
static err_t fill_ram_areas(void) {
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

    CHECK_RETHROW(insert_desc(desc));
  }

cleanup:
  if (memory_map != NULL) {
    FreePool(memory_map);
  }
  return err;
}

err_t fill_core_header() {
  err_t err = SUCCESS;

  fill_rsdp();
  fill_facs();
  CHECK_RETHROW(fill_disk_pci());
  CHECK_RETHROW(fill_ram_areas());

cleanup:
  return err;
}
