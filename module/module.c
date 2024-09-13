#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>
#include "core.h"
#include "core_loader.h"
#include "error.h"
#include "suspend/hook_sleep_prepare.h"
#include "trace.h"

typedef int (*core_start_t)(void);
void* g_core_addr = NULL;

void trace(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintk(fmt, args);
  va_end(args);
}

int init_module(void) {
  err_t err = SUCCESS;

  TRACE("Init switch_os kernel module\n");

  CHECK_RETHROW(load_core(&g_core_addr));

  TRACE("core res: %d\n",
          ((core_start_t)(g_core_addr + CORE_START_OFFSET))());

  CHECK_RETHROW(hook_sleep_prepare());

  TRACE("Hooked acpi_sleep_prepare\n");

cleanup:
  return !IS_SUCCESS(err);
}

void cleanup_module(void) {
  TRACE("Unloading switch_os kernel module\n");

  if (g_core_addr != NULL) {
    unload_core(g_core_addr);
  }
  if (is_sleep_prepare_hooked()) {
    unhook_sleep_prepare();
  }
}

MODULE_LICENSE("GPL");
