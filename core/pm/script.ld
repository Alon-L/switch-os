#include "core/consts.h"

OUTPUT_ARCH(i386)

SECTIONS
{
  . = CORE_PM_PHYS_ADDR;

  .text : {
    *(.text*);
  }
	
	. = ALIGN(128);
	.bss : {
		*(.bss*);
    . += 4096;
    pm_stack_end = .;
	}
	
  . = ALIGN(128);
	.data : {
		*(.data*);
	}
}
