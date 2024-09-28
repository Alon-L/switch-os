  .intel_syntax noprefix

  .text
  .globl pm_stub
pm_stub:
  // The first PM code that runs after wakeup.
  // The short RM assembly already prepares the GDT and loaded the segments.

  // Setup the stack and jump to the C code
  lea ebp, __pm_stack_end
  mov	esp, ebp

  jmp main_pm
