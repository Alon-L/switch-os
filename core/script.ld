#include "core/consts.h"

SECTIONS
{
  . = CORE_PHYS_ADDR;

  .core_header : {
    *(.core_header*);
  }

  . = CORE_MAIN_PHYS_ADDR;
  .text : {
    *(.text.core_main)
    *(.text*);
  }

  . = ALIGN(128);
  .data : {
    *(.data*);
  }

  . = ALIGN(128);
  .bss : {
    *(.bss*);
  }
}
