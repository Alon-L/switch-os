#include "method.h"

#include "acpi/aml.h"
#include "acpi/tables.h"
#include "dsdt.h"
#include "mem.h"
#include "patch_utils.h"

err_t find_method(const char* name, size_t* start_out, struct acpi_pkg_length* pkg_length_out) {
  err_t err = SUCCESS;

  const uint8_t* aml = g_dsdt->definition_block;
  size_t aml_size = DSDT_AML_SIZE(*g_dsdt);

  size_t method_start = -1;
  struct acpi_pkg_length pkg_length;

  // Scan the DSDT for an AML method declaration for `name`.
  // Method declarations are encoded as follows:
  //  DefMethod := MethodOp PkgLength NameString ...
  // So we need to locate the MethodOp, PkgLength and NameString (which is `name`) sequentially.
  for (size_t i = 0; i < aml_size; i++) {
    if (aml[i] != AML_METHOD_OP) {
      continue;
    }

    // `parse_pkg_length` deals with the case we overflow beyond `aml_size`.
    if (parse_pkg_length(aml + i + 1, aml_size - i - 1, &pkg_length) != SUCCESS) {
      continue;
    }

    // This looks like a method declaration beginning at `i`. Check whether the declared method has the expected name.

    // Make sure we don't overflow.
    CHECK(i + 1 + pkg_length.encoded_size + AML_METHOD_NAME_LEN <= aml_size);
    if (memcmp((const char*)&aml[i + 1 + pkg_length.encoded_size], name, AML_METHOD_NAME_LEN) != 0) {
      continue;
    }

    CHECK_TRACE(method_start == -1, "Method %s found twice\n", name);
    method_start = i;
  }

  CHECK_TRACE(method_start != -1, "Method %s not found\n", name);

  if (start_out) {
    *start_out = method_start;
  }
  if (pkg_length_out) {
    *pkg_length_out = pkg_length;
  }

cleanup:
  return err;
}

err_t append_method(const struct aml_method_part* method_parts, size_t method_parts_size) {
  err_t err = SUCCESS;

  for (size_t i = 0; i < method_parts_size; i++) {
    CHECK_RETHROW(append_dsdt(method_parts[i].aml, method_parts[i].size));
  }

cleanup:
  return err;
}

/**
 * Renames an AML method in the DSDT.
 *
 * Since all AML methods have the same length, this doesn't create a new DSDT.
 */
static err_t rename_method(const char* original_name, const char* modified_name) {
  err_t err = SUCCESS;

  CHECK(strlen(original_name) == AML_METHOD_NAME_LEN);
  CHECK(strlen(modified_name) == AML_METHOD_NAME_LEN);

  size_t method_start = -1;
  struct acpi_pkg_length method_pkg_length;
  CHECK_RETHROW(find_method(original_name, &method_start, &method_pkg_length));

  // The method declaration is encoded as:
  //  DefMethod := MethodOp PkgLength NameString ...
  size_t method_name_offset = method_start + 1 + method_pkg_length.encoded_size;

  // Make sure we don't overflow when modifying the name.
  CHECK(method_name_offset + AML_METHOD_NAME_LEN <= DSDT_AML_SIZE(*g_dsdt));

  memcpy(g_dsdt->definition_block + method_name_offset, modified_name, AML_METHOD_NAME_LEN);

  fix_table_checksum((struct acpi_table_header*)g_dsdt);

cleanup:
  return err;
}

err_t hook_method(const char* name, const char* modified_name, const struct aml_method_part* hook_parts,
                  size_t hook_parts_size) {
  err_t err = SUCCESS;

  CHECK_RETHROW(rename_method(name, modified_name));

  // Append the hook function. It is assumed to be named `name`, and contain a call to the original hook if needed by
  // calling `modified_name`.
  CHECK_RETHROW(append_method(hook_parts, hook_parts_size));

cleanup:
  return err;
}
