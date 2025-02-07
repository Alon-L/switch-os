#include "virtio_blk.h"

#include <stddef.h>
#include <stdint.h>

#include "alloc.h"
#include "core/header.h"
#include "io.h"
#include "mem.h"
#include "pci.h"
#include "pci_utils.h"
#include "virtio_queue.h"

extern struct core_header g_core_header;

/**
 * Iterate over the device's capabilities, which hold additional memory areas
 * the virtio block device uses, and fill them in `virtio_blk_dev`.
 *
 * The additional memory structs are found in the pci device's bars. Virtio
 * devices specify the location of these structs in the pci device's
 * capabilities list.
 *
 * @param virtio_blk_dev  - The virtio block device.
 */
static err_t parse_virtio_blk_caps(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;

  // Make sure the capability list is available.
  CHECK((pci_read_16(&virtio_blk_dev->pci_dev.addr, PCI_STATUS) &
         PCI_STATUS_CAP_LIST) != 0);

  struct pci_cap_iter iter;
  ITERATE_PCI_CAPABILITIES(virtio_blk_dev->pci_dev, iter) {
    // Every virtio capability vendor must be `0x09`.
    if (pci_read_8(&virtio_blk_dev->pci_dev.addr,
                   iter.off + offsetof(struct virtio_pci_cap, cap_vndr)) !=
        VIRTIO_PCI_CAP_VENDOR) {
      continue;
    }

    uint8_t cfg_type =
      pci_read_8(&virtio_blk_dev->pci_dev.addr,
                 iter.off + offsetof(struct virtio_pci_cap, cfg_type));

    // Validate the config type.
    switch (cfg_type) {
      case VIRTIO_PCI_CAP_COMMON_CFG:
      case VIRTIO_PCI_CAP_NOTIFY_CFG:
      case VIRTIO_PCI_CAP_ISR_CFG:
      case VIRTIO_PCI_CAP_DEVICE_CFG:
      case VIRTIO_PCI_CAP_PCI_CFG:
      case VIRTIO_PCI_CAP_SHARED_MEMORY_CFG:
      case VIRTIO_PCI_CAP_VENDOR_CFG:
        break;
      default: {
        // Invalid config type.
        CHECK_FAIL();
      }
    }

    uint8_t bar_num =
      pci_read_8(&virtio_blk_dev->pci_dev.addr,
                 iter.off + offsetof(struct virtio_pci_cap, bar));

    struct pci_bar pci_bar;
    CHECK_RETHROW(pci_get_bar(&virtio_blk_dev->pci_dev, bar_num, &pci_bar));

    switch (cfg_type) {
      case VIRTIO_PCI_CAP_COMMON_CFG: {
        // See section 4.1.4.3 in the virtio specs.
        // TODO: Support IO bars too
        CHECK(pci_bar.type == PCI_BAR_MEMORY);

        virtio_blk_dev->common_cfg =
          (struct virtio_pci_common_cfg*)pci_bar.addr;
        break;
      }
      case VIRTIO_PCI_CAP_NOTIFY_CFG: {
        // See section 4.1.4.4 in the virtio specs.
        // The notify capability holds the constants to derive the queue notify
        // address: `bar_addr + cap.offset + multiplier * queue_notify_off`
        // (`queue_notify_off` is stored in the common configuration, and is
        // different for every queue).
        // TODO: Support IO bars too
        CHECK(pci_bar.type == PCI_BAR_MEMORY);

        uint32_t offset =
          pci_read_32(&virtio_blk_dev->pci_dev.addr,
                      iter.off + offsetof(struct virtio_pci_cap, offset));
        virtio_blk_dev->notify.off = pci_bar.addr + offset;

        virtio_blk_dev->notify.multiplier =
          pci_read_32(&virtio_blk_dev->pci_dev.addr,
                      iter.off + offsetof(struct virtio_pci_notify_cap,
                                          notify_off_multiplier));
        break;
      }
    }
  }

cleanup:
  return err;
}

/**
 * Initializes a virtio's device status by resetting it, and then setting
 * `VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER`. This is the first thing a
 * driver should perform on the device. Updates the `virtio_blk_dev`'s status to
 * the updated status.
 */
static void init_virtio_status(struct virtio_blk_dev* virtio_blk_dev) {
  uint8_t status = VIRTIO_STATUS_RESET;
  write_mb8(&virtio_blk_dev->common_cfg->device_status, status);
  status |= VIRTIO_STATUS_ACKNOWLEDGE;
  write_mb8(&virtio_blk_dev->common_cfg->device_status, status);
  status |= VIRTIO_STATUS_DRIVER;
  write_mb8(&virtio_blk_dev->common_cfg->device_status, status);

  virtio_blk_dev->status = status;
}

/**
 * Perform the feature negotiation against the virtio device.
 * Updates the `virtio_blk_dev`'s features to the negotiated features.
 */
static err_t negotiate_virtio_features(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;
  uint64_t requested_features = VIRTIO_BLK_REQUESTED_FEATURES;

  // Query the available device features.
  write_mb32(&virtio_blk_dev->common_cfg->device_feature_select, 0);
  uint64_t available_features =
    read_mb32(&virtio_blk_dev->common_cfg->device_feature);

  write_mb32(&virtio_blk_dev->common_cfg->device_feature_select, 1);
  available_features |=
    (uint64_t)read_mb32(&virtio_blk_dev->common_cfg->device_feature) >> 32;

  // Make sure the device offers all of our requested features.
  CHECK((requested_features & available_features) == requested_features);

  // Set the requested features.
  write_mb32(&virtio_blk_dev->common_cfg->driver_feature_select, 0);
  write_mb32(&virtio_blk_dev->common_cfg->driver_feature,
             (uint32_t)requested_features);
  write_mb32(&virtio_blk_dev->common_cfg->driver_feature_select, 1);
  write_mb32(&virtio_blk_dev->common_cfg->driver_feature,
             (uint32_t)(requested_features >> 32));

  // Update the device status.
  virtio_blk_dev->status |= VIRTIO_STATUS_FEATURES_OK;
  write_mb8(&virtio_blk_dev->common_cfg->device_status, virtio_blk_dev->status);

  // Make sure the device has accepted our requested features.
  CHECK(read8(&virtio_blk_dev->common_cfg->device_status) &
        VIRTIO_STATUS_FEATURES_OK);

  virtio_blk_dev->features = requested_features;

cleanup:
  return err;
}

err_t init_virtio_blk_dev(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;

  // We expect module to fill the device's pci address for us.
  virtio_blk_dev->pci_dev.addr.bus = g_core_header.disk_pci.addr.bus;
  virtio_blk_dev->pci_dev.addr.device = g_core_header.disk_pci.addr.device;
  virtio_blk_dev->pci_dev.addr.function = g_core_header.disk_pci.addr.function;

  // Initialize the rest of the pci device.
  CHECK_RETHROW(init_pci_dev(&virtio_blk_dev->pci_dev));
  CHECK(virtio_blk_dev->pci_dev.vendor_id == VIRTIO_BLK_VENDOR_ID &&
        virtio_blk_dev->pci_dev.device_id == VIRTIO_BLK_DEVICE_ID);

  // Restore the device's bars using the bars module filled for us.
  // This is crucial since the device loses power when entering S3 suspend, and
  // loses the bar values.
  // The BIOS is the one initially responsible for configuring the bars for the
  // devices, and the kernel is responsible for restoring the bars after
  // returning from S3. Since we run before the kernel returns from S3, we
  // have to restore the bars ourselves.
  for (size_t i = 0; i < PCI_BASE_ADDRESS_NUM; i++) {
    pci_write_32(&virtio_blk_dev->pci_dev.addr, PCI_BASE_ADDRESS_0 + 4 * i,
                 g_core_header.disk_pci.bars[i]);
  }

  // We expect the device to have a normal header.
  CHECK(virtio_blk_dev->pci_dev.header_type == PCI_HEADER_TYPE_NORMAL);

  // Find the additional memory areas for the device.
  CHECK_RETHROW(parse_virtio_blk_caps(virtio_blk_dev));
  // We require the common cfg and notify cfg.
  CHECK(virtio_blk_dev->common_cfg != NULL && virtio_blk_dev->notify.off != 0);

  // Enable memory communication with the device.
  // TODO: Once we support IO bars as well, enable `PCI_COMMAND_IO`.
  uint16_t command = pci_read_16(&virtio_blk_dev->pci_dev.addr, PCI_COMMAND);
  pci_write_16(&virtio_blk_dev->pci_dev.addr, PCI_COMMAND,
               command | PCI_COMMAND_MEMORY);

  init_virtio_status(virtio_blk_dev);

  // Negotiate the device's features.
  CHECK_RETHROW(negotiate_virtio_features(virtio_blk_dev));

  // Virtio blk devices only have a single queue.
  virtio_blk_dev->queue.num = 0;
  CHECK_RETHROW(init_virtio_blk_queue(virtio_blk_dev));

  virtio_blk_dev->status |= VIRTIO_STATUS_DRIVER_OK;
  write_mb8(&virtio_blk_dev->common_cfg->device_status, virtio_blk_dev->status);

cleanup:
  if (IS_ERROR(err)) {
    if (virtio_blk_dev->common_cfg != NULL) {
      write16(&virtio_blk_dev->common_cfg->device_status, VIRTIO_STATUS_FAILED);
    }
  }

  return err;
}

/**
 * Create and send a request to the virtio device.
 *
 * A request consists of 3 descriptors which together make up a `virtio_blk_req`
 * with 512 bytes of data.
 * The virtio specification states that descriptors may be either read-only or
 * write-only. Therefore, we have to split our request to 3 descriptors:
 * 1. The read-only header which contains the type and sector.
 * 2. The 512 bytes of data, which are read-only on for write requests, or
 * write-only for read requests.
 * 3. The write-only footer which contains the request status. This is filled by
 * the device when the request is placed in the used ring.
 *
 * @param virtio_blk_dev - The virtio block device.
 * @param request_type   - Either `VIRTIO_BLK_T_IN` for read requests, or
 * `VIRTIO_BLK_T_OUT` for write requests.
 * @param sector         - The sector number to read/write to.
 * @param data           - The data buffer to read from / write to.
 * @param size           - The size of the data buffer. A request can read/write
 * more than a single sector, but the data must be aligned to 512-bytes (sector
 * size).
 */
static err_t request_virtio_blk(struct virtio_blk_dev* virtio_blk_dev,
                                uint32_t request_type, uint64_t sector,
                                uint8_t* data, size_t size) {
  err_t err = SUCCESS;
  uint16_t desc1 = VIRTIO_INVALID_DESC;
  uint16_t desc2 = VIRTIO_INVALID_DESC;
  uint16_t desc3 = VIRTIO_INVALID_DESC;
  struct virtio_blk_req* header = NULL;

  CHECK(size % VIRTIO_BLK_SECTOR_SIZE == 0);

  struct virtio_queue* queue = &virtio_blk_dev->queue;

  header = core_calloc(sizeof(struct virtio_blk_req), 1);
  CHECK(header != NULL);

  header->type = request_type;
  header->sector = sector;
  header->status = 0;

  // Allocate the 3 descriptors needed for the request.
  CHECK_RETHROW(alloc_queue_desc(queue, &desc1));
  CHECK_RETHROW(alloc_queue_desc(queue, &desc2));
  CHECK_RETHROW(alloc_queue_desc(queue, &desc3));

  // The first descriptor is read-only and contains the type and
  // sector. It is chained to the second descriptor.
  queue->desc[desc1].addr = (uint64_t)header;
  queue->desc[desc1].len = VIRTIO_BLK_REQ_HEADER_SIZE;
  queue->desc[desc1].flags = VIRTQ_DESC_F_NEXT;
  queue->desc[desc1].next = desc2;

  // The second descriptor contains the data. It is chained to the third
  // descriptor.
  queue->desc[desc2].addr = (uint64_t)data;
  queue->desc[desc2].len = size;
  queue->desc[desc2].flags = VIRTQ_DESC_F_NEXT;
  queue->desc[desc2].next = desc3;
  // The second descriptor is read-only for write requests, or write-only for
  // read requests.
  if (request_type == VIRTIO_BLK_T_IN) {
    queue->desc[desc2].flags |= VIRTQ_DESC_F_WRITE;
  }

  // The third descriptor contains the request status, which is filled by the
  // device when placed into the used ring.
  queue->desc[desc3].addr = (uint64_t)header + VIRTIO_BLK_REQ_HEADER_SIZE;
  queue->desc[desc3].len = VIRTIO_BLK_REQ_FOOTER_SIZE;
  queue->desc[desc3].flags = VIRTQ_DESC_F_WRITE;

  // Place the descriptor chain in the available ring, and increase its index.
  uint16_t avail_idx = read16(&queue->avail->idx);
  write_mb16(&queue->avail->ring[avail_idx % queue->size], desc1);
  write_mb16(&queue->avail->idx, avail_idx + 1);

  // Notify the device of the new descriptors.
  write_mb16((void*)(queue->notify_off), queue->num);

cleanup:
  if (err != SUCCESS) {
    core_free(header);
    free_queue_desc(&virtio_blk_dev->queue, desc1);
    free_queue_desc(&virtio_blk_dev->queue, desc2);
    free_queue_desc(&virtio_blk_dev->queue, desc3);
  }

  return err;
}

err_t read_virtio_blk(struct virtio_blk_dev* virtio_blk_dev, uint64_t sector,
                      uint8_t* data, size_t size) {
  err_t err = SUCCESS;

  CHECK_RETHROW(
    request_virtio_blk(virtio_blk_dev, VIRTIO_BLK_T_IN, sector, data, size));

cleanup:
  return err;
}

err_t write_virtio_blk(struct virtio_blk_dev* virtio_blk_dev, uint64_t sector,
                       uint8_t* data, size_t size) {
  err_t err = SUCCESS;

  CHECK_RETHROW(
    request_virtio_blk(virtio_blk_dev, VIRTIO_BLK_T_OUT, sector, data, size));

cleanup:
  return err;
}
