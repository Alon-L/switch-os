#include "core/consts.h"

SECTIONS
{
  . = CORE_PHYS_ADDR;

  .header : {
    *(.core_header*);
  }

  . = CORE_MAIN_PHYS_ADDR;
  .text : {
    *(.text.core_main)
    *(.text*);
  }

  . = ALIGN(4096);
  .data : {
    *(.data*);
    *(.rodata*);
    *(.bss*);
  }
}
