  .intel_syntax noprefix
  
  .text
  .globl pm_stub
pm_stub:
  // The first PM code that runs after wakeup.
  // The short RM assembly already prepares the GDT and loaded the segments.

  // Setup the stack and jump to the C code
  mov	esp, cs:pm_stack_end

  jmp main_pm 
