#include "virtio_blk.h"

#include <stddef.h>
#include <stdint.h>

#include "alloc.h"
#include "core/header.h"
#include "io.h"
#include "mem.h"
#include "pci.h"
#include "pci_utils.h"

extern struct core_header g_core_header;

/**
 * Find the additional memory the virtio block device uses, and fill it in
 * `virtio_blk_dev`.
 * The additional memory structs are found in the pci device's bars. Virtio
 * devices specify the location of these structs in the pci device's
 * capabilities list. We iterate over the capabilities list and find the structs
 * we care for.
 * @param virtio_blk_dev  - The virtio block device.
 */
static err_t init_virtio_blk_dev_structs(
  struct virtio_blk_dev* virtio_blk_dev) {
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
        // Store the common cfg struct.
        // TODO: Support IO bars too
        CHECK(pci_bar.type == PCI_BAR_MEMORY);

        virtio_blk_dev->common_cfg =
          (struct virtio_pci_common_cfg*)pci_bar.addr;

        break;
      }
      case VIRTIO_PCI_CAP_NOTIFY_CFG: {
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
 * Initializes a virtio's device status. This is the first thing a driver should
 * perform on the device.
 * Updates the `virtio_blk_dev`'s status to the updated status.
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
 * Perform the feature negotiation against the virtio device. In our case, we
 * don't require any features.
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

/**
 * Set the pointers of the parts of the split virtqueue in the device's common
 * configuration.
 */
static void configure_virtio_blk_queue_ptrs(
  struct virtio_pci_common_cfg* common_cfg, struct virtio_queue* queue) {
  write_mb16(&common_cfg->queue_select, queue->num);

  write32(&common_cfg->queue_desc_lo, (uint32_t)(uintptr_t)queue->desc);
  write32(&common_cfg->queue_desc_hi, (uint32_t)((uintptr_t)queue->desc >> 32));

  write32(&common_cfg->queue_driver_lo, (uint32_t)(uintptr_t)queue->avail);
  write32(&common_cfg->queue_driver_hi,
          (uint32_t)((uintptr_t)queue->avail >> 32));

  write32(&common_cfg->queue_device_lo, (uint32_t)(uintptr_t)queue->used);
  write32(&common_cfg->queue_device_hi,
          (uint32_t)((uintptr_t)queue->used >> 32));

  mb();
}

/**
 * Initialize the queue of a virtio blk device.
 * Valdiate the queue, allocate and configure the parts required for its split
 * virtqueue, and finally enable the queue.
 */
static err_t init_virtio_blk_queue(struct virtio_pci_common_cfg* common_cfg,
                                   struct virtio_queue* queue) {
  err_t err = SUCCESS;

  write_mb16(&common_cfg->queue_select, queue->num);
  uint16_t queue_size = read16(&common_cfg->queue_size);
  CHECK(queue_size != VIRTIO_INVALID_QUEUE_SIZE);

  queue->size = queue_size;

  // Allocate the split virtqueue parts.
  queue->desc = core_calloc(sizeof(struct virtq_desc), queue_size);
  CHECK(queue->desc != NULL);
  queue->avail =
    core_calloc(sizeof(struct virtq_avail) + sizeof(uint16_t) * queue_size, 1);
  CHECK(queue->avail != NULL);
  queue->used = core_calloc(
    sizeof(struct virtq_used) + sizeof(struct virtq_used_elem) * queue_size, 1);
  CHECK(queue->used != NULL);

  // We don't have interrupts enabled, and we wish to not get notifications
  // (interrupts) from the device when it is finished with a read/write request.
  queue->avail->flags |= VIRTQ_AVAIL_F_NO_INTERRUPT;

  // Make sure all allocations above are finished.
  mb();

  // Set the desc, avail, and used pointers in the device's common configuration
  // to reflect the new queue.
  configure_virtio_blk_queue_ptrs(common_cfg, queue);

  // Finally enable the queue.
  write_mb16(&common_cfg->queue_enable, 1);

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

  // Find the additional memory structs for the device.
  CHECK_RETHROW(init_virtio_blk_dev_structs(virtio_blk_dev));
  // We require the common cfg and notify cfg.
  CHECK(virtio_blk_dev->common_cfg != NULL && virtio_blk_dev->notify.off != 0);

  // Enable memory communication with the device.
  // TODO: Once we support IO bars as well, enable `PCI_COMMAND_IO`.
  uint16_t command = pci_read_16(&virtio_blk_dev->pci_dev.addr, PCI_COMMAND);
  pci_write_16(&virtio_blk_dev->pci_dev.addr, PCI_COMMAND,
               command | PCI_COMMAND_MEMORY);

  init_virtio_status(virtio_blk_dev);

  // Negotiate the device's features. In our case, we don't require any
  // features.
  CHECK_RETHROW(negotiate_virtio_features(virtio_blk_dev));

  // Virtio blk devices only have a single queue.
  virtio_blk_dev->queue.num = 0;
  CHECK_RETHROW(
    init_virtio_blk_queue(virtio_blk_dev->common_cfg, &virtio_blk_dev->queue));

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
