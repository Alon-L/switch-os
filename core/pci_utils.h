#ifndef _PCI_UTILS_H
#define _PCI_UTILS_H

#include <stdint.h>

struct pci_dev_addr {
  uint8_t bus;
  uint8_t device;
  uint8_t function;
};

#define PCI_VENDOR_ID 0x00        // 16 bits
#define PCI_DEVICE_ID 0x02        // 16 bits
#define PCI_COMMAND 0x04          // 16 bits
#define PCI_STATUS 0x06           // 16 bits
#define PCI_BASE_ADDRESS_0 0x10   // 32 bits
#define PCI_BASE_ADDRESS_1 0x14   // 32 bits
#define PCI_BASE_ADDRESS_2 0x18   // 32 bits
#define PCI_BASE_ADDRESS_3 0x1c   // 32 bits
#define PCI_BASE_ADDRESS_4 0x20   // 32 bits
#define PCI_BASE_ADDRESS_5 0x24   // 32 bits
#define PCI_CAPABILITY_LIST 0x34  // 8 bits
#define PCI_CAPABILITY_PTR_OFFSET 1

/**
 * pci_read_{8,16,32} read values from the pci device's configuration space.
 * @param pci_dev   - The pci device
 * @param offset    - The offset to read from
 * @returns The value in the given offset for the pci device's configuration
 * space.
 *
 * pci_write_{8,16,32} write values to the pci device's configuration space.
 * @param pci_dev   - The pci device address
 * @param offset    - The offset to write to
 * @param value     - The value to write into the offset
 */
#define DECLARE_PCI_OP(num, type)                               \
  type pci_read_##num(const struct pci_dev_addr* pci_dev_addr,  \
                      uint32_t offset);                         \
  void pci_write_##num(const struct pci_dev_addr* pci_dev_addr, \
                       uint32_t offset, type value)

DECLARE_PCI_OP(8, uint8_t);
DECLARE_PCI_OP(16, uint16_t);
DECLARE_PCI_OP(32, uint32_t);
#undef DECLARE_PCI_OP

#endif
