#ifndef _VIRTIO_BLK_H
#define _VIRTIO_BLK_H

#include <stddef.h>

#include "error.h"
#include "pci.h"

#define VIRTIO_BLK_REQUESTED_FEATURES 0

struct virtio_queue {
  uint16_t num;
  uint16_t size;
  uint16_t seen_used;
  uint64_t notify_off;

#define VIRTIO_INVALID_FREE_HEAD 0xffff
  // The head of the free descriptors list. Every descriptor in this list is
  // linked to the next one using the `next` field.
  // The final free descriptor in the list holds `VIRTIO_INVALID_FREE_HEAD` in
  // the `next` field. If the head is `VIRTIO_INVALID_FREE_HEAD` then there are
  // no free descriptors.
  uint16_t free_head;

  struct virtq_desc* desc;
  struct virtq_avail* avail;
  struct virtq_used* used;
};

struct virtio_blk_dev {
  struct pci_dev pci_dev;

  struct virtio_pci_common_cfg* common_cfg;

  struct virtio_queue queue;

  // The offset and multiplier values stored in the notification capability.
  struct {
    uint64_t off;
    uint32_t multiplier;
  } notify;

  uint8_t status;
  uint64_t features;
};

/**
 * Initialize a virtio blk device and its queue.
 * @param virtio_blk_dev - The virtio blk device.
 */
err_t init_virtio_blk_dev(struct virtio_blk_dev* virtio_blk_dev);

/**
 * Read sector(s) from the virtio blk device.
 * @param virtio_blk-dev - The virtio blk device.
 * @param sector         - The sector to start reading from.
 * @param data           - A buffer which will be filled with the read data.
 * @param size           - The size of the data to read. Must be aligned to 512
 * bytes (sector size).
 */
err_t read_virtio_blk(struct virtio_blk_dev* virtio_blk_dev, uint64_t sector, uint8_t* data, size_t size);

/**
 * Write sector(s) to the virtio blk device.
 * @param virtio_blk-dev - The virtio blk device.
 * @param sector         - The sector to start writing to.
 * @param data           - A buffer which contains the data to write.
 * @param size           - The size of the data to write. Must be aligned to 512
 * bytes (sector size).
 */
err_t write_virtio_blk(struct virtio_blk_dev* virtio_blk_dev, uint64_t sector, const uint8_t* data, size_t size);

/**
 * Waits for a response, then pops and validates it.
 * The response is discarded.
 * This is used to receive an acknowledgment that a
 * request has completed successfully.
 */
err_t consume_response_virtio_blk(struct virtio_blk_dev* virtio_blk_dev);

// -----------------------------------------------
// ----- VIRTIO CONSTANTS FROM SPECIFICATION -----
// -----------------------------------------------

#define VIRTIO_BLK_VENDOR_ID 0x1AF4
#define VIRTIO_BLK_DEVICE_ID 0x1001

struct virtio_pci_cap {
  uint8_t cap_vndr;    // Generic PCI field: PCI_CAP_ID_VNDR
  uint8_t cap_next;    // Generic PCI field: next ptr.
  uint8_t cap_len;     // Generic PCI field: capability length
  uint8_t cfg_type;    // Identifies the structure.
  uint8_t bar;         // Where to find it.
  uint8_t id;          // Multiple capabilities of the same type
  uint8_t padding[2];  // Pad to full dword.
  uint32_t offset;     // Offset within bar.
  uint32_t length;     // Length of the structure, in bytes.
};

struct virtio_pci_notify_cap {
  struct virtio_pci_cap cap;
  uint32_t notify_off_multiplier;  // Multiplier for queue_notify_off.
};

#define VIRTIO_PCI_CAP_COMMON_CFG 1         // Common configuration
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2         // Notifications
#define VIRTIO_PCI_CAP_ISR_CFG 3            // ISR Status
#define VIRTIO_PCI_CAP_DEVICE_CFG 4         // Device specific configuration
#define VIRTIO_PCI_CAP_PCI_CFG 5            // PCI configuration access
#define VIRTIO_PCI_CAP_SHARED_MEMORY_CFG 8  // Shared memory region
#define VIRTIO_PCI_CAP_VENDOR_CFG 9         // Vendor-specific data
#define VIRTIO_PCI_CAP_VENDOR 0x09

struct virtio_pci_common_cfg {
  // About the whole device.
  uint32_t device_feature_select;  // read-write
  uint32_t device_feature;         // read-only for driver
  uint32_t driver_feature_select;  // read-write
  uint32_t driver_feature;         // read-write
  uint16_t config_msix_vector;     // read-write
  uint16_t num_queues;             // read-only for driver
  uint8_t device_status;           // read-write
  uint8_t config_generation;       // read-only for driver

  // About a specific virtqueue.
  uint16_t queue_select;       // read-write
  uint16_t queue_size;         // read-write
  uint16_t queue_msix_vector;  // read-write
  uint16_t queue_enable;       // read-write
  uint16_t queue_notify_off;   // read-only for driver
  uint32_t queue_desc_lo;      // read-write
  uint32_t queue_desc_hi;      // read-write
  uint32_t queue_driver_lo;    // read-write
  uint32_t queue_driver_hi;    // read-write
  uint32_t queue_device_lo;    // read-write
  uint32_t queue_device_hi;    // read-write
  uint16_t queue_notify_data;  // read-only for driver
  uint16_t queue_reset;        // read-write
} __attribute__((packed));

#define VIRTIO_STATUS_RESET 0
#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET 64
#define VIRTIO_STATUS_FAILED 128

struct virtq_desc {
  uint64_t addr;  // Address (guest-physical).
  uint32_t len;   // Length.

// This marks a buffer as continuing via the next field.
#define VIRTQ_DESC_F_NEXT 1
// This marks a buffer as device write-only (otherwise device read-only).
#define VIRTQ_DESC_F_WRITE 2
// This means the buffer contains a list of buffer descriptors.
#define VIRTQ_DESC_F_INDIRECT 4
  uint16_t flags;  // The flags as indicated above.
  uint16_t next;   // Next field if flags & NEXT
} __attribute__((packed));

struct virtq_avail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
  uint16_t flags;
  uint16_t idx;
  uint16_t ring[];
} __attribute__((packed));

struct virtq_used_elem {
  uint32_t id;  // Index of start of used descriptor chain.
  // The number of bytes written into the device writable portion of the buffer
  // described by the descriptor chain.
  uint32_t len;
} __attribute__((packed));

struct virtq_used {
#define VIRTQ_USED_F_NO_NOTIFY 1
  uint16_t flags;
  uint16_t idx;
  struct virtq_used_elem ring[];
} __attribute__((packed));

struct virtio_blk_req {
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
  // The data is technically here, but we chain 3 descriptors for every
  // request, and point the second descriptor to the data.
  char* data[0];
  uint8_t status;
} __attribute__((packed));

#define VIRTIO_BLK_REQ_HEADER_SIZE (offsetof(struct virtio_blk_req, status))
#define VIRTIO_BLK_REQ_FOOTER_SIZE (sizeof(struct virtio_blk_req) - VIRTIO_BLK_REQ_HEADER_SIZE)

#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1
#define VIRTIO_BLK_T_FLUSH 4

#define VIRTIO_BLK_S_OK 0
#define VIRTIO_BLK_S_IOERR 1
#define VIRTIO_BLK_S_UNSUPP 2

#define VIRTIO_INVALID_QUEUE_SIZE 0xffff

#define VIRTIO_F_INDIRECT_DESC 28
#define VIRTIO_F_IN_ORDER 35

#define VIRTIO_BLK_SECTOR_SIZE 512

#define VIRTIO_INVALID_DESC 0xffff

#endif
