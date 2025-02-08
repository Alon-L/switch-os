#ifndef _DUMP_H
#define _DUMP_H

#include <stdbool.h>
#include <stdint.h>

#include "drivers/virtio/virtio_blk.h"
#include "error.h"

/**
 * Store a dump of the kernel's memory areas to the disk.
 * The dump contains a header, and the entirety of the RAM (excluding core).
 *
 * This function blocks until the dump is complete.
 */
err_t disk_store_dump(struct virtio_blk_dev* virtio_blk_dev);

/**
 * Returns whether the disk contains a valid dump.
 */
err_t does_disk_contain_dump(struct virtio_blk_dev* virtio_blk_dev,
                             bool* contains_dump_out);

/**
 * Loads a dump from the disk into the RAM.
 * This replaces all the current RAM areas with the areas stored in the dump.
 *
 * This function blocks until the dump is fully loaded.
 */
err_t disk_load_dump(struct virtio_blk_dev* virtio_blk_dev);

#endif
