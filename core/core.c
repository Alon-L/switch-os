#include "acpi.h"
#include "alloc.h"
#include "core/consts.h"
#include "core/header.h"
#include "drivers/virtio/virtio_blk.h"
#include "dump.h"
#include "error.h"
#include "trace.h"

__attribute__((section(".core_header"))) struct core_header g_core_header = {
  .magic = CORE_HEADER_MAGIC,
  .action = CORE_ACTION_INVALID,
};

__attribute__((noreturn)) void core_main(void) {
  err_t err = SUCCESS;
  (void)err;

  TRACE("Running switch os core...\n");

  core_init_allocators();
  g_kernel_waking_vector = g_core_header.original_waking_vector;

  CHECK(is_core_header_valid(&g_core_header));

  struct virtio_blk_dev virtio_blk_dev;
  CHECK_RETHROW(init_virtio_blk_dev(&virtio_blk_dev));

  switch (g_core_header.action) {
    case CORE_ACTION_STORE: {
      TRACE("Storing dump...\n");
      CHECK_RETHROW(disk_store_dump(&virtio_blk_dev));
      break;
    }
    case CORE_ACTION_SWITCH: {
      TRACE("Switching dump...\n");
      bool contains_dump = false;
      CHECK_RETHROW(does_disk_contain_dump(&virtio_blk_dev, &contains_dump));
      CHECK_TRACE(contains_dump, "Dump not found on disk. Aborting switch.\n");
      CHECK_RETHROW(disk_switch_dump(&virtio_blk_dev));
      break;
    }
    default: {
      CHECK_FAIL_TRACE("Invalid core action!\n");
    }
  }

cleanup:
  acpi_setup();

  TRACE("Waking up kernel...\n");
  acpi_return_kernel();

  // We should be in suspend by this point.

  __builtin_unreachable();
}
