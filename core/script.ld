#include "core/consts.h"

SECTIONS
{
  . = CORE_PHYS_ADDR;

  .data : {
    core_header = .;
  
    // Must match `struct core_header` in `core/header.h`.
    LONG(CORE_HEADER_MAGIC);
    LONG(0);
    LONG(_start - core_header);
  }

  .text : {
    *(_start);
  }
}
