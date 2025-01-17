#include <uacpi/kernel_api.h>

#include "core/header.h"
#include "io.h"
#include "mem.h"
#include "trace.h"
#include "uacpi/status.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

extern struct core_header core_header;

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr* out_rsdp_address) {
  *out_rsdp_address = core_header.rsdp;

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_raw_memory_read(uacpi_phys_addr address,
                                          uacpi_u8 byte_width,
                                          uacpi_u64* out_value) {
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

uacpi_status uacpi_kernel_raw_memory_write(uacpi_phys_addr address,
                                           uacpi_u8 byte_width,
                                           uacpi_u64 in_value) {
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

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address,
                                          uacpi_handle* out_handle) {
  memcpy(out_handle, &address, sizeof(address));
  return UACPI_STATUS_OK;
}

void uacpi_kernel_pci_device_close(uacpi_handle handle) {
  return;
}

#define PCI_CONF_ADDR(bus, device, function, offset)               \
  (0x80000000 | ((bus) << 16) | (device << 11) | (function << 8) | \
   (offset & 0xFC))

uacpi_status uacpi_kernel_pci_read(uacpi_handle handle, uacpi_size offset,
                                   uacpi_u8 byte_width, uacpi_u64* value) {
  uacpi_pci_address* address = handle;
  outl(0xCF8,
       PCI_CONF_ADDR(address->bus, address->device, address->function, offset));

  switch (byte_width) {
    case 1: {
      *value = inb(0xCFC);
      break;
    }
    case 2: {
      *value = inw(0xCFC);
      break;
    }
    case 4: {
      *value = inl(0xCFC);
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write(uacpi_handle handle, uacpi_size offset,
                                    uacpi_u8 byte_width, uacpi_u64 value) {
  uacpi_pci_address* address = handle;
  outl(0xCF8,
       PCI_CONF_ADDR(address->bus, address->device, address->function, offset));

  switch (byte_width) {
    case 1: {
      outb((uint8_t)value, 0xCFC);
      break;
    }
    case 2: {
      outw((uint16_t)value, 0xCFC);
      break;
    }
    case 4: {
      outl((uint32_t)value, 0xCFC);
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len,
                                 uacpi_handle* out_handle) {
  *out_handle = (uacpi_handle)(base);
  return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
  return;
}

uacpi_status uacpi_kernel_io_read(uacpi_handle handle, uacpi_size offset,
                                  uacpi_u8 byte_width, uacpi_u64* value) {
  uint16_t p = (uacpi_io_addr)handle + offset;

  switch (byte_width) {
    case 1: {
      *value = inb(p);
      break;
    }
    case 2: {
      *value = inw(p);
      break;
    }
    case 4: {
      *value = inl(p);
      break;
    }
    default:
      return UACPI_STATUS_INVALID_ARGUMENT;
  }

  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write(uacpi_handle handle, uacpi_size offset,
                                   uacpi_u8 byte_width, uacpi_u64 value) {
  uint16_t p = (uacpi_io_addr)handle + offset;

  switch (byte_width) {
    case 1: {
      outb((uint8_t)value, p);
      break;
    }
    case 2: {
      outw((uint16_t)value, p);
      break;
    }
    case 4: {
      outl((uint32_t)value, p);
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

static char buf[1024 * 1024];
static size_t buf_idx = 0;

void* uacpi_kernel_alloc(uacpi_size size) {
  if (buf_idx + size > sizeof(buf)) {
    TRACE("Not enough storage for alloc!\n");
    return NULL;
  }

  void* ptr = &buf[buf_idx];
  buf_idx += size;
  return ptr;
}

void* uacpi_kernel_calloc(uacpi_size count, uacpi_size size) {
  size_t total_size = count * size;

  void* ptr = uacpi_kernel_alloc(total_size);
  if (ptr == NULL) {
    return NULL;
  }

  memset(ptr, 0, total_size);

  return ptr;
}

void uacpi_kernel_free(void* mem) {
  return;
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* log) {
  TRACE(log);
  return;
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void) {
  return 0;
}

void uacpi_kernel_stall(uacpi_u8 usec) {
  return;
}

void uacpi_kernel_sleep(uacpi_u64 msec) {
  return;
}

static char mutex;
static char event;
static char spinlock;

uacpi_handle uacpi_kernel_create_mutex(void) {
  return &mutex;
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
  return;
}

uacpi_handle uacpi_kernel_create_event(void) {
  return &event;
}

void uacpi_kernel_free_event(uacpi_handle handle) {
  return;
}

uacpi_thread_id uacpi_kernel_get_thread_id(void) {
  return 0;
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 val) {
  return UACPI_STATUS_OK;
}

void uacpi_kernel_release_mutex(uacpi_handle handle) {
  return;
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 event) {
  return false;
}

void uacpi_kernel_signal_event(uacpi_handle handle) {
  return;
}

void uacpi_kernel_reset_event(uacpi_handle handle) {
  return;
}

uacpi_status uacpi_kernel_handle_firmware_request(
  uacpi_firmware_request* request) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(
  uacpi_u32 irq, uacpi_interrupt_handler handle, uacpi_handle ctx,
  uacpi_handle* out_irq_handle) {
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(
  uacpi_interrupt_handler handle, uacpi_handle irq_handle) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_handle uacpi_kernel_create_spinlock(void) {
  return &spinlock;
}

void uacpi_kernel_free_spinlock(uacpi_handle handle) {
  return;
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle) {
  return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags flags) {
  return;
}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type type,
                                        uacpi_work_handler handler,
                                        uacpi_handle ctx) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {
  return UACPI_STATUS_UNIMPLEMENTED;
}
