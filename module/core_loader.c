#include "core_loader.h"
#include "core/consts.h"
#include "core/header.h"
#include "linux/io.h"
#include "linux/mm.h"

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
static err_t set_memory_exec(void* start, size_t size) {
  err_t err = SUCCESS;
  void* addr;
  pte_t* addr_pte;
  unsigned int level;

  addr = start;
  for (; (uintptr_t)addr < (uintptr_t)start + size; addr += PAGE_SIZE) {
    addr_pte = lookup_address((uintptr_t)addr, &level);
    CHECK_TRACE(addr_pte != NULL, "Failed to find the pte of address: %lx",
                (uintptr_t)addr);

    set_pte(addr_pte, pte_mkexec(*addr_pte));
  }

cleanup:
  return err;
}

err_t load_core(struct core_header** core_header_out) {
  err_t err = SUCCESS;
  void* core_addr;
  struct core_header* core_header;

  CHECK_TRACE(CORE_SIZE <= CORE_MAX_PHYS_MEM_SIZE,
              "Not enough reserved RAM for core\n");

  // {ioremap} is used to map specific physical memory onto virtual memory.
  core_addr = ioremap(CORE_PHYS_ADDR, CORE_SIZE);
  CHECK_TRACE(core_addr != NULL, "Failed to map physical addr %lx\n",
              (uintptr_t)CORE_PHYS_ADDR);

  memcpy(core_addr, CORE_START, CORE_SIZE);

  // TODO: is a cache flush required on x86 after setting memory to exec?
  CHECK_RETHROW(set_memory_exec(core_addr, CORE_SIZE));

  core_header = (struct core_header*)(core_addr + CORE_HEADER_OFFSET);
  CHECK(is_core_header_magic_valid(core_header));

  *core_header_out = core_header;

cleanup:
  return err;
}

void unload_core(struct core_header* core_header) {
  iounmap(core_header_get_loaded_addr(core_header));
}
