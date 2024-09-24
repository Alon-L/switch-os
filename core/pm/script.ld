#include "core/consts.h"

OUTPUT_ARCH(i386);

// 32KB
PM_STACK_SIZE = 0x1000 * 8;

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
    . += PM_STACK_SIZE;
    pm_stack_end = .;
	}
	
  . = ALIGN(128);
	.data : {
		*(.data*);
	}
}
