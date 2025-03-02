#include "gdt.h"

#include <stdint.h>

#include "core/consts.h"

struct gdt_entry {
  uint16_t limit;
  uint16_t base1;
  uint8_t base2;
  uint8_t access;
  uint8_t flags;  // 4 low bits are the 4 high bits of limit.
  uint8_t base3;
};

#define GDT_ACCESS_TYPE_CODE (11)  // Execute/Read, accessed
#define GDT_ACCESS_TYPE_DATA (3)   // Read/Write, accessed
#define GDT_ACCESS_CODE_DATA_DESC (1 << 4)
#define GDT_ACCESS_PRESENT (1 << 7)

#define GDT_FLAGS_64BIT (1 << 5)
#define GDT_FLAGS_GRANULARITY (1 << 7)

static const struct gdt_entry g_lm_gdt[3] = {
  {0},  // Null entry
  {
    .limit = 0,
    .base1 = 0,
    .base2 = 0,
    .access = GDT_ACCESS_TYPE_CODE | GDT_ACCESS_CODE_DATA_DESC | GDT_ACCESS_PRESENT,
    .flags = 0xf | GDT_FLAGS_64BIT | GDT_FLAGS_GRANULARITY,
    .base3 = 0,
  },  // 64-bit code descriptor
  {
    .limit = 0,
    .base1 = 0,
    .base2 = 0,
    .access = GDT_ACCESS_TYPE_DATA | GDT_ACCESS_CODE_DATA_DESC | GDT_ACCESS_PRESENT,
    .flags = 0xf | GDT_FLAGS_64BIT | GDT_FLAGS_GRANULARITY,
    .base3 = 0,
  },  // 64-bit data descriptor
};

static struct {
  uint16_t limit;
  uint64_t gdt;
} __attribute__((packed)) g_lm_gdt_ptr;

__attribute__((noreturn)) void load_lm_gdt() {
  g_lm_gdt_ptr.limit = sizeof(g_lm_gdt) - 1;
  g_lm_gdt_ptr.gdt = (uintptr_t)&g_lm_gdt;

  asm volatile(
    "lgdt %0\n"
    "jmp 8:%1\n" ::"m"(g_lm_gdt_ptr),
    "i"(CORE_MAIN_PHYS_ADDR));

  __builtin_unreachable();
}
