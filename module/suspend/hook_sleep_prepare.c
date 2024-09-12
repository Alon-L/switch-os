#include "hook_sleep_prepare.h"
#include <linux/kernel.h>
#include <linux/kprobes.h>

static int my_acpi_sleep_prepare(struct kretprobe_instance* ri,
                                 struct pt_regs* regs) {
  pr_info("Inside handler\n");

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

int hook_sleep_prepare(void) {
  if (register_kretprobe(&g_kretprobe) != 0) {
    pr_err("Failed to place kretprobe on acpi_sleep_prepare\n");
    return 1;
  }

  g_is_hooked = true;
  return 0;
}

void unhook_sleep_prepare(void) {
  unregister_kretprobe(&g_kretprobe);
  g_is_hooked = false;
}

bool is_sleep_prepare_hooked(void) {
  return g_is_hooked;
}
