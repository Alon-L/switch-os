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

/**
 * Find the offset of the first capability in the pci device's capabilities
 * list.
 * @param pci_dev           - The pci device.
 * @param first_cap_off_out - The offset of the first capability. This receives
 * the value 0 if no first capability is found.
 */
err_t find_first_pci_capability(const struct pci_dev* pci_dev,
                                uint8_t* first_cap_off_out);

/**
 * Find the offset of the next capability in the pci device's capabilities
 * list.
 * @param pci_dev           - The pci device.
 * @param prev_cap_ptr_off  - The offset of the previous capability's pointer.
 * For example, the first capability pointer is the value stored in the
 * configuration space's capabilities pointer register. Therefore, the first
 * capability pointer offset is `PCI_CAPABILITIES_POINTER`.
 * @param next_cap_off_out  - The offset of the next capability. The given
 * previous capaability pointer points at this offset. This receives the value 0
 * if no next capability is found.
 */
err_t find_next_pci_capability(const struct pci_dev* pci_dev,
                               uint8_t prev_cap_ptr_off,
                               uint8_t* next_cap_off_out);

/**
 * Iterate over all the capabilities of a pci device.
 * This wraps the previous functions `find_first_pci_capability`,
 * `find_next_pci_capability` to a more user-friendly form.
 *
 * NOTE: The value of `err` contains whether the iteration errored. The user
 * must check this value when the iteration is complete.
 */
#define ITERATE_PCI_CAPABILITIES(pci_dev, cap_off_var)             \
  for (err = find_first_pci_capability((pci_dev), &(cap_off_var)); \
       (cap_off_var) != 0 && IS_SUCCESS(err);                      \
       err =                                                       \
         find_next_pci_capability((pci_dev), (cap_off_var), &(cap_off_var)))

#endif
