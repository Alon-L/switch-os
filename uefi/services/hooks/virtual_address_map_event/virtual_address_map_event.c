#include <efi.h>
#include <stddef.h>

#include "../headers.h"
#include "error.h"
#include "trace.h"
#include "utils.h"

__attribute__((section(".header"))) struct virtual_address_map_event_header g_hook_header = {};

VOID EFIAPI _start(IN EFI_EVENT Event, IN VOID* Context) {
  err_t err = SUCCESS;

  TRACE("Virtual address map event. Replacing physical pointers\n");

  for (size_t i = 0; i < ARRAY_SIZE(g_hook_header.ptrs_to_fix); i++) {
    if (g_hook_header.ptrs_to_fix[i] == NULL) {
      break;
    }

    // The pointers in `ptrs_to_fix` point to addresses in the loaded hooks' headers. These addresses store pointers to
    // physical addresses.
    // We need to relocate these physical addresses, and also our pointers in `ptrs_to_fix` to the hooks' headers, since
    // they are also physical addresses.
    CHECK(g_hook_header.convert_pointer(0, (void**)g_hook_header.ptrs_to_fix[i]) == EFI_SUCCESS);
    CHECK(g_hook_header.convert_pointer(0, (void**)&g_hook_header.ptrs_to_fix[i]) == EFI_SUCCESS);
  }

  // Relocate our pointer to `convert_pointer`. Technically this is unnecessary, since `VirtualAddressMap` can only be
  // called once, but we do it for good measure.
  CHECK(g_hook_header.convert_pointer(0, (void**)&g_hook_header.convert_pointer) == EFI_SUCCESS);

cleanup:
  return;
}
