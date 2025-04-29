#include "configure_core_header.h"

#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/types.h>

#include "acpi.h"
#include "core/header.h"

extern struct core_header* g_core_header;

static err_t fill_original_waking_vector(void) {
  err_t err = SUCCESS;

  uint32_t waking_vector;
  CHECK_RETHROW(acpi_find_waking_vector(&waking_vector));
  g_core_header->original_waking_vector = waking_vector;

cleanup:
  return err;
}

err_t configure_core_header(void) {
  err_t err = SUCCESS;

  CHECK(g_core_header != NULL);

  CHECK_RETHROW(fill_original_waking_vector());

cleanup:
  return err;
}
