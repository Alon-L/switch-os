#pragma once

#include <stddef.h>
#include <stdint.h>

#include "error.h"

// A PkgLength is made of a PkgLeadByte and up to 3 additional bytes. Therefore, it can only span up to 4 bytes.
#define MAX_PKG_LENGTH_ENCODED_SIZE 4

#define MAX_PKG_LENGTH_SIZE (1 << 28)

// This struct represents a parsed AML PkgLength object.
struct acpi_pkg_length {
  /// The package length represented by this PkgLength object. Bounded by `MAX_PKG_LENGTH_SIZE`.
  size_t size;

  /// The number of bytes this PkgLength object spans across. Bounded by `MAX_PKG_LENGTH_ENCODED_SIZE`.
  /// Can be used as the array size to the `aml` field.
  size_t encoded_size;

  /// The AML encoding of this PkgLength object.
  uint8_t aml[MAX_PKG_LENGTH_ENCODED_SIZE];
};

/**
 * Builds a new PkgLength object that represents a package of length `size`.
 */
err_t build_pkg_length(size_t size, struct acpi_pkg_length* pkg_length_out);

/**
 * Parses AML code that encodes a PkgLength object into a `acpi_pkg_length` struct.
 *
 * Returns an error if the AML code doesn't represent a valid PkgLength object.
 *
 * NOTE: the `CHECKS` in this function are silent, so it can be used to locate AML code that represents a PkgLength
 * without spamming traces.
 */
err_t parse_pkg_length(const uint8_t* aml, size_t aml_size, struct acpi_pkg_length* pkg_length_out);
