#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include "core_loader.h"

typedef int (*core_start_t)(void);

void* g_core_addr = NULL;

int init_module(void) {
  pr_info("Init switch_os kernel module\n");

  if (load_core(&g_core_addr) != 0) {
    pr_info("Failed to load core\n");
    return 1;
  }

  pr_info("core res: %d\n", ((core_start_t)(g_core_addr + 0x1000))());

  return 0;
}

void cleanup_module(void) {
  pr_info("Unloading switch_os kernel module\n");

  if (g_core_addr != NULL) {
    unload_core(g_core_addr);
  }
}

MODULE_LICENSE("GPL");
