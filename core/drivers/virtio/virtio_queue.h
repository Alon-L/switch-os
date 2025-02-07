#ifndef _VIRTIO_QUEUE_H
#define _VIRTIO_QUEUE_H

#include <stdbool.h>

#include "error.h"
#include "virtio_blk.h"

/**
 * Initialize the queue of a virtio blk device.
 * Valdiate the queue, allocate and configure the parts required for its split
 * virtqueue, and finally enable the queue.
 */
err_t init_virtio_blk_queue(struct virtio_blk_dev* virtio_blk_dev);

/**
 * Allocate a free descriptor.
 *
 * The queue contains a list of free descriptors that are chained together
 * using their `next` field. This returns the head of this list.
 *
 * @param free_desc_out - The newly allocated descriptor number.
 */
err_t alloc_queue_desc(struct virtio_queue* queue, uint16_t* free_desc_out);

/**
 * Free a previously allocated descriptor.
 *
 * Set the descriptor as the head of the free descriptors list, and chain it
 * to the previous head.
 */
void free_queue_desc(struct virtio_queue* queue, uint16_t desc);

/**
 * Returns whether the device has filled a new descriptor in the used ring that
 * we haven't looked at yet.
 */
bool is_new_used_virtio_blk(struct virtio_queue* queue);

/**
 * Pop a single used descriptor recently filled by the device.
 * The caller must check a new used descriptor exists prior to calling this,
 * using `is_new_used_virtio_blk`.
 *
 * @param desc_out  - The descriptor id of the popped descriptor,
 * @param len_out   - The length of data the device has written to the
 * descriptor.
 */
err_t pop_used_virtio_blk(struct virtio_queue* queue, uint16_t* desc_out,
                          uint32_t* len_out);

/**
 * Validate a response to a read/write request.
 *
 * The response includes 3 descriptors chained together.
 * The given length must equal the size of the write-only descriptors.
 *
 * @param desc  - The descriptor id of the first descriptor in the chain.
 * @param len   - The length of data the device has written to the descriptors.
 * This is obtained from `pop_used_virtio_blk`.
 */
err_t validate_response_virtio_blk(struct virtio_queue* queue, uint16_t desc,
                                   uint32_t len);

/**
 * Combines `pop_used_virtio_blk` and `validate_chain_virtio_blk` to pop and
 * validate a descriptor chain request.
 * This does not return the descriptors.
 */
err_t pop_and_validate_virtio_blk(struct virtio_queue* queue);

#endif
