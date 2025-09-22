#include "core/consts.h"

OUTPUT_ARCH(i386);

SECTIONS
{
  . = CORE_RM_PHYS_ADDR;

  .header : {}

  .text : {
    *(.text.core_wakeup);
    *(.text*);
  }

  .data : {
    *(.data*);
    *(.rodata*);
    *(.bss*);
  }
}
