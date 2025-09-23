#include "hook_pts.h"

#include <stddef.h>

#include "acpi/aml.h"
#include "acpi/patch/pkg_length.h"
#include "asl/pts_hook.c"
#include "asl/pts_hook.offset.h"
#include "mem.h"
#include "patch/method.h"
#include "tables.h"
#include "utils.h"

/**
 * Relocates the `FACS` OperationRegion in the `_PTS` hook to point to the physical address of the real FACS table.
 *
 * See the hook `pts_hook.asl` for the OperationRegion declaration, and see the generated `pts_hook.offset.h` for the
 * format offsets table.
 */
static err_t pts_relocate_facs_addr(void) {
  err_t err = SUCCESS;

  // The offsets table in `pts_hook.offset.h` includes an entry for the `FACS` OperationRegion called `_PTS.FACS`.
  // It includes the offset to the address of the `FACS` OperationRegion which we wish to relocate.
  size_t facs_addr_reloc_offset = -1;
  for (size_t i = 0; i < ARRAY_SIZE(SSDT__OffsetTable); i++) {
    if (strcmp(SSDT__OffsetTable[i].Pathname, "_PTS.FACS") == 0) {
      // The offset is from the beginning of the AML program, but we need the offset from the `SSDT___PTS_FACS` byte
      // array. It comes right after the `SSDT__Header` and `SSDT____PTS`.
      facs_addr_reloc_offset = SSDT__OffsetTable[i].Offset - sizeof(SSDT__Header) - sizeof(SSDT___PTS);
    }
  }

  CHECK(facs_addr_reloc_offset != -1);
  // Make sure we don't overflow.
  CHECK(facs_addr_reloc_offset + sizeof(uint32_t) <= sizeof(SSDT___PTS_FACS));

  // Relocate the address to the real FACS table.
  uint32_t* facs_addr_reloc = (uint32_t*)(SSDT___PTS_FACS + facs_addr_reloc_offset);
  *facs_addr_reloc = (uint32_t)(uintptr_t)g_facs;

cleanup:
  return err;
}

err_t create_or_hook_pts(void) {
  err_t err = SUCCESS;

  CHECK_RETHROW(pts_relocate_facs_addr());

  if (find_method("_PTS", NULL, NULL) == SUCCESS) {
    // Hook the `_PTS` method by renaming it to `SPTS`, and creating a new `_PTS` method which calls the renamed `SPTS`.
    // See the code in `pts_hook.asl` for its implementation.
    CHECK_RETHROW(
      hook_method("_PTS", "SPTS", SSDT___PTS, sizeof(SSDT___PTS), SSDT___PTS_FACS, sizeof(SSDT___PTS_FACS)));
  } else {
    TRACE("Creating a _PTS method\n");

    // The hook in `pts_hook.asl` ends with a call to `SPTS` (the original `_PTS`). This is only necessary in the case
    // we hook an existing `_PTS`. Otherwise we have to get rid of this call.
    // This call is encoded as:
    //  0x53, 0x50, 0x54, 0x53, 0x68  /* A call to `SPTS` with `arg0` as an argument */
    // We could truncate the hook's body by 5 bytes, but this would also require fixing its PkgLength.
    // Instead, we can replace these bytes with ZeroOps.
    size_t call_spts_byte_count = AML_METHOD_NAME_LEN + 1;
    memset(SSDT___PTS_FACS + sizeof(SSDT___PTS_FACS) - call_spts_byte_count, AML_ZERO_OP, call_spts_byte_count);

    CHECK_RETHROW(append_method(SSDT___PTS, sizeof(SSDT___PTS), SSDT___PTS_FACS, sizeof(SSDT___PTS_FACS)));
  }

cleanup:
  return err;
}
