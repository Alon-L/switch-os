#ifndef _HOOKS_HEADERS
#define _HOOKS_HEADERS

#include <efi.h>

#include "core/header.h"
#include "mem_area.h"

struct set_variable_hook_header {
  EFI_SET_VARIABLE original_set_variable;
  uint32_t* waking_vector_addr;
  uint32_t* original_waking_vector_addr;
  struct core_header* core_header;
};

struct get_memory_map_hook_header {
  EFI_GET_MEMORY_MAP original_get_memory_map;
  EFI_INSTALL_CONFIGURATION_TABLE install_configuration_table;
  struct mem_area runtime_areas[32];
};

struct virtual_address_map_event_header {
  EFI_CONVERT_POINTER convert_pointer;
  void** ptrs_to_fix[32];
};

#endif
