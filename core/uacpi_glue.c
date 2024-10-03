#include <uacpi/kernel_api.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rdsp_address) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_raw_memory_read(
    uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_raw_memory_write(
    uacpi_phys_addr address, uacpi_u8 byte_width, uacpi_u64 in_value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_raw_io_read(
    uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 *out_value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_raw_io_write(
    uacpi_io_addr address, uacpi_u8 byte_width, uacpi_u64 in_value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_read(
    uacpi_pci_address *address, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 *value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_pci_write(
    uacpi_pci_address *address, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_map(
    uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
  return;
}

uacpi_status uacpi_kernel_io_read(
    uacpi_handle, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 *value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_io_write(
    uacpi_handle, uacpi_size offset,
    uacpi_u8 byte_width, uacpi_u64 value
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
  return NULL;
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
  return;
}

void *uacpi_kernel_alloc(uacpi_size size) {
  return NULL;
}

void *uacpi_kernel_calloc(uacpi_size count, uacpi_size size) {
  return NULL;
}

void uacpi_kernel_free(void *mem) {
  return;
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* log) {
  return;
}

uacpi_u64 uacpi_kernel_get_ticks(void) {
  return 0;
}

void uacpi_kernel_stall(uacpi_u8 usec) {
  return;
}

void uacpi_kernel_sleep(uacpi_u64 msec) {
  return;
}

uacpi_handle uacpi_kernel_create_mutex(void) {
  return 0;
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
  return;
}

uacpi_handle uacpi_kernel_create_event(void) {
  return 0;
}

void uacpi_kernel_free_event(uacpi_handle handle) {
  return;
}

uacpi_thread_id uacpi_kernel_get_thread_id(void) {
  return 0;
}

uacpi_bool uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 val) {
  return false;
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

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request* request) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(
    uacpi_u32 irq, uacpi_interrupt_handler handle, uacpi_handle ctx,
    uacpi_handle *out_irq_handle
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(
    uacpi_interrupt_handler handle, uacpi_handle irq_handle
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_handle uacpi_kernel_create_spinlock(void) {
  return NULL;
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

uacpi_status uacpi_kernel_schedule_work(
    uacpi_work_type type, uacpi_work_handler handler, uacpi_handle ctx
) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {
  return UACPI_STATUS_UNIMPLEMENTED;
}
