#pragma once

#include "../tables.h"

/**
 * Fixes an ACPI table's checksum after modifying it.
 */
void fix_table_checksum(struct acpi_table_header* table);
