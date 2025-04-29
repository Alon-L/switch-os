#pragma once

#include <stdint.h>

#include "error.h"
#include "pci_utils.h"

#ifndef PCI_MAX_BUS
#define PCI_MAX_BUS (256)
#endif

#ifndef PCI_MAX_DEVICE
#define PCI_MAX_DEVICE (32)
#endif

#ifndef PCI_MAX_FUNCTION
#define PCI_MAX_FUNCTION (8)
#endif

enum pci_bar_type {
  PCI_BAR_IO,
  PCI_BAR_MEMORY,
};

struct pci_bar {
  // The address is 32 bit except in cases of 64 bit memory bars. In this case,
  // the address is concatenated using the next bar.
  uint64_t addr;
  enum pci_bar_type type;
};

struct pci_dev {
  struct pci_dev_addr addr;
  uint16_t vendor_id;
  uint16_t device_id;
  uint8_t header_type;
};

struct pci_dev_id {
  uint16_t vendor_id;
  uint16_t device_id;
};

/**
 * Init the fields in the pci device.
 * @param pci_dev - The pci device. The struct is already expected to contain
 * the address.
 */
err_t init_pci_dev(struct pci_dev* pci_dev);

/**
 * Search for a pci device by enumerating the pci buses.
 * @param pci_dev       - An already allocated `pci_dev` to be filled with the
 * found pci device.
 * @param lookup_id     - The requested vendor ID and device ID for the pci
 * device. device.
 */
err_t lookup_pci_dev(struct pci_dev* pci_dev, const struct pci_dev_id* lookup_id);

/**
 * Returns a `struct pci_bar` that represents the given bar number.
 * @param pci_dev       - The pci device.
 * @param bar_num       - The bar number to retrieve.
 * @param pci_bar_out   - The pci bar struct for the given bar number.
 */
err_t pci_get_bar(const struct pci_dev* pci_dev, uint8_t bar_num, struct pci_bar* pci_bar_out);

struct pci_cap_iter {
  const struct pci_dev* pci_dev;
  uint8_t off;
};

/**
 * Initialize a pci capability iterator with the first capability.
 * @param pci_dev   - The pci device.
 * @param iter      - The iterator will be filled with the pci device,
 * and the offset of the first capability. The offset is set to 0 if the
 * capabilities list is empty.
 */
void pci_cap_iter_init(const struct pci_dev* pci_dev, struct pci_cap_iter* iter);

/**
 * Find next pci capability for the iterator.
 * @param iter      - The iterator will be filled with the offset of the next
 * capability in the list. The offset is set to 0 if no next capability exists.
 */
void pci_cap_iter_next(struct pci_cap_iter* iter);

/**
 * Iterate over all the capabilities of a pci device.
 */
#define ITERATE_PCI_CAPABILITIES(pci_dev, iter_var) \
  for (pci_cap_iter_init(&(pci_dev), &(iter_var)); (iter_var).off != 0; pci_cap_iter_next(&(iter_var)))
