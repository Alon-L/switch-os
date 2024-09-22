#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>
#include "core/header.h"
#include "core_loader.h"
#include "error.h"
#include "suspend/hook_sleep_prepare.h"
#include "trace.h"

struct core_header* g_core_header = NULL;

void trace(char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintk(fmt, args);
  va_end(args);
}

int init_module(void) {
  err_t err = SUCCESS;

  TRACE("Init switch_os kernel module\n");

  CHECK_RETHROW(load_core(&g_core_header));

  CHECK_RETHROW(hook_sleep_prepare());

  TRACE("Hooked acpi_sleep_prepare\n");

cleanup:
  return !IS_SUCCESS(err);
}

void cleanup_module(void) {
  TRACE("Unloading switch_os kernel module\n");

  if (g_core_header != NULL) {
    unload_core(g_core_header);
  }
  if (is_sleep_prepare_hooked()) {
    unhook_sleep_prepare();
  }
}

MODULE_LICENSE("GPL");
