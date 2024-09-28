#include "core/consts.h"
#include "core/header.h"

__attribute__((section(".core_header"))) struct core_header core_header = {
  .magic = CORE_HEADER_MAGIC,
  .original_waking_vector = 0,
};

__attribute__((noreturn)) void core_main(void) {
  while (1) {}

  __builtin_unreachable();
}
