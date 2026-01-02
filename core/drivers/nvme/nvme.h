#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "error.h"
#include "pci.h"

// NVMe Controller Registers (Memory-mapped)
#define NVME_REG_CAP 0x00     // Controller Capabilities
#define NVME_REG_VS 0x08      // Version
#define NVME_REG_INTMS 0x0C   // Interrupt Mask Set
#define NVME_REG_INTMC 0x10   // Interrupt Mask Clear
#define NVME_REG_CC 0x14      // Controller Configuration
#define NVME_REG_CSTS 0x1C    // Controller Status
#define NVME_REG_NSSR 0x20    // NVM Subsystem Reset
#define NVME_REG_AQA 0x24     // Admin Queue Attributes
#define NVME_REG_ASQ 0x28     // Admin Submission Queue Base Address
#define NVME_REG_ACQ 0x30     // Admin Completion Queue Base Address
#define NVME_REG_CMBLOC 0x38  // Controller Memory Buffer Location
#define NVME_REG_CMBSZ 0x3C   // Controller Memory Buffer Size
#define NVME_REG_DBS 0x1000   // Doorbase base

// Controller Capabilities (CAP) register bits
#define NVME_CAP_MQES(cap) ((cap) & 0xffff)
#define NVME_CAP_TIMEOUT(cap) (((cap) >> 24) & 0xff)
#define NVME_CAP_DSTRD(cap) (((cap) >> 32) & 0xf)
#define NVME_CAP_NSSRC(cap) (((cap) >> 36) & 0x1)
#define NVME_CAP_CSS(cap) (((cap) >> 37) & 0xff)
#define NVME_CAP_MPSMIN(cap) (((cap) >> 48) & 0xf)
#define NVME_CAP_MPSMAX(cap) (((cap) >> 52) & 0xf)
#define NVME_CAP_CMBS(cap) (((cap) >> 57) & 0x1)

#define NVME_CAP_CSS_NVM (1 << 0)
#define NVME_CAP_CSS_CSI (1 << 6)

// Controller Configuration (CC) register bits
#define NVME_CC_EN_SHIFT 0
#define NVME_CC_CSS_SHIFT 4
#define NVME_CC_MPS_SHIFT 7
#define NVME_CC_AMS_SHIFT 11
#define NVME_CC_SHN_SHIFT 14
#define NVME_CC_IOSQES_SHIFT 16
#define NVME_CC_IOCQES_SHIFT 20

#define NVME_CC_EN (1 << NVME_CC_EN_SHIFT)
#define NVME_CC_CSS_NVM (0 << NVME_CC_CSS_SHIFT)
#define NVME_CC_AMS_RR (0 << NVME_CC_AMS_SHIFT)

// Controller Status (CSTS) register bits
#define NVME_CSTS_RDY (1 << 0)            // Ready
#define NVME_CSTS_CFS (1 << 1)            // Controller Fatal Status
#define NVME_CSTS_SHST_MASK (3 << 2)      // Shutdown Status
#define NVME_CSTS_SHST_NORMAL (0 << 2)    // Normal operation
#define NVME_CSTS_SHST_PROGRESS (1 << 2)  // Shutdown in progress
#define NVME_CSTS_SHST_COMPLETE (2 << 2)  // Shutdown complete
#define NVME_CSTS_NSSRO (1 << 4)          // NVM Subsystem Reset Occurred
#define NVME_CSTS_PP (1 << 5)             // Processing Paused

// Admin Queue Attributes (AQA) register bits
#define NVME_AQA_ASQS_MASK 0xFFF          // Admin Submission Queue Size
#define NVME_AQA_ACQS_MASK (0xFFF << 16)  // Admin Completion Queue Size

// Queue sizes and limits
#define NVME_ADMIN_QUEUE_SIZE 64  // Admin queue entries
#define NVME_IO_QUEUE_SIZE 1024   // I/O queue entries
#define NVME_MAX_IO_QUEUES 16     // Maximum I/O queue pairs
#define NVME_QUEUE_ENTRY_SIZE 64  // Size of each queue entry

// NVMe Command Opcodes
#define NVME_ADMIN_DELETE_SQ 0x00     // Delete I/O Submission Queue
#define NVME_ADMIN_CREATE_SQ 0x01     // Create I/O Submission Queue
#define NVME_ADMIN_GET_LOG_PAGE 0x02  // Get Log Page
#define NVME_ADMIN_DELETE_CQ 0x04     // Delete I/O Completion Queue
#define NVME_ADMIN_CREATE_CQ 0x05     // Create I/O Completion Queue
#define NVME_ADMIN_IDENTIFY 0x06      // Identify
#define NVME_ADMIN_ABORT 0x08         // Abort
#define NVME_ADMIN_SET_FEATURES 0x09  // Set Features
#define NVME_ADMIN_GET_FEATURES 0x0A  // Get Features
#define NVME_ADMIN_ASYNC_EVENT 0x0C   // Asynchronous Event Request
#define NVME_ADMIN_FW_COMMIT 0x10     // Firmware Commit
#define NVME_ADMIN_FW_DOWNLOAD 0x11   // Firmware Image Download
#define NVME_ADMIN_FORMAT_NVM 0x80    // Format NVM

#define NVME_IO_FLUSH 0x00         // Flush
#define NVME_IO_WRITE 0x01         // Write
#define NVME_IO_READ 0x02          // Read
#define NVME_IO_WRITE_ZEROES 0x08  // Write Zeroes
#define NVME_IO_COMPARE 0x05       // Compare

// Status Code Types
#define NVME_SCT_GENERIC 0x0   // Generic Command Status
#define NVME_SCT_SPECIFIC 0x1  // Command Specific Status
#define NVME_SCT_MEDIA 0x2     // Media and Data Integrity Errors
#define NVME_SCT_VENDOR 0x7    // Vendor Specific

// Generic Status Codes
#define NVME_SC_SUCCESS 0x00          // Successful Completion
#define NVME_SC_INVALID_OPCODE 0x01   // Invalid Command Opcode
#define NVME_SC_INVALID_FIELD 0x02    // Invalid Field in Command
#define NVME_SC_ID_CONFLICT 0x03      // Command ID Conflict
#define NVME_SC_DATA_XFER_ERROR 0x04  // Data Transfer Error
#define NVME_SC_POWER_LOSS 0x05       // Commands Aborted due to Power Loss Notification
#define NVME_SC_INTERNAL 0x06         // Internal Error
#define NVME_SC_ABORT_REQ 0x07        // Command Abort Requested
#define NVME_SC_ABORT_QUEUE 0x08      // Command Aborted due to SQ Deletion
#define NVME_SC_FUSED_FAIL 0x09       // Command Aborted due to Failed Fused Command
#define NVME_SC_FUSED_MISSING 0x0A    // Command Aborted due to Missing Fused Command
#define NVME_SC_INVALID_NS 0x0B       // Invalid Namespace or Format
#define NVME_SC_CMD_SEQ_ERROR 0x0C    // Command Sequence Error

#define NVME_ADMIN_QUEUE_DEPTH 32
#define NVME_MAX_NAMESPACES 256

struct nvme_queue {
  struct nvme_command* sq_entries;
  struct nvme_completion* cq_entries;
  uint32_t* sq_doorbell;
  uint32_t* cq_doorbell;
  uint16_t sq_tail;
  uint16_t cq_head;
  uint16_t qid;
  uint16_t q_depth;
  uint8_t cq_phase;
};

struct nvme_id_ns {
  uint64_t nsze;
  uint64_t ncap;
  uint64_t nuse;
  uint8_t nsfeat;
  uint8_t nlbaf;
  uint8_t flbas;
  uint8_t mc;
  uint8_t dpc;
  uint8_t dps;
  uint8_t nmic;
  uint8_t rescap;
  uint8_t fpi;
  uint8_t dlfeat;
  uint16_t nawun;
  uint16_t nawupf;
  uint16_t nacwu;
  uint16_t nabsn;
  uint16_t nabo;
  uint16_t nabspf;
  uint16_t noiob;
  uint8_t nvmcap[16];
  uint16_t npwg;
  uint16_t npwa;
  uint16_t npdg;
  uint16_t npda;
  uint16_t nows;
  uint8_t rsvd74[18];
  uint32_t anagrpid;
  uint8_t rsvd96[3];
  uint8_t nsattr;
  uint16_t nvmsetid;
  uint16_t endgid;
  uint8_t nguid[16];
  uint8_t eui64[8];
  struct {
    uint16_t ms;
    uint8_t lbdas;
    uint8_t rp;
  } lbaf[64];
  uint8_t vs[3712];
};

struct nvme_namespace {
  uint32_t nsid;
  uint64_t size;
  uint16_t lba_shift;
  bool valid;
  struct nvme_id_ns* ns_data;
};

struct nvme_ctrl {
  struct pci_dev pci_dev;
  void* base_io;
  uint32_t stride;
  uint32_t page_size;
  uint16_t max_queue_entries;

  struct nvme_queue admin_queue;
  struct nvme_queue io_queue;

  struct nvme_namespace namespaces[NVME_MAX_NAMESPACES];
  uint32_t num_namespaces;

  uint16_t next_cmd_id;
};

struct nvme_command {
  uint8_t opcode;
  uint8_t flags;
  uint16_t command_id;
  uint32_t nsid;
  uint64_t rsvd;
  uint64_t metadata;
  uint64_t prp1;
  uint64_t prp2;
  uint32_t cdw10;
  uint32_t cdw11;
  uint32_t cdw12;
  uint32_t cdw13;
  uint32_t cdw14;
  uint32_t cdw15;
} __attribute__((packed));

struct nvme_completion {
  union nvme_result {
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
  } result;
  uint16_t sq_head;    /* how much of this queue may be reclaimed */
  uint16_t sq_id;      /* submission queue that generated this entry */
  uint16_t command_id; /* of the command which completed */
  uint16_t status;     /* did the command fail, and if so, why? */
} __attribute__((packed));

struct nvme_aqa {
  uint16_t reserved1 : 4;
  uint16_t acqs : 12;
  uint16_t reserved0 : 4;
  uint16_t asqs : 12;
};

err_t init_nvme_ctrl(struct nvme_ctrl* ctrl);
