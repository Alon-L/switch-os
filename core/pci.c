#include "pci.h"

#include <stdbool.h>
#include <stddef.h>

#include "pci_utils.h"

/**
 * Init the rest of the fields in the pci device.
 * @param pci_dev - The pci device. The struct is already expected to contain
 * the address.
 */
static err_t init_pci_dev(struct pci_dev* pci_dev) {
  err_t err = SUCCESS;

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
