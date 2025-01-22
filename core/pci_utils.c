#include "pci_utils.h"

#include "io.h"

#define PCI_CONF_ADDR(pci_dev_addr, offset)                                    \
  (0x80000000 | (((pci_dev_addr).bus) << 16) | ((pci_dev_addr).device << 11) | \
   ((pci_dev_addr).function << 8) | ((offset) & 0xFC))

#define DEFINE_PCI_OP(num, type)                                \
  type pci_read_##num(const struct pci_dev_addr* pci_dev_addr,  \
                      uint32_t offset) {                        \
    out32(0xCF8, PCI_CONF_ADDR(*pci_dev_addr, offset));         \
    return in##num(0xCFC);                                      \
  }                                                             \
                                                                \
  void pci_write_##num(const struct pci_dev_addr* pci_dev_addr, \
                       uint32_t offset, type value) {           \
    out32(0xCF8, PCI_CONF_ADDR(*pci_dev_addr, offset));         \
    return out##num(value, 0xCFC);                              \
  }

DEFINE_PCI_OP(8, uint8_t);
DEFINE_PCI_OP(16, uint16_t);
DEFINE_PCI_OP(32, uint32_t);

#undef PCI_CONF_ADDR
#undef DEFINE_PCI_OP
