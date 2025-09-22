#include "hooks_loader.h"

#include <efi.h>
#include <efilib.h>

#include "efiapi.h"
#include "loaders/get_memory_map.h"
#include "loaders/set_variable.h"
#include "loaders/virtual_address_map_event.h"
#include "offsets.h"
#include "utils.h"

err_t load_hook(const void* hook_start, size_t hook_size, struct loaded_hook* loaded_hook_out) {
  err_t err = SUCCESS;

  CHECK_TRACE(!loaded_hook_out->is_loaded, "Hook is already loaded\n");

  uintptr_t hook_addr = 0;
  size_t hook_pages = ALIGN_UP(hook_size, EFI_PAGE_SIZE) / EFI_PAGE_SIZE;
  CHECK(uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesCode, hook_pages, &hook_addr) ==
        EFI_SUCCESS);

  __builtin_memcpy((void*)hook_addr, hook_start, hook_size);

  loaded_hook_out->header = (void*)hook_addr + RAW_HOOK_HEADER_OFFSET;
  loaded_hook_out->entry = (void*)hook_addr + RAW_HOOK_ENTRY_OFFSET;
  loaded_hook_out->is_loaded = true;

cleanup:
  return err;
}

void free_hook(struct loaded_hook* loaded_hook, size_t hook_size) {
  if (!loaded_hook->is_loaded) {
    return;
  }

  size_t hook_pages = ALIGN_UP(hook_size, EFI_PAGE_SIZE) / EFI_PAGE_SIZE;
  size_t hook_header_pages = HOOK_HEADER_SIZE / EFI_PAGE_SIZE;

  // The hook's data and code were allocated separately, and thus need to be freed separately.
  // See `allocate_hook_memory`.
  uefi_call_wrapper(BS->FreePages, 2, (uintptr_t)loaded_hook->header, hook_header_pages);
  uefi_call_wrapper(BS->FreePages, 2, (uintptr_t)loaded_hook->entry, hook_pages - hook_header_pages);

  loaded_hook->is_loaded = false;
}

err_t hook_services(void) {
  err_t err = SUCCESS;

  CHECK_RETHROW(hook_set_variable());
  CHECK_RETHROW(hook_get_memory_map());
  CHECK_RETHROW(create_virtual_address_map_event());

cleanup:
  return err;
}

void unhook_services(void) {
  remove_virtual_address_map_event();
  unhook_get_memory_map();
  unhook_set_variable();
}
