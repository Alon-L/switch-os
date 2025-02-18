#include "acpi.h"

#include <uacpi/event.h>
#include <uacpi/sleep.h>

#include "core/header.h"

extern struct core_header g_core_header;

uint32_t g_kernel_waking_vector = 0;

err_t acpi_setup(void) {
  err_t err = SUCCESS;

  // This loads all tables, brings the event subsystem online, and enters ACPI mode.
  uacpi_status ret = uacpi_initialize(0);
  CHECK(uacpi_likely_success(ret));

  // Load the AML namespace. This feeds DSDT and all SSDTs to the interpreter for execution.
  ret = uacpi_namespace_load();
  CHECK(uacpi_likely_success(ret));

  // Initialize the namespace. This calls all necessary _STA/_INI AML methods, as well as _REG for registered operation
  // region handlers.
  ret = uacpi_namespace_initialize();
  CHECK(uacpi_likely_success(ret));

  // Tell uACPI that we have marked all GPEs we wanted for wake (even though we haven't actually marked any, as we have
  // no power management support right now). This is needed to let uACPI enable all unmarked GPEs that have a
  // corresponding AML handler.
  ret = uacpi_finalize_gpe_initialization();
  CHECK(uacpi_likely_success(ret));

cleanup:
  return err;
}

void acpi_return_kernel(void) {
  if (g_kernel_waking_vector != 0) {
    uacpi_set_waking_vector(g_kernel_waking_vector, 0);
  } else {
    TRACE("Kernel waking vector not set!\n");
  }

  uacpi_prepare_for_sleep_state(UACPI_SLEEP_STATE_S3);
  uacpi_enter_sleep_state(UACPI_SLEEP_STATE_S3);
}
