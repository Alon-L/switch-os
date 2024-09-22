#include "wakeup.h"

  .intel_syntax noprefix

  .text
	.code16

	.balign	16
  .globl core_wakeup
core_wakeup:
	cli
	cld

	mov	cx, 16
	
  mov	ax, cs
	mov	ss, ax
  mov	esp, dword ptr[rm_stack_end]
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
  push 0
	popf

  .bss
  .balign 16

  .space 2048
  .globl rm_stack_end
rm_stack_end:

