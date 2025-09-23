#pragma once

#include <stddef.h>
#include <stdint.h>

#include "error.h"
#include "pkg_length.h"

/**
 * Locates an AML method by name in the DSDT.
 *
 * @param start_out       - The offset to the AML method opcode that declares this method.
 * @param pkg_length_out  - The parsed PkgLength object in the method's declaration.
 * @return                - Success if the method was found and parsed, or a failure otherwise.
 */
err_t find_method(const char* name, size_t* start_out, struct acpi_pkg_length* pkg_length_out);

/**
 * Appends a method to the end of the DSDT table.
 *
 * This function accepts the method's header and body. This is to be compliant with IASL's AML output.
 */
err_t append_method(const uint8_t* hook_header_aml, size_t hook_header_size, const uint8_t* hook_body_aml,
                    size_t hook_body_size);

/**
 * Hooks an existing AML method in the DSDT.
 *
 * The hook is performed by renaming the AML method to `modified_name`, and appending a new method with the original
 * name.
 * The appended method now becomes the real method, and it can decide whether to call the original method by calling
 * `modified_name`.
 */
err_t hook_method(const char* name, const char* modified_name, const uint8_t* hook_header_aml, size_t hook_header_size,
                  const uint8_t* hook_body_aml, size_t hook_body_size);
