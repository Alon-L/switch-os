#include "virtio_queue.h"

#include "alloc.h"
#include "drivers/virtio/virtio_blk.h"
#include "error.h"
#include "io.h"

/**
 * Set the pointers of the parts of the split virtqueue in the device's common
 * configuration.
 */
static void configure_queue_ptrs(struct virtio_pci_common_cfg* common_cfg, struct virtio_queue* queue) {
  write_mb16(&common_cfg->queue_select, queue->num);

  write32(&common_cfg->queue_desc_lo, (uint32_t)(uintptr_t)queue->desc);
  write32(&common_cfg->queue_desc_hi, (uint32_t)((uintptr_t)queue->desc >> 32));

  write32(&common_cfg->queue_driver_lo, (uint32_t)(uintptr_t)queue->avail);
  write32(&common_cfg->queue_driver_hi, (uint32_t)((uintptr_t)queue->avail >> 32));

  write32(&common_cfg->queue_device_lo, (uint32_t)(uintptr_t)queue->used);
  write32(&common_cfg->queue_device_hi, (uint32_t)((uintptr_t)queue->used >> 32));

  mb();
}

/**
 * Initialize the offset used to notify the queue of new descriptors in the
 * available queue. The offset is calculated using the off and multiplier values
 * found in the notification capability.
 */
static err_t init_queue_notify_off(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;

  CHECK(virtio_blk_dev->notify.off != 0);

  uint16_t queue_notify_off = read16(&virtio_blk_dev->common_cfg->queue_notify_off);

  virtio_blk_dev->queue.notify_off = virtio_blk_dev->notify.off + queue_notify_off * virtio_blk_dev->notify.multiplier;

cleanup:
  return err;
}

err_t init_virtio_blk_queue(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;
  struct virtio_queue* queue = &virtio_blk_dev->queue;

  // Select the queue.
  write_mb16(&virtio_blk_dev->common_cfg->queue_select, queue->num);

  // Query and validate the queue size.
  uint16_t queue_size = read16(&virtio_blk_dev->common_cfg->queue_size);
  CHECK(queue_size != VIRTIO_INVALID_QUEUE_SIZE && queue_size > 0);
  queue->size = queue_size;

  // Allocate the split virtqueue parts.
  queue->desc = core_calloc(sizeof(struct virtq_desc), queue_size);
  CHECK(queue->desc != NULL);
  queue->avail = core_calloc(sizeof(struct virtq_avail) + sizeof(uint16_t) * queue_size, 1);
  CHECK(queue->avail != NULL);
  queue->used = core_calloc(sizeof(struct virtq_used) + sizeof(struct virtq_used_elem) * queue_size, 1);
  CHECK(queue->used != NULL);

  // We don't have interrupts enabled, and we wish to not get notifications
  // (interrupts) from the device when it is finished with a read/write request.
  queue->avail->flags |= VIRTQ_AVAIL_F_NO_INTERRUPT;

  // Make sure all allocations above are finished.
  mb();

  // Set the desc, avail, and used pointers in the device's common configuration
  // to reflect the new queue.
  configure_queue_ptrs(virtio_blk_dev->common_cfg, queue);

  // Every unused descriptor points at the next unused descriptor. This creates
  // a linked list of all the unused descriptors.
  // When a descriptor is freed, it becomes the head of the free descriptors
  // list, and is chained to the the previous head.
  // See `alloc_queue_desc` and `free_queue_desc`.
  for (size_t i = 0; i < queue_size; i++) {
    queue->desc[i].next = i + 1;
  }
  queue->desc[queue_size - 1].next = VIRTIO_INVALID_FREE_HEAD;
  queue->free_head = 0;

  queue->seen_used = 0;

  CHECK_RETHROW(init_queue_notify_off(virtio_blk_dev));

  // Finally enable the queue.
  write_mb16(&virtio_blk_dev->common_cfg->queue_enable, 1);

cleanup:
  if (err != SUCCESS) {
    core_free(queue->desc);
    core_free(queue->avail);
    core_free(queue->used);
  }

  return err;
}

err_t alloc_queue_desc(struct virtio_queue* queue, uint16_t* free_desc_out) {
  err_t err = SUCCESS;
  uint16_t free_head = queue->free_head;

  // Check there are free descriptors to be used.
  CHECK_TRACE(free_head != VIRTIO_INVALID_FREE_HEAD,
              "Unable to allocate a descriptor, since the descriptors queue is full!");

  // Update the head of the free descriptors list.
  queue->free_head = queue->desc[free_head].next;

  *free_desc_out = free_head;

cleanup:
  return err;
}

void free_queue_desc(struct virtio_queue* queue, uint16_t desc) {
  // Check weather the descriptor is valid.
  if (desc == VIRTIO_INVALID_DESC || desc >= queue->size) {
    return;
  }

  uint16_t prev_free_head = queue->free_head;
  queue->desc[desc].next = prev_free_head;

  queue->free_head = desc;
}

bool is_unseen_used_virtio_queue(struct virtio_queue* queue) {
  return queue->seen_used != read16(&queue->used->idx);
}

err_t pop_used_virtio_queue(struct virtio_queue* queue, uint16_t* desc_out, uint32_t* len_out) {
  err_t err = SUCCESS;

  CHECK(is_unseen_used_virtio_queue(queue));

  *desc_out = (uint16_t)read32(&queue->used->ring[queue->seen_used % queue->size].id);
  *len_out = read32(&queue->used->ring[queue->seen_used % queue->size].len);

  queue->seen_used++;

cleanup:
  return err;
}
