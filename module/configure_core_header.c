#include "configure_core_header.h"

#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/types.h>

#include "acpi.h"
#include "core/header.h"

extern struct core_header* g_core_header;

// TODO: This is currently hard coded to a virtio blk device. Make this
// configurable.
#define DISK_PCI_VENDOR_ID 0x1AF4
#define DISK_PCI_DEVICE_ID 0x1001

static err_t fill_disk_pci(void) {
  err_t err = SUCCESS;

  struct pci_dev* pci_dev = pci_get_device(DISK_PCI_VENDOR_ID, DISK_PCI_DEVICE_ID, NULL);
  CHECK(pci_dev != NULL);

  g_core_header->disk_pci.addr.bus = pci_dev->bus->number;
  g_core_header->disk_pci.addr.device = PCI_SLOT(pci_dev->devfn);
  g_core_header->disk_pci.addr.function = PCI_FUNC(pci_dev->devfn);

  if (pci_dev->state_saved) {
    // The device has alreay saved its state and most likely already disabled
    // power. Prior to suspending, the kernel calls `pci_save_state` on every
    // pci device, and stores its state in `pci_dev->saved_config_space`. Read
    // from there.
    for (size_t i = 0; i < ARRAY_SIZE(g_core_header->disk_pci.bars); i++) {
      g_core_header->disk_pci.bars[i] = pci_dev->saved_config_space[i + PCI_BASE_ADDRESS_0 / 4];
    }
  } else {
    // Store the bars directly from the pci device.
    for (size_t i = 0; i < ARRAY_SIZE(g_core_header->disk_pci.bars); i++) {
      pci_read_config_dword(pci_dev, PCI_BASE_ADDRESS_0 + 4 * i, &g_core_header->disk_pci.bars[i]);
    }
  }

cleanup:
  if (pci_dev != NULL) {
    pci_dev_put(pci_dev);
  }

  return err;
}

static err_t fill_original_waking_vector(void) {
  err_t err = SUCCESS;

  uint32_t waking_vector;
  CHECK_RETHROW(acpi_find_waking_vector(&waking_vector));
  g_core_header->original_waking_vector = waking_vector;

cleanup:
  return err;
}

static err_t fill_rsdp(void) {
  err_t err = SUCCESS;

  CHECK_RETHROW(acpi_find_rsdp(&g_core_header->rsdp));

cleanup:
  return err;
}

static int walk_iomem_cb(struct resource* res, void* arg) {
  // Validate the area has positive size.
  if (res->end <= res->start) {
    return -EINVAL;
  }
  size_t res_size = res->end + 1 - res->start;

  // Linux sometimes splits a memory area into several contiguous memory areas.
  // Defragment this split.
  if (g_core_header->ram_areas_size >= 1) {
    struct mem_area* prev =
      &g_core_header->ram_areas[g_core_header->ram_areas_size - 1];
    // Check if current area is a continuation of the previous area.
    if (prev->start + prev->size == res->start) {
      prev->size += res_size;
      return 0;
    }
  }

  if (g_core_header->ram_areas_size >= ARRAY_SIZE(g_core_header->ram_areas)) {
    return -ENOMEM;
  }

  struct mem_area* area =
    &g_core_header->ram_areas[g_core_header->ram_areas_size++];

  area->start = res->start;
  area->size = res_size;

  return 0;
}

static err_t fill_ram_areas(void) {
  err_t err = SUCCESS;

  g_core_header->ram_areas_size = 0;

  // Call `walk_iomem_cb` for every memory area of type "System RAM" (see
  // kernel function `walk_system_ram_res`).
  CHECK(walk_iomem_res_desc(IORES_DESC_NONE,
                            IORESOURCE_SYSTEM_RAM | IORESOURCE_BUSY, 0, -1,
                            NULL, &walk_iomem_cb) == 0);

cleanup:
  return err;
}

err_t configure_core_header(void) {
  err_t err = SUCCESS;

  CHECK(g_core_header != NULL);

  CHECK_RETHROW(fill_rsdp());
  CHECK_RETHROW(fill_disk_pci());
  CHECK_RETHROW(fill_original_waking_vector());
  CHECK_RETHROW(fill_ram_areas());

cleanup:
  return err;
}
