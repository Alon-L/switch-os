#include "hook_sleep_prepare.h"
#include <linux/acpi.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include "core/header.h"

extern struct core_header* g_core_header;

/*
 * The waking vector is stored in the FACS ACPI table.
 * We can access this table by using the FADT ACPI table, as it stores the
 * physical address of the FACS table.
 */
static err_t get_acpi_waking_vector(uint32_t* waking_vector_out) {
  err_t err = SUCCESS;
  struct acpi_table_fadt fadt = acpi_gbl_FADT;
  uint32_t facs_phys_addr;
  struct acpi_table_facs* facs = NULL;
  uint32_t waking_vector;

  facs_phys_addr = fadt.facs;
  CHECK_TRACE(facs_phys_addr != 0,
              "FACS table physical address is uninitialized by the kernel\n");

  // The ACPI tables must be mapped as write-back.
  facs = ioremap_cache(facs_phys_addr, sizeof(struct acpi_table_facs));
  CHECK_TRACE(facs != NULL, "Failed to map FACS table onto virtual memory\n");

  waking_vector = facs->firmware_waking_vector;
  CHECK_TRACE(
      waking_vector != 0,
      "Waking vector physical address is uninitialized by the kernel\n");

  *waking_vector_out = waking_vector;

cleanup:
  if (facs != NULL) {
    // Unmap FACS from virtual memory
    iounmap(facs);
  }

  return err;
}

/*
 * Called right when `acpi_sleep_prepare` ends.
 * `acpi_sleep_prepare` sets the ACPI waking vector. We access it by using `get_acpi_waking_vector`,
 * pass it to core, and override the waking vector to core's wakeup procedure. 
 */
static int my_acpi_sleep_prepare(struct kretprobe_instance* ri,
                                 struct pt_regs* regs) {
  err_t err = SUCCESS;
  uint32_t waking_vector;

  CHECK_RETHROW(get_acpi_waking_vector(&waking_vector));
  TRACE("Original waking vector %x\n", waking_vector);

  CHECK(g_core_header != NULL);

  g_core_header->original_waking_vector = waking_vector;

  acpi_set_firmware_waking_vector(CORE_WAKEUP_PHYS_ADDR, 0);

cleanup:
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
