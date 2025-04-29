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
  asm volatile("movl %0, %%cr3" ::"r"(pml4));
}

static void enable_pae(void) {
  asm volatile(
    "movl %cr4, %eax\n"
    "orl $(1 << 5), %eax\n"
    "movl %eax, %cr4");
}

static void enable_lm(void) {
  asm volatile(
    "movl $0xC0000080, %ecx\n"
    "rdmsr\n"
    "orl $(1 << 8), %eax\n"
    "wrmsr");
}

static void enable_paging(void) {
  asm volatile(
    "movl %cr0, %eax\n"
    "orl $(1 << 31), %eax\n"
    "movl %eax, %cr0");
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
