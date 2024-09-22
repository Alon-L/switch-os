#include "core/consts.h"

OUTPUT_ARCH(i386)

SECTIONS
{
  . = CORE_REALMODE_PHYS_ADDR;

  .text : {
    *(.text*);
  }
	
	. = ALIGN(128);
	.bss : {
		*(.bss*);
	}
}
