#include "core_loader.h"
#include "linux/io.h"
#include "linux/mm.h"
#include "linux/printk.h"

extern char _binary_build_core_bin_start[];
extern char _binary_build_core_bin_end[];

#define CORE_START (_binary_build_core_bin_start)
#define CORE_SIZE                          \
  ((uintptr_t)_binary_build_core_bin_end - \
   (uintptr_t)_binary_build_core_bin_start)

/*
 * Sets a range of memory to be executable, by turning on the
 * EXEC bit of the page-table entry for all the memory's pages.
 */
static inline int set_memory_exec(void* start, size_t size) {
  void* addr;
  pte_t* addr_pte;
  unsigned int level;

  addr = start;
  for (; (uintptr_t)addr < (uintptr_t)start + size; addr += PAGE_SIZE) {
    addr_pte = lookup_address((uintptr_t)addr, &level);
    if (addr_pte == NULL) {
      return 1;
    }

    set_pte(addr_pte, pte_mkexec(*addr_pte));
  }

  return 0;
}

int load_core(void** core_addr_out) {
  void* core_addr;

  if (CORE_SIZE > CORE_MAX_PHYS_MEM_SIZE) {
    pr_err("Not enough reserved RAM for core\n");
    return 1;
  }

  // {ioremap} is used to map specific physical memory onto virtual memory.
  core_addr = ioremap(CORE_PHYS_ADDR, CORE_SIZE);
  if (core_addr == NULL) {
    pr_err("Failed to map physical addr %lx\n", (uintptr_t)CORE_PHYS_ADDR);
    return 1;
  }

  memcpy(core_addr, CORE_START, CORE_SIZE);

  // TODO: is a cache flush required on x86 after setting memory to exec?
  if (set_memory_exec(core_addr, CORE_SIZE) != 0) {
    pr_err("Failed to set core memory as executable\n");
    return 1;
  }

  *core_addr_out = core_addr;
  return 0;
}

void unload_core(void* core_addr) {
  iounmap(core_addr);
}
