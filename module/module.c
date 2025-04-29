#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "core/consts.h"
#include "core/header.h"
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

static err_t get_core_header_virt(struct core_header** core_header_out) {
  err_t err = SUCCESS;

  // The UEFI application has already loaded core into `CORE_PHYS_ADDR`.
  struct core_header* core_header = ioremap(CORE_PHYS_ADDR, sizeof(struct core_header));
  CHECK_TRACE(core_header != NULL, "Failed to map physical address %lx\n", (uintptr_t)CORE_PHYS_ADDR);

  // Sanity check to make sure this is really the core header.
  CHECK(is_core_header_magic_valid(core_header));

  *core_header_out = core_header;

cleanup:
  return err;
}

static int __init switchos_init(void) {
  err_t err = SUCCESS;

  TRACE("Init switch_os kernel module\n");

  CHECK_RETHROW(get_core_header_virt(&g_core_header));
  CHECK_RETHROW(register_devices());

cleanup:
  return IS_SUCCESS(err) ? 0 : -1;
}

static void __exit switchos_exit(void) {
  TRACE("Unloading switch_os kernel module\n");

  unhook_sleep_prepare();
}

MODULE_LICENSE("GPL");

module_init(switchos_init);
module_exit(switchos_exit);
