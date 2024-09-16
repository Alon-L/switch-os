#include "core/consts.h"

SECTIONS
{
  . = CORE_HEADER_OFFSET;

  .data : {
    core_header = .;
  
    // Must match `struct core_header` in `core/header.h`.
    LONG(CORE_HEADER_MAGIC);
    LONG(_start - core_header);
  }

  .text : {
    *(_start);
  }
}
