#ifndef _INCLUDE_CORE_HEADER
#define _INCLUDE_CORE_HEADER

#include "core/consts.h"

#if defined(CORE) || defined(UEFI)
#include <stdbool.h>
#include <stdint.h>
#elif defined(MODULE)
#include <linux/kernel.h>
#endif

#include "utils.h"

struct mem_area {
  uint64_t start;
  uint64_t size;
};

enum core_action {
  CORE_ACTION_STORE,
  CORE_ACTION_SWITCH,
  CORE_ACTION_INVALID,
};

#define MAX_RAM_AREAS 64

struct core_header {
  // [READ]   A magic to validate the beginning of the core header. Must be
  //          `CORE_HEADER_MAGIC`.
  const uint32_t magic;

  // [WRITE]  The action for core to execute. Must be filled to a
  //          value other than `CORE_ACTION_INVALID`.
  enum core_action action;

  // [WRITE]  The original waking vector of the kernel that entered core. Must be
  //          filled.
  uint32_t original_waking_vector;

  // [WRITE]  The rsdp table's physical address. Must be filled.
  uint64_t rsdp;

  // [WRITE]  Values used to find and restore the disk used for dumping and
  //          loading the memory. Must be filled.
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

  // [WRITE]  All the memory areas listed as RAM which can be used by the OS.
  //          Core uses these areas to create the memory dump. Must be filled.
  uint32_t ram_areas_size;
  struct mem_area ram_areas[MAX_RAM_AREAS];
};

static inline bool is_core_header_magic_valid(const struct core_header* core_header) {
  return core_header->magic == CORE_HEADER_MAGIC;
}

static inline bool is_core_header_action_valid(const struct core_header* core_header) {
  return core_header->action < CORE_ACTION_INVALID;
}

static inline bool is_core_header_original_waking_vector_valid(const struct core_header* core_header) {
  return core_header->original_waking_vector != 0;
}

static inline bool is_core_header_rsdp_valid(const struct core_header* core_header) {
  return core_header->rsdp != 0;
}

static inline bool is_core_header_ram_areas_valid(const struct core_header* core_header) {
  if (core_header->ram_areas_size > ARRAY_SIZE(core_header->ram_areas)) {
    return false;
  }

  for (uint32_t i = 0; i < core_header->ram_areas_size; i++) {
    if (core_header->ram_areas[i].size == 0) {
      return false;
    }
  }

  return true;
}

static inline bool is_core_header_valid(const struct core_header* core_header) {
  return is_core_header_magic_valid(core_header) && is_core_header_action_valid(core_header) &&
         is_core_header_original_waking_vector_valid(core_header) && is_core_header_rsdp_valid(core_header) &&
         is_core_header_ram_areas_valid(core_header);
}

#endif
