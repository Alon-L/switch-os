#include <efi.h>
#include <stdbool.h>

#include "../headers.h"
#include "error.h"
#include "trace.h"

#define EFI_MEMORY_ATTRIBUTES_TABLE_GUID {0xdcfa911d, 0x26eb, 0x469f, 0xa2, 0x20, 0x38, 0xb7, 0xdc, 0x46, 0x12, 0x20};

__attribute__((section(".header"))) struct get_memory_map_hook_header g_hook_header = {};

EFI_STATUS EFIAPI _start(IN OUT UINTN* MemoryMapSize, IN OUT EFI_MEMORY_DESCRIPTOR* MemoryMap, OUT UINTN* MapKey,
                         OUT UINTN* DescriptorSize, OUT UINT32* DescriptorVersion) {
  err_t err = SUCCESS;

  // Call the original GetMemoryMap. Its returned value is returned from the hook regardless.
  EFI_STATUS res =
    g_hook_header.original_get_memory_map(MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);

  if (res == EFI_SUCCESS) {
    // Locate a descriptor that contains the waking vector, and mark it as `EFI_MEMORY_RUNTIME`.
    // This requires the kernel to map this descriptor when calling `VirtualAddressMap`, which lets us access the waking
    // vector in our `SetVariable` hook.
    bool is_desc_found = false;
    for (size_t i = 0; i < *MemoryMapSize / *DescriptorSize; i++) {
      EFI_MEMORY_DESCRIPTOR* desc = NextMemoryDescriptor(MemoryMap, i * *DescriptorSize);
      uintptr_t desc_end = desc->PhysicalStart + (desc->NumberOfPages * EFI_PAGE_SIZE);

      // Check if the descriptor contains the waking vector inside.
      if (desc->PhysicalStart <= g_hook_header.waking_vector_phys_addr &&
          g_hook_header.waking_vector_phys_addr < desc_end) {
        TRACE("Found a descriptor that contains the waking vector (%lx - %lx, type: %x)\n", desc->PhysicalStart,
              desc_end, desc->Type);

        CHECK_TRACE(!is_desc_found, "A descriptor containing the waking vector was found twice\n");
        is_desc_found = true;

        desc->Attribute |= EFI_MEMORY_RUNTIME;
      }
    }

    CHECK_TRACE(is_desc_found, "A descriptor containing the waking_vector was not found\n");
  }

  // The Memory Attributes Table (see section 4.6.4 of the UEFI specs) allows fine-tuning page permissions for runtime
  // services code and data in better granularity.
  // Instead of defining the page permissions via GetMemoryMap, where the granularity is for memory descriptors, here
  // subareas of memory descriptors can be declared with different permissions.
  //
  // This messes with our runtime services hooks, since the firmware can ship this table as explicit as possible, where
  // it only allows the original runtime service code to be executable, and it can also define areas as readonly.
  //
  // We solve this issue by completely deleting this table, since its existence is optional. By deleting it, the
  // entirety of the runtime services descriptors have the same permissions - which should be RWX.
  //
  // Notice that the deletion of the table must occur inside this hook, since the firmware sometimes recreates this
  // table (so one deletion at our UEFI application's init is not enough).
  //
  // TODO: We would ideally want to give our hooks RWX permissions in this table, but this requires parsing it and
  // rebuilding it. This shouldn't be too much work.
  EFI_GUID memory_attributes_table_guid = EFI_MEMORY_ATTRIBUTES_TABLE_GUID;
  g_hook_header.install_configuration_table(&memory_attributes_table_guid, NULL);

cleanup:
  return res;
}
