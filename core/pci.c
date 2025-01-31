#include "pci.h"

#include <stdbool.h>
#include <stddef.h>

#include "pci_utils.h"

err_t init_pci_dev(struct pci_dev* pci_dev) {
  err_t err = SUCCESS;

  pci_dev->vendor_id = pci_read_16(&pci_dev->addr, PCI_VENDOR_ID);
  pci_dev->device_id = pci_read_16(&pci_dev->addr, PCI_DEVICE_ID);

  pci_dev->header_type =
    pci_read_8(&pci_dev->addr, PCI_HEADER_TYPE) & PCI_HEADER_TYPE_MASK;
  CHECK(pci_dev->header_type == PCI_HEADER_TYPE_NORMAL ||
        pci_dev->header_type == PCI_HEADER_TYPE_BRIDGE ||
        pci_dev->header_type == PCI_HEADER_TYPE_CARDBUS);

cleanup:
  return err;
}

err_t lookup_pci_dev(struct pci_dev* pci_dev,
                     const struct pci_dev_id* lookup_id) {
  err_t err = SUCCESS;

  // Iterate over all pci busses and try to find a pci device that matches the
  // requested vendor and device IDs.
  for (size_t bus = 0; bus < PCI_MAX_BUS; bus++) {
    for (size_t device = 0; device < PCI_MAX_DEVICE; device++) {
      for (size_t function = 0; function < PCI_MAX_FUNCTION; function++) {
        pci_dev->addr.bus = bus;
        pci_dev->addr.device = device;
        pci_dev->addr.function = function;

        uint16_t vendor_id = pci_read_16(&pci_dev->addr, PCI_VENDOR_ID);
        if (vendor_id == PCI_VENDOR_ID_INVALID) {
          // Invalid device.
          break;
        }
        uint16_t device_id = pci_read_16(&pci_dev->addr, PCI_DEVICE_ID);

        if (lookup_id->vendor_id == vendor_id &&
            lookup_id->device_id == device_id) {
          CHECK_RETHROW(init_pci_dev(pci_dev));
          goto cleanup;
        }
      }
    }
  }

  // No matching pci device was found.
  CHECK_FAIL();

cleanup:
  return err;
}

err_t pci_get_bar(const struct pci_dev* pci_dev, uint8_t bar_num,
                  struct pci_bar* pci_bar_out) {
  err_t err = SUCCESS;

  CHECK(bar_num < PCI_BASE_ADDRESS_NUM);

  uint32_t bar = pci_read_32(&pci_dev->addr, PCI_BASE_ADDRESS_0 + bar_num * 4);
  CHECK(bar != PCI_BASE_ADDRESS_INVALID);

  switch (bar & PCI_BASE_ADDRESS_SPACE) {
    case PCI_BASE_ADDRESS_SPACE_IO: {
      pci_bar_out->type = PCI_BAR_IO;
      pci_bar_out->addr = bar & PCI_BASE_ADDRESS_IO_MASK;
      break;
    }
    case PCI_BASE_ADDRESS_SPACE_MEMORY: {
      pci_bar_out->type = PCI_BAR_MEMORY;
      pci_bar_out->addr = bar & PCI_BASE_ADDRESS_MEM_MASK;
      break;
    }
    default: {
      // Invalid bar type
      CHECK_FAIL();
    }
  }

  // Concatenate this bar with the next one if this is a 64 bit memory bar.
  if (pci_bar_out->type == PCI_BAR_MEMORY &&
      (bar & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_64) {
    CHECK(bar_num + 1 < PCI_BASE_ADDRESS_NUM);

    uint32_t bar_next =
      pci_read_32(&pci_dev->addr, PCI_BASE_ADDRESS_0 + (bar_num + 1) * 4);

    pci_bar_out->addr += (uint64_t)bar_next << 32;
  }

cleanup:
  return err;
}

/**
 * Returns whether a pci capability is valid.
 * @param pci_dev   - The pci device.
 * @param cap_off   - The offset of the potential pci capability
 */
static bool is_cap_valid(const struct pci_dev* pci_dev, uint8_t cap_off) {
  // The first 0x40 bytes of the configuration space contain reserved fields
  // which aren't capabilities.
  if (cap_off < 0x40) {
    return false;
  }

  uint8_t cap_id = pci_read_8(&pci_dev->addr, cap_off);
  if (cap_id == 0xff) {
    // The capability is invalid.
    return false;
  }

  return true;
}

void pci_cap_iter_init(const struct pci_dev* pci_dev,
                       struct pci_cap_iter* iter) {
  iter->pci_dev = pci_dev;

  uint8_t first_cap_off = pci_read_8(&pci_dev->addr, PCI_CAPABILITY_LIST);
  if (is_cap_valid(pci_dev, first_cap_off)) {
    iter->off = first_cap_off;
  } else {
    iter->off = 0;
  }
}

void pci_cap_iter_next(struct pci_cap_iter* iter) {
  // The next capability pointer field inside a capability has an offset of
  // `PCI_CAPABILITY_PTR_OFFSET`.
  uint8_t next_cap_off =
    pci_read_8(&iter->pci_dev->addr, iter->off + PCI_CAPABILITY_PTR_OFFSET);

  if (is_cap_valid(iter->pci_dev, next_cap_off)) {
    iter->off = next_cap_off;
  } else {
    iter->off = 0;
  }
}
