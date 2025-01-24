#ifndef _PCI_H
#define _PCI_H

#include <stdint.h>

#include "error.h"
#include "pci_utils.h"

#define PCI_MAX_BUS (256)
#define PCI_MAX_DEVICE (32)
#define PCI_MAX_FUNCTION (8)

struct pci_dev {
  struct pci_dev_addr addr;
};

struct pci_dev_lookup_req {
  uint32_t vendor_id_mask;
  uint32_t device_id_mask;
};

/**
 * Search for a pci device by enumerating the pci buses.
 * @param pci_dev_out   - An allocated `pci_dev` to be filled with the found pci
 * device.
 * @param req_mas       - A mask for the vendor id and device id of the searched
 * device.
 */
err_t lookup_pci_dev(struct pci_dev* pci_dev_out,
                     const struct pci_dev_lookup_req* req_mask);

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
void pci_cap_iter_init(const struct pci_dev* pci_dev,
                       struct pci_cap_iter* iter);

/**
 * Find next pci capability for the iterator.
 * @param iter      - The iterator will be filled with the offset of the next
 * capability in the list. The offset is set to 0 if no next capability exists.
 */
void pci_cap_iter_next(struct pci_cap_iter* iter);

/**
 * Iterate over all the capabilities of a pci device.
 */
#define ITERATE_PCI_CAPABILITIES(pci_dev, iter_var)                    \
  for (pci_cap_iter_init((pci_dev), &(iter_var)); (iter_var).off != 0; \
       pci_cap_iter_next(&(iter_var)))

#endif
