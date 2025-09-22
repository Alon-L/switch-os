#include "../offsets.h"

SECTIONS
{
  . = HOOK_HEADER_OFFSET;

  .header : {
    *(.header*);
  }

  . = ALIGN(32);
  .data : {
    *(.data*);
    *(.rodata*);
    *(.bss*);
  }

  . = HOOK_ENTRY_OFFSET;
  .text : {
    *(.text._start);
    *(.text*);
  }
}
