#include <uacpi/event.h>
#include <uacpi/sleep.h>

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

int setup_acpi(void) {
  // TODO: Remove all these comments
  /*
   * Start with this as the first step of the initialization. This loads all
   * tables, brings the event subsystem online, and enters ACPI mode. We pass
   * in 0 as the flags as we don't want to override any default behavior for
   * now.
   */
  uacpi_status ret = uacpi_initialize(0);
  if (uacpi_unlikely_error(ret)) {
    return -1;
  }

  /*
   * Load the AML namespace. This feeds DSDT and all SSDTs to the interpreter
   * for execution.
   */
  ret = uacpi_namespace_load();
  if (uacpi_unlikely_error(ret)) {
    return -1;
  }

  /*
   * Initialize the namespace. This calls all necessary _STA/_INI AML methods,
   * as well as _REG for registered operation region handlers.
   */
  ret = uacpi_namespace_initialize();
  if (uacpi_unlikely_error(ret)) {
    return -1;
  }

  /*
   * Tell uACPI that we have marked all GPEs we wanted for wake (even though we
   * haven't actually marked any, as we have no power management support right
   * now). This is needed to let uACPI enable all unmarked GPEs that have a
   * corresponding AML handler. These handlers are used by the firmware to
   * dynamically execute AML code at runtime to e.g. react to thermal events or
   * device hotplug.
   */
  ret = uacpi_finalize_gpe_initialization();
  if (uacpi_unlikely_error(ret)) {
    return -1;
  }

  /*
   * That's it, uACPI is now fully initialized and working! You can proceed to
   * using any public API at your discretion. The next recommended step is
   * namespace enumeration and device discovery so you can bind drivers to ACPI
   * objects.
   */
  return 0;
}

__attribute__((noreturn)) void core_main(void) {
  err_t err = SUCCESS;
  (void)err;

  TRACE("Running switch os core...\n");

  core_init_allocators();

  // TODO: Validate g_core_header.

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
  setup_acpi();
  uacpi_set_waking_vector(g_core_header.original_waking_vector, 0);
  uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S3);
  uacpi_enter_sleep_state(UACPI_SLEEP_STATE_S3);

  // We should be in suspend by this point.

  __builtin_unreachable();
}
