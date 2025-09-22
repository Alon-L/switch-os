#include "get_memory_map.h"

#include <efi.h>
#include <efilib.h>
#include <stdint.h>

#include "acpi/tables.h"
#include "hooks/headers.h"
#include "hooks/hooks_loader.h"

DECLARE_HOOK_BINARY(get_memory_map);

struct loaded_hook g_loaded_get_memory_map;

static EFI_GET_MEMORY_MAP g_original_get_memory_map = NULL;

err_t hook_get_memory_map(void) {
  err_t err = SUCCESS;

  CHECK_RETHROW(load_hook(HOOK_START(get_memory_map), HOOK_SIZE(get_memory_map), &g_loaded_get_memory_map));

  *(struct get_memory_map_hook_header*)g_loaded_get_memory_map.header = (struct get_memory_map_hook_header){
    .original_get_memory_map = gBS->GetMemoryMap,
    .waking_vector_phys_addr = (uintptr_t)&g_facs->firmware_waking_vector,
    .install_configuration_table = gBS->InstallConfigurationTable,
  };

  g_original_get_memory_map = gBS->GetMemoryMap;

  gBS->GetMemoryMap = (EFI_GET_MEMORY_MAP)g_loaded_get_memory_map.entry;
  SetCrc(&gBS->Hdr);

cleanup:
  return err;
}

void unhook_get_memory_map(void) {
  if (g_loaded_get_memory_map.is_loaded) {
    gBS->GetMemoryMap = g_original_get_memory_map;
    SetCrc(&gBS->Hdr);

    free_hook(&g_loaded_get_memory_map, HOOK_SIZE(get_memory_map));
  }
}
