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
 * Validates whether a capability pointer is valid, and returns whether it
 * points to the end of the capabilities list.
 * @param cap_ptr     - A pointer to a capability. This contains an offset in
 * the configuration space to another capability.
 * @param is_end_out  - Whether the capability that `cap_ptr` is part of is the
 * end of the capabilities list.
 */
static err_t validate_capability_ptr(const struct pci_dev* pci_dev,
                                     uint8_t cap_ptr, bool* is_end_out) {
  err_t err = SUCCESS;

  // The first 0x40 bytes of the configuration space contain reserved fields
  // which may not be capabilities.
  CHECK(cap_ptr >= 0x40);

  uint8_t next_cap_id = pci_read_8(&pci_dev->addr, cap_ptr);
  if (next_cap_id == 0xff) {
    // The capability `cap_ptr` points to is invalid. Therefore the capability
    // that `cap_ptr` is part of is the final capability.
    *is_end_out = true;
  } else {
    *is_end_out = false;
  }

cleanup:
  return err;
}

err_t find_first_pci_capability(const struct pci_dev* pci_dev,
                                uint8_t* first_cap_off_out) {
  err_t err = SUCCESS;

  uint8_t first_cap_ptr = pci_read_8(&pci_dev->addr, PCI_CAPABILITY_LIST);
  bool is_end = false;
  CHECK_RETHROW(validate_capability_ptr(pci_dev, first_cap_ptr, &is_end));

  if (is_end) {
    *first_cap_off_out = 0;
  } else {
    *first_cap_off_out = first_cap_ptr;
  }

cleanup:
  return err;
}

err_t find_next_pci_capability(const struct pci_dev* pci_dev,
                               uint8_t prev_cap_ptr_off,
                               uint8_t* next_cap_off_out) {
  err_t err = SUCCESS;

  uint8_t prev_cap_ptr =
    pci_read_8(&pci_dev->addr, prev_cap_ptr_off + PCI_CAPABILITY_PTR_OFFSET);
  bool is_end = false;
  CHECK_RETHROW(validate_capability_ptr(pci_dev, prev_cap_ptr, &is_end));

  if (is_end) {
    *next_cap_off_out = 0;
  } else {
    *next_cap_off_out = prev_cap_ptr;
  }

cleanup:
  return err;
}
