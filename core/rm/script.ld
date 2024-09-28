#include "core/consts.h"

OUTPUT_ARCH(i386);

SECTIONS
{
  . = CORE_RM_PHYS_ADDR;

  .text : {
    *(core_wakeup);
    *(.text*);
  }

  .bss : {
    *(.bss*);
  }

  .data : {
    *(.data*);
  }
}
