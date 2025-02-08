#ifndef _INCLUDE_CORE_HEADER
#define _INCLUDE_CORE_HEADER

#include "core/consts.h"

#if defined(CORE)
#include <stdbool.h>
#include <stdint.h>
#elif defined(MODULE)
#include <linux/kernel.h>
#endif

struct mem_area {
  uint64_t start;
  uint64_t size;
};

typedef int (*core_start_t)(void);

struct core_header {
  // [READ]   A magic to validate the beginning of the core header. Must be
  //          `CORE_HEADER_MAGIC`.
  const uint32_t magic;

  // [WRITE]  The original waking vector of the kernel that entered core. The
  //          module must fill this.
  uint32_t original_waking_vector;

  // [WRITE]  The rsdp table's physical address. The module must fill this.
  uint64_t rsdp;

  // [WRITE]  Values used to find and restore the disk used for dumping and
  //          loading the memory. The module must fill this.
  struct {
    // The disk's pci address.
    struct {
      uint8_t bus;
      uint8_t device;
      uint8_t function;
    } addr;

    // The disk's pci bar values.
    // The device loses power when entering S3, and core receives it after a
    // reset. Therefore, it does not contain the bios's configured bars.
    // Similarly to the kernel, we restore these bars before starting
    // communication with the device.
    uint32_t bars[6];
  } disk_pci;

  // [WRITE]  All the memory areas listed as RAM. Core uses these areas to
  //          create the memory dump. The module must fill this.
  uint8_t ram_areas_size;
  struct mem_area ram_areas[32];
};

static inline bool is_core_header_magic_valid(struct core_header* core_header) {
  return core_header->magic == CORE_HEADER_MAGIC;
}

#endif
