#include "core_loader.h"
#include "core/consts.h"
#include "core/header.h"
#include "linux/io.h"
#include "linux/mm.h"

extern char _binary_build_core_bin_trimmed_start[];
extern char _binary_build_core_bin_trimmed_end[];

extern char _binary_build_rm_bin_trimmed_start[];
extern char _binary_build_rm_bin_trimmed_end[];

#define CORE_START (_binary_build_core_bin_trimmed_start)
#define CORE_SIZE                                  \
  ((uintptr_t)_binary_build_core_bin_trimmed_end - \
   (uintptr_t)_binary_build_core_bin_trimmed_start)

#define CORE_RM_START (_binary_build_rm_bin_trimmed_start)
#define CORE_RM_SIZE                             \
  ((uintptr_t)_binary_build_rm_bin_trimmed_end - \
   (uintptr_t)_binary_build_rm_bin_trimmed_start)

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

/*
 * Allocates executable physical memory [`phys_addr`, `phys_addr` + `size`) and copies
 * `buf` into it.
 * Sets `virt_addr_out` (if not NULL) to the virtual address that maps `phys_addr`.
 */
static err_t load_exec_phys_memory(uintptr_t phys_addr, void* buf, size_t size,
                                   void** virt_addr_out) {
  err_t err = SUCCESS;
  void* virt_addr = NULL;

  // `ioremap` is used to map specific physical memory onto virtual memory.
  virt_addr = ioremap(phys_addr, size);
  CHECK_TRACE(virt_addr != NULL, "Failed to map physical addr %lx\n",
              (uintptr_t)phys_addr);

  memcpy(virt_addr, buf, size);

  // TODO: is a cache flush required on x86 after setting memory to exec?
  CHECK_RETHROW(set_memory_exec(virt_addr, size));

  if (virt_addr_out) {
    *virt_addr_out = virt_addr;
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
  CHECK_TRACE(CORE_RM_SIZE <= CORE_MAX_RM_PHYS_MEM_SIZE,
              "Not enough reserved RAM for core rm\n");

  CHECK_RETHROW(
      load_exec_phys_memory(CORE_PHYS_ADDR, CORE_START, CORE_SIZE, &core_addr));
  CHECK_RETHROW(load_exec_phys_memory(
      CORE_RM_PHYS_ADDR, CORE_RM_START, CORE_RM_SIZE, NULL));

  core_header = (struct core_header*)core_addr;
  CHECK(is_core_header_magic_valid(core_header));

  *core_header_out = core_header;

cleanup:
  return err;
}

void unload_core(struct core_header* core_header) {
  iounmap(core_header);
}
