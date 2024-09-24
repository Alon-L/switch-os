#include "core/consts.h"

  .intel_syntax noprefix

  .data
  .balign 16
gdtr:
  // Null Descriptor
  .word 0
  .long 0
  .word 0
	
  // Code Segment, the entire 4GB address space
  .word	0xffff      // Limit 0xFFFFF (along with bits 48-51)
	.long	0x9b000000  // Base 0 and Access Byte (P & S & E & RW & A) 
	.word	0x008f      // Upper 0xF for limit and Granularity Flag for 4GB limit
  
  // Data Segment, the entire 4GB address space
  .word	0xffff      // Limit 0xFFFFF (along with bits 48-51)
	.long	0x93000000  // Base 0 and Access Byte (P & S & RW & A) 
	.word	0x008f      // Upper 0xF for limit and Granularity Flag for 4GB limit
  
  .text
	.code16

	.balign	16
  .globl core_wakeup
core_wakeup:
	cli
	cld

  // Reset all segments
  mov	ax, cs
	mov	ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
  // Reset eflags
  mov	esp, cs:rm_stack_end
  push 0
	popf
 
  // Enter protected mode
  lgdt [cs:gdtr]
  mov eax, cr0 
  or al, 1 // Set PE bit
  mov cr0, eax

  jmp 8:pm // Perform a far jump to selector 8 (code segment) in the GDT to load it.
pm:
  .code32

  // Load all other segments with selector 16 (data segment) in the GDT.
  mov cx, 16
  mov ds, cx
  mov es, cx
  mov ss, cx
  mov fs, cx
  mov gs, cx

  // Continue in our PM stub.
  jmp CORE_PM_PHYS_ADDR

  .bss
  .balign 16

  .space 64
  .globl rm_stack_end
rm_stack_end:

