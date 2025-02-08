#include "dump.h"

#include "core/consts.h"
#include "core/header.h"
#include "drivers/virtio/virtio_blk.h"
#include "drivers/virtio/virtio_queue.h"
#include "error.h"
#include "mem.h"
#include "utils.h"

extern struct core_header g_core_header;

struct disk_mem_area {
  struct mem_area area;
  uint32_t sector;
};

// The dump header is used to validate a dump's existence and organize it.
// It is always stored in sector 0 of the disk.
struct dump_header {
  // A magic to validate the beginning of a dump header. Must be
  // `CORE_DISK_DUMP_MAGIC`.
  uint32_t magic;

  // The kernel's waking vector. Once a dump is loaded back to memory, this
  // address will be used to wakeup the loaded kernel.
  uint32_t waking_vector;

  // The kernel's RAM areas.
  uint32_t areas_size;
  struct disk_mem_area areas[64];
} __attribute__((aligned(VIRTIO_BLK_SECTOR_SIZE)));

_Static_assert(sizeof(struct dump_header) % VIRTIO_BLK_SECTOR_SIZE == 0,
               "The dump header must be aligned to virtio blk sector size.");

// The number of sectors a dump header takes.
#define DUMP_HEADER_SECTORS_SIZE (sizeof(struct dump_header) / VIRTIO_BLK_SECTOR_SIZE)

/**
 * Read a dump header from the disk in sector 0.
 * Waits until receiving a response for the read.
 */
static err_t read_dump_header(struct virtio_blk_dev* virtio_blk_dev, struct dump_header* header_out) {
  err_t err = SUCCESS;

  CHECK_RETHROW(read_virtio_blk(virtio_blk_dev, 0, (void*)header_out, sizeof(struct dump_header)));

  CHECK_RETHROW(consume_response_virtio_blk(virtio_blk_dev));

cleanup:
  return err;
}

/**
 * Write a dump header to the disk in sector 0.
 * Waits until receiving a response for the write.
 */
static err_t write_dump_header(struct virtio_blk_dev* virtio_blk_dev, struct dump_header* header) {
  err_t err = SUCCESS;

  CHECK_RETHROW(write_virtio_blk(virtio_blk_dev, 0, (void*)header, sizeof(struct dump_header)));

  CHECK_RETHROW(consume_response_virtio_blk(virtio_blk_dev));

cleanup:
  return err;
}

/**
 * Fill a dump header with values from `g_core_header`.
 * This does not write the dump header to the disk.
 */
static err_t fill_dump_header(struct dump_header* header) {
  err_t err = SUCCESS;

  header->magic = CORE_DISK_DUMP_MAGIC;
  header->waking_vector = g_core_header.original_waking_vector;

  // Make sure all ram areas can fit inside the header.
  CHECK(g_core_header.ram_areas_size <= ARRAY_SIZE(header->areas));
  header->areas_size = g_core_header.ram_areas_size;

  // Make enough space for the header.
  size_t next_sector = DUMP_HEADER_SECTORS_SIZE;
  // Write the dump memory areas using `g_core_header.ram_areas`.
  for (size_t i = 0; i < g_core_header.ram_areas_size; i++) {
    const struct mem_area* area = &g_core_header.ram_areas[i];

    CHECK(area->size > 0);
    // Make sure the area is aligned to `VIRTIO_BLK_SECTOR_SIZE`.
    CHECK(area->size % VIRTIO_BLK_SECTOR_SIZE == 0);

    struct disk_mem_area* disk_area = &header->areas[i];
    disk_area->area.start = area->start;
    disk_area->area.size = area->size;
    disk_area->sector = next_sector;

    next_sector += area->size / VIRTIO_BLK_SECTOR_SIZE;
  }

cleanup:
  return err;
}

/**
 * Returns whether a dump header is valid.
 */
static bool is_dump_header_valid(const struct dump_header* header) {
  if (header->magic != CORE_DISK_DUMP_MAGIC) {
    return false;
  }

  if (header->waking_vector == 0) {
    return false;
  }

  if (header->areas_size == 0) {
    return false;
  }

  for (size_t i = 0; i < header->areas_size; i++) {
    // Every memory area in a dump must have a positive size.
    if (header->areas[i].area.size == 0) {
      return false;
    }
  }

  if (header->areas_size != g_core_header.ram_areas_size) {
    return false;
  }

  // The header's memory areas must match the RAM areas. This is required to make sure we safely load the dump into RAM
  // memory.
  for (size_t i = 0; i < header->areas_size; i++) {
    if (g_core_header.ram_areas[i].start != header->areas[i].area.start ||
        g_core_header.ram_areas[i].size != header->areas[i].area.size) {
      return false;
    }
  }

  return true;
}

err_t disk_store_dump(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;

  struct dump_header header;
  CHECK_RETHROW(fill_dump_header(&header));

  // Write all the memory areas to the disk.
  for (size_t i = 0; i < header.areas_size; i++) {
    struct disk_mem_area* disk_area = &header.areas[i];
    CHECK_RETHROW(
      write_virtio_blk(virtio_blk_dev, disk_area->sector, (void*)disk_area->area.start, disk_area->area.size));
  }

  // Validate the responses for the prior writes.
  for (size_t i = 0; i < header.areas_size; i++) {
    CHECK_RETHROW(consume_response_virtio_blk(virtio_blk_dev));
  }

  // Write the header last after all memory areas writes are complete, in order
  // to avoid corruption. The disk contains a dump only if a valid header
  // exists.
  CHECK_RETHROW(write_dump_header(virtio_blk_dev, &header));

cleanup:
  return err;
}

err_t does_disk_contain_dump(struct virtio_blk_dev* virtio_blk_dev, bool* contains_dump_out) {
  err_t err = SUCCESS;

  struct dump_header header;
  CHECK_RETHROW(read_dump_header(virtio_blk_dev, &header));

  *contains_dump_out = is_dump_header_valid(&header);

cleanup:
  return err;
}

err_t disk_load_dump(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;

  struct dump_header header;
  CHECK_RETHROW(read_dump_header(virtio_blk_dev, &header));
  CHECK(is_dump_header_valid(&header));

  // Read all the memory areas from the disk.
  // This reads directly into the RAM addresses the areas correspond to.
  for (size_t i = 0; i < header.areas_size; i++) {
    struct disk_mem_area* disk_area = &header.areas[i];

    // This read is safe, since the valid header's memory areas match the RAM areas. Therefore, this reads into RAM
    // memory.
    CHECK_RETHROW(
      read_virtio_blk(virtio_blk_dev, disk_area->sector, (void*)disk_area->area.start, disk_area->area.size));
  }

  // Validate the responses for the prior reads.
  for (size_t i = 0; i < header.areas_size; i++) {
    CHECK_RETHROW(consume_response_virtio_blk(virtio_blk_dev));
  }

  // TODO: Hold a different variable for the waking vector to return to, other
  // than using the header. So create a variable called `kernel_waking_vector`
  // and then set it to header.waking_vector and wakeup using it.
  g_core_header.original_waking_vector = header.waking_vector;

cleanup:
  return err;
}

#define SWITCH_BUF_SIZE (8 * 1024 * 1024)

/**
 * Switch a memory area on the disk with the memory in RAM.
 * This loads the memory area from the disk to the RAM, and stores the RAM back to the disk.
 */
static err_t switch_memory_area(struct virtio_blk_dev* virtio_blk_dev, const struct disk_mem_area* disk_area) {
  err_t err = SUCCESS;

  // The switch buffer serves as temporary storage for switching between the dump on the disk and the RAM.
  // It is very large for efficiency (the larger the buffer, the less requests required).
  // Our block allocator will struggle with a buffer this large, thus we statically allocate it.
  static __attribute__((aligned(4096))) uint8_t switch_buf[SWITCH_BUF_SIZE];
  _Static_assert(sizeof(switch_buf) % VIRTIO_BLK_SECTOR_SIZE == 0,
                 "Switch buffer size must be aligned with virtio blk sector size.");

  size_t end = disk_area->area.start + disk_area->area.size;
  size_t sector = disk_area->sector;
  for (size_t addr = disk_area->area.start; addr < end; addr += SWITCH_BUF_SIZE) {
    size_t read_size = MIN(sizeof(switch_buf), end - addr);
    CHECK(read_size % VIRTIO_BLK_SECTOR_SIZE == 0);

    // Read part of the memory from the disk into the switch buffer.
    CHECK_RETHROW(read_virtio_blk(virtio_blk_dev, sector, (void*)switch_buf, read_size));
    CHECK_RETHROW(consume_response_virtio_blk(virtio_blk_dev));
    // Write part of the the RAM to the disk.
    CHECK_RETHROW(write_virtio_blk(virtio_blk_dev, sector, (void*)addr, read_size));
    CHECK_RETHROW(consume_response_virtio_blk(virtio_blk_dev));
    // Now copy the memory from the temporary switch buffer to the RAM.
    memcpy((void*)addr, switch_buf, read_size);

    sector += read_size / VIRTIO_BLK_SECTOR_SIZE;
  }

cleanup:
  return err;
}

err_t disk_switch_dump(struct virtio_blk_dev* virtio_blk_dev) {
  err_t err = SUCCESS;

  struct dump_header header;
  CHECK_RETHROW(read_dump_header(virtio_blk_dev, &header));
  CHECK(is_dump_header_valid(&header));

  // Switch every memory area in the dump.
  for (size_t i = 0; i < header.areas_size; i++) {
    CHECK_RETHROW(switch_memory_area(virtio_blk_dev, &header.areas[i]));
  }

  // Switch the waking vector.
  uint32_t original_waking_vector = g_core_header.original_waking_vector;
  g_core_header.original_waking_vector = header.waking_vector;
  header.waking_vector = original_waking_vector;

  // Write the updated header (this is only required for the waking vector change).
  CHECK_RETHROW(write_dump_header(virtio_blk_dev, &header));

cleanup:
  return err;
}
