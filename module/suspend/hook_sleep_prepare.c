#include "hook_sleep_prepare.h"
#include <linux/kernel.h>
#include <linux/kprobes.h>

static int my_acpi_sleep_prepare(struct kretprobe_instance* ri,
                                 struct pt_regs* regs) {
  TRACE("Inside handler\n");
  //acpi_set_firmware_waking_vector(0, 0);

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
  unregister_kretprobe(&g_kretprobe);
  g_is_hooked = false;
}

bool is_sleep_prepare_hooked(void) {
  return g_is_hooked;
}
