#include "core/consts.h"

OUTPUT_ARCH(i386);

// 32KB
PM_STACK_SIZE = 1024 * 32;

// PML4 + 512 * PDPT
PAGE_TABLE_SIZE = 8 * 512 + 8 * 512 * 512;

SECTIONS
{
  . = CORE_PM_PHYS_ADDR;

  .text : {
    *(pm_stub);
    *(.text*);
  }

  . = ALIGN(128);
  .bss : {
    *(.bss*);

    . = ALIGN(4096);
    __pml4 = .;
    . += PAGE_TABLE_SIZE;

    . += PM_STACK_SIZE;
    __pm_stack_end = .;
  }

  . = ALIGN(128);
  .data : {
    *(.data*);
  }
}
