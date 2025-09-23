#include "pkg_length.h"

#include "mem.h"

err_t build_pkg_length(size_t size, struct acpi_pkg_length* pkg_length_out) {
  err_t err = SUCCESS;

  CHECK_TRACE(size <= MAX_PKG_LENGTH_SIZE, "The given size for the PkgLength is too large\n");

  if (size <= 63) {
    // A PkgLeadByte can solely represent PkgLengths up to size 63.
    pkg_length_out->encoded_size = 1;
    pkg_length_out->aml[0] = size;
  } else {
    // Additional bytes need to follow the PkgLeadByte to encode packages longer than 63.

    // Bits 0-3 of the PkgLeadByte represent the 4 LSBs of the size.
    pkg_length_out->aml[0] = size & 0b1111;
    size >>= 4;

    // Each following byte encodes the next 8 LSBs of the size.
    size_t byte_count = 1;
    while (size != 0) {
      byte_count++;
      CHECK(byte_count <= MAX_PKG_LENGTH_ENCODED_SIZE);
      pkg_length_out->aml[byte_count - 1] = size & 0xff;
      size >>= 8;
    }

    // Bits 6-7 of the PkgLeadByte represent the number of following bytes.
    pkg_length_out->aml[0] = (byte_count - 1) << 6;

    pkg_length_out->encoded_size = byte_count;
  }

  pkg_length_out->size = size;

cleanup:
  return err;
}

err_t parse_pkg_length(const uint8_t* aml, size_t aml_size, struct acpi_pkg_length* pkg_length_out) {
  err_t err = SUCCESS;

  CHECK_SILENT(aml_size > 0);

  uint8_t pkg_lead_byte = aml[0];
  if (pkg_lead_byte <= 63) {
    // If the PkgLeadByte is less than 64, the PkgLength is represented solely by it.
    pkg_length_out->encoded_size = 1;
    pkg_length_out->size = pkg_lead_byte;
  } else {
    // Additional bytes need to follow the PkgLeadByte to encode packages longer than 63.

    // Bits 4-5 in the PkgLeadByte are reserved and must be 0,
    CHECK_SILENT((pkg_lead_byte & 0b110000) == 0);

    // Bits 0-3 of the PkgLeadByte represent the 4 LSBs of the size.
    uint8_t pkg_length_nybble = pkg_lead_byte & 0b1111;
    // Bits 6-7 of the PkgLeadByte represent the number of following bytes.
    uint8_t bytedata_count = (pkg_lead_byte & 0b11000000) >> 6;

    // Bytedata count must be between 0-3.
    CHECK_SILENT(bytedata_count <= 3);
    // Make sure we won't overflow.
    CHECK_SILENT(bytedata_count <= aml_size);

    // Each following byte encodes the next 8 LSBs of the size. Concatenate the next `bytedata_count` bytes into the
    // final size.
    size_t size = pkg_length_nybble;
    for (size_t i = 0; i < bytedata_count; i++) {
      size |= (aml[i + 1] << ((8 * i) + 4));
    }

    // The encoded size of the PkgLength is the lead byte and the bytedatas.
    pkg_length_out->encoded_size = bytedata_count + 1;
    pkg_length_out->size = size;
  }

  memset(pkg_length_out->aml, 0, sizeof(pkg_length_out->aml));
  memcpy(pkg_length_out->aml, aml, pkg_length_out->encoded_size);

cleanup:
  return err;
}
