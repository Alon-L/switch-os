#include "gdt.h"
#include "paging.h"

__attribute__((noreturn)) void main_pm() {
  // Setup identity page mapping (one-to-one), and enter long mode.
  setup_identity_paging();
  load_lm_gdt();

  // By now we should be in long mode, in `core_main`.

  __builtin_unreachable();
}
