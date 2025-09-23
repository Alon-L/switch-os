#include "patch_utils.h"

#include <stddef.h>

void fix_table_checksum(struct acpi_table_header* table) {
  // Reset the checksum field before calculating the checksum, so the previous checksum isn't counted.
  table->checksum = 0;

  uint8_t checksum = 0;
  for (size_t i = 0; i < table->length; i++) {
    checksum += ((uint8_t*)table)[i];
  }

  // The checksum must fulfill `table->checksum + CHECKSUM(table) == 0`.
  table->checksum = -checksum;
}
