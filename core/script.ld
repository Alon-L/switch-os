#include "core/consts.h"

SECTIONS
{
  . = CORE_PHYS_ADDR;

  .data : {
    core_header = .;
  
    // Must match `struct core_header` in `core/header.h`.
    LONG(CORE_HEADER_MAGIC);
    // TODO: add symbol for original waking vector
    LONG(0);
    SHORT(CORE_RM_PHYS_ADDR);

    *(.data*);
  }

  .text : {
    *(.text*);
  }
	
	. = ALIGN(128);
	.bss : {
		*(.bss*);
	}
}
