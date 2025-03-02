#include "paging.h"

#include <stdint.h>

// The page table first contains the PML4 table, and then sequentially contains the 512 PDPTs.
extern char __pml4;
#define PML4 ((uint64_t*)(&__pml4))

#define PT_SIZE (4096)
#define PT_ENTRY_SIZE (8)
#define PT_ENTRIES_NUM (PT_SIZE / PT_ENTRY_SIZE)

#define PAGE_PRESENT_BIT (1U << 0)
#define PAGE_RW_BIT (1U << 1)
#define PAGE_SIZE_BIT (1U << 7)

#define GB_PAGE_SIZE ((uint64_t)(1024 * 1024 * 1024))

static inline uint64_t pml4_entry(uintptr_t pdpt_addr) {
  return pdpt_addr | PAGE_PRESENT_BIT | PAGE_RW_BIT;
}

static inline uint64_t pdpt_entry(uint64_t page_addr) {
  return page_addr | PAGE_PRESENT_BIT | PAGE_RW_BIT | PAGE_SIZE_BIT;
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

// Fill a PDPT with identity mapping that consists of huge pages of size 1GB.
static void setup_pdpt(uint64_t* pdpt, uint32_t pdpt_idx) {
  for (uint32_t i = 0; i < PT_ENTRIES_NUM; i++) {
    uint64_t page_addr = GB_PAGE_SIZE * ((pdpt_idx * PT_ENTRIES_NUM) + i);
    pdpt[i] = pdpt_entry(page_addr);
  }
}

static void setup_pml4(void) {
  for (uint32_t i = 0; i < PT_ENTRIES_NUM; i++) {
    // All PDPTs are stored sequentially in memory after the PML4.
    uintptr_t pdpt_addr = (uintptr_t)PML4 + (PT_SIZE * (i + 1));

    setup_pdpt((uint64_t*)pdpt_addr, i);

    PML4[i] = pml4_entry(pdpt_addr);
  }
}

void setup_identity_paging(void) {
  setup_pml4();
  load_cr3(PML4);
  enable_pae();
  enable_lm();
  enable_paging();
}
