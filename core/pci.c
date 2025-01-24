#include "pci.h"

#include <stdbool.h>
#include <stddef.h>

#include "pci_utils.h"

err_t lookup_pci_dev(struct pci_dev* pci_dev_out,
                     const struct pci_dev_lookup_req* req_mask) {
  err_t err = SUCCESS;

  // Iterate over all pci busses and try to find a pci device that matches the
  // mask.
  for (size_t bus = 0; bus < PCI_MAX_BUS; bus++) {
    for (size_t device = 0; device < PCI_MAX_DEVICE; device++) {
      for (size_t function = 0; function < PCI_MAX_FUNCTION; function++) {
        pci_dev_out->addr.bus = bus;
        pci_dev_out->addr.device = device;
        pci_dev_out->addr.function = function;

        uint16_t vendor_id = pci_read_16(&pci_dev_out->addr, PCI_VENDOR_ID);
        uint16_t device_id = pci_read_16(&pci_dev_out->addr, PCI_DEVICE_ID);

        if ((vendor_id & req_mask->vendor_id_mask) == vendor_id &&
            (device_id & req_mask->device_id_mask) == device_id) {
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
