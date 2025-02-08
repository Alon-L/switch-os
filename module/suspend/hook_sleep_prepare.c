#include "hook_sleep_prepare.h"

#include <linux/acpi.h>
#include <linux/kprobes.h>

#include "../configure_core_header.h"
#include "core/consts.h"

/**
 * Called right when `acpi_sleep_prepare` ends.
 * We configure core's header and hook the waking vector.
 *
 * `acpi_sleep_prepare` sets the ACPI waking vector. We access it by using
 * `get_acpi_waking_vector`, and override the waking vector to
 * core's wakeup procedure.
 */
static int my_acpi_sleep_prepare(struct kretprobe_instance* ri,
                                 struct pt_regs* regs) {
  if (configure_core_header() != SUCCESS) {
    return 0;
  }

  acpi_set_firmware_waking_vector(CORE_WAKEUP_PHYS_ADDR, 0);
  return 0;
}

static struct kretprobe g_kretprobe = {
  .kp =
    {
      .symbol_name = "acpi_sleep_prepare",
    },
  .handler = my_acpi_sleep_prepare,
};

static bool g_is_hooked = false;

err_t hook_sleep_prepare(void) {
  err_t err = SUCCESS;

  CHECK(register_kretprobe(&g_kretprobe) == 0);

  g_is_hooked = true;

cleanup:
  return err;
}

void unhook_sleep_prepare(void) {
  if (g_is_hooked) {
    unregister_kretprobe(&g_kretprobe);
    g_is_hooked = false;
  }
}
