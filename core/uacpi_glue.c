#include <uacpi/kernel_api.h>
#include <uacpi/status.h>

#include "alloc.h"
#include "core/header.h"
#include "io.h"
#include "mem.h"
#include "pci.h"
#include "trace.h"
#include "uacpi/types.h"

// Some functions in this file have a dummy implementation that does not use all their parameters.
#pragma GCC diagnostic ignored "-Wunused-parameter"

extern struct core_header g_core_header;

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr* out_rsdp_address) {
  *out_rsdp_address = g_core_header.rsdp;

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_memory_read(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64* out_value) {
  switch (byte_width) {
    case 1: {
      *out_value = *(uint8_t*)address;
      break;
    }
    case 2: {
      *out_value = *(uint16_t*)address;
      break;
    }
    case 4: {
      *out_value = *(uint32_t*)address;
      break;
    }
    case 8: {
      *out_value = *(uint64_t*)address;
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_memory_write(uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 in_value) {
  switch (byte_width) {
    case 1: {
      *(uint8_t*)address = in_value;
      break;
    }
    case 2: {
      *(uint16_t*)address = in_value;
      break;
    }
    case 4: {
      *(uint32_t*)address = in_value;
      break;
    }
    case 8: {
      *(uint64_t*)address = in_value;
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle* out_handle) {
  struct pci_dev_addr* pci_dev_addr = uacpi_kernel_alloc(sizeof(struct pci_dev_addr));

  if (pci_dev_addr == NULL) {
    return UACPI_STATUS_OUT_OF_MEMORY;
  }

  pci_dev_addr->bus = address.bus;
  pci_dev_addr->device = address.device;
  pci_dev_addr->function = address.function;

  *out_handle = pci_dev_addr;

  return UACPI_STATUS_OK;
}

void uacpi_kernel_pci_device_close(uacpi_handle handle) {
  uacpi_kernel_free(handle);
  return;
}

uacpi_status uacpi_kernel_pci_read(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64* value) {
  struct pci_dev_addr* pci_dev_addr = handle;

  switch (byte_width) {
    case 1: {
      *value = pci_read_8(pci_dev_addr, offset);
      break;
    }
    case 2: {
      *value = pci_read_16(pci_dev_addr, offset);
      break;
    }
    case 4: {
      *value = pci_read_32(pci_dev_addr, offset);
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value) {
  struct pci_dev_addr* pci_dev_addr = handle;

  switch (byte_width) {
    case 1: {
      pci_write_8(pci_dev_addr, offset, (uint8_t)value);
      break;
    }
    case 2: {
      pci_write_16(pci_dev_addr, offset, (uint8_t)value);
      break;
    }
    case 4: {
      pci_write_32(pci_dev_addr, offset, (uint8_t)value);
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle* out_handle) {
  *out_handle = (uacpi_handle)(base);
  return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
  return;
}

uacpi_status uacpi_kernel_io_read(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64* value) {
  // `handle` is io base.
  uint16_t p = (uacpi_io_addr)handle + offset;

  switch (byte_width) {
    case 1: {
      *value = in8(p);
      break;
    }
    case 2: {
      *value = in16(p);
      break;
    }
    case 4: {
      *value = in32(p);
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write(uacpi_handle handle, uacpi_size offset, uacpi_u8 byte_width, uacpi_u64 value) {
  // `handle` is io base.
  uint16_t p = (uacpi_io_addr)handle + offset;

  switch (byte_width) {
    case 1: {
      out8(p, (uint8_t)value);
      break;
    }
    case 2: {
      out16(p, (uint16_t)value);
      break;
    }
    case 4: {
      out32(p, (uint32_t)value);
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }

  return UACPI_STATUS_OK;
}

void* uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
  return (void*)addr;
}

void uacpi_kernel_unmap(void* addr, uacpi_size len) {
  return;
}

void* uacpi_kernel_alloc(uacpi_size size) {
  return core_malloc(size);
}

void* uacpi_kernel_calloc(uacpi_size count, uacpi_size size) {
  return core_calloc(count, size);
}

void uacpi_kernel_free(void* mem) {
  core_free(mem);
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* log) {
  if (level <= UACPI_LOG_WARN) {
    TRACE("%s", log);
  }
  return;
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void) {
  // Unimplemented.
  return 0;
}

void uacpi_kernel_stall(uacpi_u8 usec) {
  // Unimplemented.
  return;
}

void uacpi_kernel_sleep(uacpi_u64 msec) {
  // Unimplemented.
  return;
}

uacpi_handle uacpi_kernel_create_mutex(void) {
  return core_malloc(1);
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
  core_free((void*)handle);
}

uacpi_handle uacpi_kernel_create_event(void) {
  return core_malloc(1);
}

void uacpi_kernel_free_event(uacpi_handle handle) {
  core_free((void*)handle);
}

uacpi_thread_id uacpi_kernel_get_thread_id(void) {
  // We run single threaded.
  return 0;
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 val) {
  // Unimplemented.
  return UACPI_STATUS_OK;
}
void uacpi_kernel_release_mutex(uacpi_handle handle) {
  // Unimplemented.
  return;
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 event) {
  // Unimplemented.
  return false;
}

void uacpi_kernel_signal_event(uacpi_handle handle) {
  // Unimplemented.
  return;
}

void uacpi_kernel_reset_event(uacpi_handle handle) {
  // Unimplemented.
  return;
}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request* request) {
  // Unimplemented.
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler handle, uacpi_handle ctx,
                                                    uacpi_handle* out_irq_handle) {
  // Technically unimplemented, but we need to return UACPI_STATUS_OK for uACPI initialization to succeed.
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler handle, uacpi_handle irq_handle) {
  // Unimplemented.
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_handle uacpi_kernel_create_spinlock(void) {
  return core_malloc(1);
}

void uacpi_kernel_free_spinlock(uacpi_handle handle) {
  core_free((void*)handle);
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle) {
  // Unimplemented.
  return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags flags) {
  // Unimplemented.
  return;
}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type type, uacpi_work_handler handler, uacpi_handle ctx) {
  // Unimplemented.
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {
  // Unimplemented.
  return UACPI_STATUS_UNIMPLEMENTED;
}
