#ifndef _PM_PAGING_H
#define _PM_PAGING_H

// Setup identity page mapping (one-to-one) for long mode.
// This maps every virtual page to the physical page with the same address. It is done using huge pages of 1GB.
//
// This also enables long mode, but does not enter it yet. Call `load_lm_gdt` to enter long mode.
void setup_identity_paging(void);

#endif
