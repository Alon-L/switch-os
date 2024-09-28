#include "core/consts.h"

  .intel_syntax noprefix

  .data
  .balign 16
gdt:
  // Self-descriptor. This serves as the 6-byte GDTR argument to pass to LGDT 
  .word	gdt_end - gdt - 1   // GDT limit (size)
  .long gdt	                // GDT base (pointer to itself)
  .word	0

  // Code Segment, the entire 4GB address space
  .word	0xffff      // Limit 0xFFFFF (along with bits 48-51)
  .long	0x9b000000  // Base 0 and Access Byte (P & S & E & RW & A) 
  .word	0x00cf      // Upper 0xF for limit, Granularity Flag for 4GB limit, DB for 32bit protected mode

  // Data Segment, the entire 4GB address space
  .word	0xffff      // Limit 0xFFFFF (along with bits 48-51)
  .long	0x93000000  // Base 0 and Access Byte (P & S & RW & A) 
  .word	0x00cf      // Upper 0xF for limit, Granularity Flag for 4GB limit, DB for 32bit protected mode
gdt_end:

  .text
  .code16

  .balign	16
  .globl core_wakeup
core_wakeup:
  cli
  cld

  // Reset eflags
  mov	esp, cs:rm_stack_end
  push 0
  popf

  // Enter protected mode
  lgdt [gdt]
  mov eax, cr0 
  or al, 1 // Set PE bit
  mov cr0, eax

  jmp 0x8:begin_pm // Perform a far jump to selector 8 (code segment) in the GDT to load it.
begin_pm:
  .code32

  // Load all other segments with selector 16 (data segment) in the GDT.
  mov cx, 16
  mov ds, cx
  mov es, cx
  mov ss, cx
  mov fs, cx
  mov gs, cx

  // Continue in our PM stub (absolute jump).
  mov eax, CORE_PM_PHYS_ADDR
  jmp eax

  .bss
  .balign 16

  .space 64
rm_stack_end:

