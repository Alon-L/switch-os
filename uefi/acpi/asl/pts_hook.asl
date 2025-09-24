#include "core/consts.h"

DefinitionBlock ("", "SSDT", 2, "", "", 0x0)
{
  Method (_PTS, 1, NotSerialized)
  {
    OperationRegion (FACS, SystemMemory, 0x12345678, 64)
    Field (FACS, AnyAcc, NoLock, Preserve)
    {
      Offset (12),
      FWAK, 32,
      Offset (40),
      RSV1, 32,
    }

    // Store the original waking vector in the second reserved field.
    RSV1 = FWAK

    // Overwrite the waking vector with core's entry.
    FWAK = CORE_RM_PHYS_ADDR

    SPTS(arg0)
  }

  Method (SPTS, 1, NotSerialized)
  {
    // This function is a placeholder, and won't be loaded when hooking the `_PTS`.
    // In case `_PTS` already exists, it will be renamed to `SPTS` and our hook will call it.
  }
}
