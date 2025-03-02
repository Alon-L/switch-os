#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "core/header.h"
#include "core_loader.h"
#include "devices.h"
#include "error.h"
#include "suspend/hook_sleep_prepare.h"
#include "trace.h"

struct core_header* g_core_header = NULL;

void trace(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintk(fmt, args);
  va_end(args);
}

static int __init switchos_init(void) {
  err_t err = SUCCESS;

  TRACE("Init switch_os kernel module\n");

  CHECK_RETHROW(register_devices());

cleanup:
  return IS_SUCCESS(err) ? 0 : -1;
}

static void __exit switchos_exit(void) {
  TRACE("Unloading switch_os kernel module\n");

  if (g_core_header != NULL) {
    unload_core(g_core_header);
  }
  unhook_sleep_prepare();
}

MODULE_LICENSE("GPL");

module_init(switchos_init);
module_exit(switchos_exit);
