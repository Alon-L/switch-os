#include <efivar/efivar.h>
#include <fcntl.h>
#include <wchar.h>

#include "error.h"
#include "osl.h"

err_t enable_uefi_privileges(void) {
  return SUCCESS;
}

err_t enable_sleep_privileges(void) {
  return SUCCESS;
}

err_t set_uefi_variable(struct guid* guid, const wchar_t* name, void* data, size_t data_size) {
  err_t err = SUCCESS;

  char name_str[64];
  CHECK(wcsrtombs(name_str, &name, sizeof(name_str), NULL) > 0);

  CHECK(efi_set_variable(*(efi_guid_t*)guid, name_str, data, data_size, 0, 0) == 0);

cleanup:
  return err;
}

err_t enter_sleep_s3(void) {
  err_t err = SUCCESS;
  int power_state_fd = -1;

  power_state_fd = open("/sys/power/state", O_WRONLY);
  CHECK(power_state_fd >= 0);

  static char power_state_mem_str[] = "mem\n";
  CHECK(write(power_state_fd, power_state_mem_str, sizeof(power_state_mem_str)) == sizeof(power_state_mem_str));

cleanup:
  if (power_state_fd >= 0) {
    close(power_state_fd);
  }
  return err;
}
