
#include "virtual_address_map_event.h"

#include <efi.h>
#include <efilib.h>
#include <stdint.h>

#include "get_memory_map.h"
#include "services/headers.h"
#include "services/hooks_loader.h"
#include "set_variable.h"

#define EFI_EVENT_VIRTUAL_ADDRESS_CHANGE_GUID \
  {0x13fa7698, 0xc831, 0x49c7, 0x87, 0xea, 0x8f, 0x43, 0xfc, 0xc2, 0x51, 0x96}

DECLARE_HOOK_BINARY(virtual_address_map_event);

struct loaded_hook g_loaded_virtual_address_map_event;

static EFI_EVENT g_virtual_address_map_event_obj = NULL;

err_t create_virtual_address_map_event(void) {
  err_t err = SUCCESS;

  CHECK_TRACE(
    g_loaded_set_variable.is_loaded && g_loaded_get_memory_map.is_loaded,
    "The virtual address map event must be loaded after all other hooks, since it relocates their pointers\n");

  CHECK_RETHROW(load_hook(HOOK_START(virtual_address_map_event), HOOK_SIZE(virtual_address_map_event),
                          &g_loaded_virtual_address_map_event));

  *(struct virtual_address_map_event_header*)g_loaded_virtual_address_map_event.header =
    (struct virtual_address_map_event_header){
      .convert_pointer = gRT->ConvertPointer,
      // This array includes all absolute pointers which need to get relocated when virtual addresses are used.
      // In addition to these addresses, the convert pointer field above will be implicitly relocated as well.
      .ptrs_to_fix =
        {
          (void**)&((struct set_variable_hook_header*)g_loaded_set_variable.header)->original_set_variable,
          (void**)&((struct set_variable_hook_header*)g_loaded_set_variable.header)->waking_vector_addr,
        },
    };

  EFI_GUID efi_event_virtual_address_change_guid = EFI_EVENT_VIRTUAL_ADDRESS_CHANGE_GUID;
  CHECK(gBS->CreateEventEx(EVT_NOTIFY_SIGNAL, TPL_NOTIFY, (EFI_EVENT_NOTIFY)g_loaded_virtual_address_map_event.entry,
                           NULL, &efi_event_virtual_address_change_guid,
                           &g_virtual_address_map_event_obj) == EFI_SUCCESS);

cleanup:
  return err;
}

void remove_virtual_address_map_event(void) {
  if (g_loaded_virtual_address_map_event.is_loaded) {
    gBS->CloseEvent(&g_virtual_address_map_event_obj);

    free_hook(&g_loaded_virtual_address_map_event, HOOK_SIZE(virtual_address_map_event));
  }
}
