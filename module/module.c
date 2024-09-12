#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>
#include "core.h"
#include "core_loader.h"
#include "suspend/hook_sleep_prepare.h"

typedef int (*core_start_t)(void);

void* g_core_addr = NULL;

int init_module(void) {
  pr_info("Init switch_os kernel module\n");

  if (load_core(&g_core_addr) != 0) {
    pr_err("Failed to load core\n");
    return 1;
  }

  pr_info("core res: %d\n",
          ((core_start_t)(g_core_addr + CORE_START_OFFSET))());
  
  if (hook_sleep_prepare() != 0) {
    pr_err("Failed to hook sleep_prepare\n");
    return 1;
  }

  pr_info("Hooked acpi_sleep_prepare\n");

  return 0;
}

void cleanup_module(void) {
  pr_info("Unloading switch_os kernel module\n");

  if (g_core_addr != NULL) {
    unload_core(g_core_addr);
  }
  if (is_sleep_prepare_hooked()) {
    unhook_sleep_prepare();
  }
}

MODULE_LICENSE("GPL");
