#include "pci.h"

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

err_t find_first_pci_capability(const struct pci_dev* pci_dev,
                                uint8_t* first_cap_off_out) {
  err_t err = SUCCESS;

  uint8_t first_cap_ptr = pci_read_8(&pci_dev->addr, PCI_CAPABILITY_LIST);

  // The first 0x40 bytes of the configuration space contain reserved fields
  // which may not be capabilities.
  CHECK(first_cap_ptr >= 0x40);

  uint8_t next_cap_id = pci_read_8(&pci_dev->addr, first_cap_ptr);
  if (next_cap_id == 0xff) {
    // There are no capabilities.
    *first_cap_off_out = 0;
    goto cleanup;
  }

  *first_cap_off_out = first_cap_ptr;

cleanup:
  return err;
}

err_t find_next_pci_capability(const struct pci_dev* pci_dev,
                               uint8_t prev_cap_ptr_off,
                               uint8_t* next_cap_off_out) {
  err_t err = SUCCESS;

  uint8_t prev_cap_ptr =
    pci_read_8(&pci_dev->addr, prev_cap_ptr_off + PCI_CAPABILITY_PTR_OFFSET);

  // The first 0x40 bytes of the configuration space contain reserved fields
  // which may not be capabilities.
  CHECK(prev_cap_ptr >= 0x40);

  uint8_t next_cap_id = pci_read_8(&pci_dev->addr, prev_cap_ptr);
  if (next_cap_id == 0xff) {
    // We have reached the last capability in the capabilities list.
    *next_cap_off_out = 0;
    goto cleanup;
  }

  *next_cap_off_out = prev_cap_ptr;

cleanup:
  return err;
}
