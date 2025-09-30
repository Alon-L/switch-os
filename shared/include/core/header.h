#ifndef _INCLUDE_CORE_HEADER
#define _INCLUDE_CORE_HEADER

#include <stdbool.h>
#include <stdint.h>

#include "core/consts.h"
#include "mem_area.h"

enum core_action {
  CORE_ACTION_INVALID,
  CORE_ACTION_STORE,
  CORE_ACTION_SWITCH,

  CORE_ACTION_MAX,
};

#define MAX_RAM_AREAS 64

struct core_header {
  // [READ]   A magic to validate the beginning of the core header. Must be
  //          `CORE_HEADER_MAGIC`.
  const uint32_t magic;

  // [WRITE]  The action for core to execute. Must be filled to a
  //          value other than `CORE_ACTION_INVALID`.
  enum core_action action;

  // [WRITE]  The physical address of the ACPI FACS table. Must be filled.
  uint64_t facs;

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

static inline bool is_core_header_magic_valid(uint32_t magic) {
  return magic == CORE_HEADER_MAGIC;
}

static inline bool is_core_header_action_valid(enum core_action action) {
  return action != CORE_ACTION_INVALID && action < CORE_ACTION_MAX;
}

static inline bool is_core_header_facs_valid(uint64_t facs) {
  return facs != 0;
}

static inline bool is_core_header_rsdp_valid(uint64_t rsdp) {
  return rsdp != 0;
}

static inline bool is_core_header_ram_areas_valid(const struct mem_area* ram_areas, uint32_t ram_areas_size) {
  if (ram_areas_size > MAX_RAM_AREAS) {
    return false;
  }

  for (uint32_t i = 0; i < ram_areas_size; i++) {
    if (ram_areas[i].size == 0) {
      return false;
    }
  }

  return true;
}

static inline bool is_core_header_valid(const struct core_header* core_header) {
  return is_core_header_magic_valid(core_header->magic) && is_core_header_action_valid(core_header->action) &&
         is_core_header_facs_valid(core_header->facs) && is_core_header_rsdp_valid(core_header->rsdp) &&
         is_core_header_ram_areas_valid(core_header->ram_areas, core_header->ram_areas_size);
}

#endif
