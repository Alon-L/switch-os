#ifndef _PM_GDT_H
#define _PM_GDT_H

// Load the long mode GDT, which simply marks the entire memory range as code and data.
// The function then jumps to `core_main` without returning.
__attribute__((noreturn)) void load_lm_gdt(void);

#endif
