#pragma once

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
#define PCI_HEADER_TYPE 0x0e      // 8 bits
#define PCI_BASE_ADDRESS_0 0x10   // 32 bits
#define PCI_BASE_ADDRESS_1 0x14   // 32 bits
#define PCI_BASE_ADDRESS_2 0x18   // 32 bits
#define PCI_BASE_ADDRESS_3 0x1c   // 32 bits
#define PCI_BASE_ADDRESS_4 0x20   // 32 bits
#define PCI_BASE_ADDRESS_5 0x24   // 32 bits
#define PCI_CAPABILITY_LIST 0x34  // 8 bits

#define PCI_CAPABILITY_PTR_OFFSET 1

#define PCI_HEADER_TYPE_MASK 0x7f
#define PCI_HEADER_TYPE_NORMAL 0
#define PCI_HEADER_TYPE_BRIDGE 1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_STATUS_IMM_READY 0x01     // Immediate Readiness
#define PCI_STATUS_INTERRUPT 0x08     // Interrupt status
#define PCI_STATUS_CAP_LIST 0x10      // Support Capability List
#define PCI_STATUS_66MHZ 0x20         // Support 66 MHz PCI 2.1 bus
#define PCI_STATUS_UDF 0x40           // Support User Definable Features [obsolete]
#define PCI_STATUS_FAST_BACK 0x80     // Accept fast-back to back
#define PCI_STATUS_PARITY 0x100       // Detected parity error
#define PCI_STATUS_DEVSEL_MASK 0x600  // DEVSEL timing
#define PCI_STATUS_DEVSEL_FAST 0x000
#define PCI_STATUS_DEVSEL_MEDIUM 0x200
#define PCI_STATUS_DEVSEL_SLOW 0x400
#define PCI_STATUS_SIG_TARGET_ABORT 0x800   // Set on target abort
#define PCI_STATUS_REC_TARGET_ABORT 0x1000  // Master ack of "
#define PCI_STATUS_REC_MASTER_ABORT 0x2000  // Set on master abort
#define PCI_STATUS_SIG_SYSTEM_ERROR 0x4000  // Set when we drive SERR
#define PCI_STATUS_DETECTED_PARITY 0x8000   // Set on parity error

#define PCI_VENDOR_ID_INVALID 0xffff

#define PCI_BASE_ADDRESS_NUM 0x6
#define PCI_BASE_ADDRESS_INVALID 0xffffffff
#define PCI_BASE_ADDRESS_SPACE 0x01
#define PCI_BASE_ADDRESS_SPACE_IO 0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define PCI_BASE_ADDRESS_MEM_TYPE_32 0x00
#define PCI_BASE_ADDRESS_MEM_TYPE_1M 0x02
#define PCI_BASE_ADDRESS_MEM_TYPE_64 0x04
#define PCI_BASE_ADDRESS_MEM_MASK (~0x0fUL)
#define PCI_BASE_ADDRESS_IO_MASK (~0x03UL)

#define PCI_COMMAND_IO 0x1      // Enable response in I/O space
#define PCI_COMMAND_MEMORY 0x2  // Enable response in Memory space

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
#define DECLARE_PCI_OP(num, type)                                                \
  type pci_read_##num(const struct pci_dev_addr* pci_dev_addr, uint32_t offset); \
  void pci_write_##num(const struct pci_dev_addr* pci_dev_addr, uint32_t offset, type value)

DECLARE_PCI_OP(8, uint8_t);
DECLARE_PCI_OP(16, uint16_t);
DECLARE_PCI_OP(32, uint32_t);
#undef DECLARE_PCI_OP
