#include <stdint.h>
#include "core/consts.h"

extern char __pml4;
#define PML4 ((uint64_t*)(&__pml4))

#define TABLE_SIZE (4096)
#define TABLE_ENTRY_SIZE (8)
#define TABLE_ENTRY_NUM (TABLE_SIZE / TABLE_ENTRY_SIZE)

#define GB_PAGE_SIZE ((uint64_t)(1024 * 1024 * 1024))

#define PRESENT_BIT (1U << 0)
#define READ_WRITE_BIT (1U << 1)
#define PAGE_SIZE_BIT (1U << 7)

static const uint64_t lm_gdt[3] = {
  0,                   // Null entry
  0x00af9b000000ffff,  // 64-bit code segment
  0x00cf93000000ffff,  // 32-bit data segment
};

static struct {
  uint16_t limit;
  uint64_t gdt;
} __attribute__((packed)) lm_gdt_ptr;

static uint64_t build_pml4_entry(uintptr_t pdpt_addr) {
  return pdpt_addr | PRESENT_BIT | READ_WRITE_BIT;
}

static uint64_t build_pdpt_entry(uint64_t page_addr) {
  return page_addr | PRESENT_BIT | READ_WRITE_BIT | PAGE_SIZE_BIT;
}

static void load_cr3(void* pml4) {
  asm volatile("mov cr3, %0" ::"r"(pml4));
}

static void enable_pae(void) {
  asm volatile(
    "mov eax, cr4\n"
    "or eax, 1 << 5\n"
    "mov cr4, eax");
}

static void enable_lm(void) {
  asm volatile(
    "mov ecx, 0xC0000080\n"
    "rdmsr\n"
    "or eax, 1 << 8\n"
    "wrmsr");
}

static void enable_paging(void) {
  asm volatile(
    "mov eax, cr0\n"
    "or eax, 1 << 31\n"
    "mov cr0, eax");
}

static __attribute__((noreturn)) void load_lm_gdt() {
  lm_gdt_ptr.limit = sizeof(lm_gdt) - 1;
  lm_gdt_ptr.gdt = (uintptr_t)&lm_gdt;

  asm volatile(
    "lgdt %0\n"
    "jmp 8:%1\n" ::"m"(lm_gdt_ptr),
    "i"(CORE_MAIN_PHYS_ADDR));

  __builtin_unreachable();
}

static void setup_pdpt(uint64_t* pdpt, uint32_t pdpt_idx) {
  for (uint32_t i = 0; i < TABLE_ENTRY_NUM; i++) {
    uint64_t* pdpt_entry = pdpt + i;

    uint64_t relevant_page_addr =
      GB_PAGE_SIZE * ((pdpt_idx * TABLE_ENTRY_NUM) + i);

    *pdpt_entry = build_pdpt_entry(relevant_page_addr);
  }
}

static void setup_pml4(void) {
  for (uint32_t i = 0; i < TABLE_ENTRY_NUM; i++) {
    uint64_t* pml4_entry = PML4 + i;

    /*
     * The PDPTs that corresponds to this PML4 entry is indexed `i`.
     * All PDPTs are placed sequentially in memory after the PML4.
     */
    uintptr_t relevant_pdpt_addr = (uintptr_t)PML4 + (TABLE_SIZE * (i + 1));

    setup_pdpt((uint64_t*)relevant_pdpt_addr, i);
    *pml4_entry = build_pml4_entry(relevant_pdpt_addr);
  }
}

__attribute__((noreturn)) void main_pm() {
  setup_pml4();
  load_cr3(PML4);
  enable_pae();
  enable_lm();
  enable_paging();
  load_lm_gdt();

  __builtin_unreachable();
}
