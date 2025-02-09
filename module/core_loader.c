#include "core_loader.h"

#include <core/consts.h>
#include <core/header.h>
#include <error.h>
#include <linux/io.h>
#include <linux/mm.h>

extern char _binary_build_core_bin_trimmed_start[];
extern char _binary_build_core_bin_trimmed_end[];

extern char _binary_build_rm_bin_trimmed_start[];
extern char _binary_build_rm_bin_trimmed_end[];

extern char _binary_build_pm_bin_trimmed_start[];
extern char _binary_build_pm_bin_trimmed_end[];

#define CORE_START (_binary_build_core_bin_trimmed_start)
#define CORE_SIZE ((uintptr_t)_binary_build_core_bin_trimmed_end - (uintptr_t)_binary_build_core_bin_trimmed_start)

#define CORE_RM_START (_binary_build_rm_bin_trimmed_start)
#define CORE_RM_SIZE ((uintptr_t)_binary_build_rm_bin_trimmed_end - (uintptr_t)_binary_build_rm_bin_trimmed_start)

#define CORE_PM_START (_binary_build_pm_bin_trimmed_start)
#define CORE_PM_SIZE ((uintptr_t)_binary_build_pm_bin_trimmed_end - (uintptr_t)_binary_build_pm_bin_trimmed_start)

/**
 * Writes a given buffer to a specified physical address.
 * @param phys_addr - The physical address to write to.
 * @param buf       - The buffer to write.
 * @param size      - The size of the buffer.
 * @virt_addr_out   - The returned mapped virtual address for the physical address.
 */
static err_t load_phys_memory(uintptr_t phys_addr, void* buf, size_t size, void** virt_addr_out) {
  err_t err = SUCCESS;
  void* virt_addr = NULL;

  // `ioremap` is used to map specific physical memory onto virtual memory.
  virt_addr = ioremap(phys_addr, size);
  CHECK_TRACE(virt_addr != NULL, "Failed to map physical addr %lx\n", (uintptr_t)phys_addr);

  memcpy(virt_addr, buf, size);

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

  CHECK_TRACE(CORE_SIZE <= CORE_MAX_PHYS_MEM_SIZE, "Not enough reserved RAM for core\n");
  CHECK_TRACE(CORE_RM_SIZE <= CORE_MAX_RM_PHYS_MEM_SIZE, "Not enough reserved RAM for core rm\n");
  CHECK_TRACE(CORE_PM_SIZE <= CORE_MAX_PM_PHYS_MEM_SIZE, "Not enough reserved RAM for core pm\n");

  CHECK_RETHROW(load_phys_memory(CORE_PHYS_ADDR, CORE_START, CORE_SIZE, &core_addr));
  CHECK_RETHROW(load_phys_memory(CORE_RM_PHYS_ADDR, CORE_RM_START, CORE_RM_SIZE, NULL));
  CHECK_RETHROW(load_phys_memory(CORE_PM_PHYS_ADDR, CORE_PM_START, CORE_PM_SIZE, NULL));

  core_header = (struct core_header*)core_addr;
  CHECK(is_core_header_magic_valid(core_header));

  *core_header_out = core_header;

cleanup:
  return err;
}

void unload_core(struct core_header* core_header) {
  iounmap(core_header);
}
