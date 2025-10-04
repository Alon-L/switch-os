#include <stdbool.h>
#include <stdio.h>
// clang-format off
#include <windows.h>
#include <winerror.h>
#include <winnt.h>
#include <powrprof.h>
// clang-format on

#include "error.h"
#include "osl.h"
#include "trace.h"

static err_t set_privilege(HANDLE token, char* privilege, bool enable_privilege) {
  err_t err = SUCCESS;
  TOKEN_PRIVILEGES tp;
  LUID luid;

  CHECK_TRACE(LookupPrivilegeValue(NULL, privilege, &luid) != 0, "Failed to find the privilege value for %s\n",
              privilege);

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = enable_privilege ? SE_PRIVILEGE_ENABLED : 0;

  // Enable the privilege or disable all privileges.
  CHECK_TRACE(AdjustTokenPrivileges(token, false, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL) != 0,
              "Failed to adjust the token's privileges\n");

  CHECK_TRACE(GetLastError() != ERROR_NOT_ALL_ASSIGNED, "The token does not have the specified privilege\n");

cleanup:
  return err;
}

err_t enable_uefi_privileges(void) {
  err_t err = SUCCESS;
  HANDLE process_token = INVALID_HANDLE_VALUE;

  CHECK(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &process_token) != 0);

  // `SetFirmwareEnvironmentVariableW` requires the `SE_SYSTEM_ENVIRONMENT_NAME` privilege.
  CHECK_RETHROW(set_privilege(process_token, SE_SYSTEM_ENVIRONMENT_NAME, true));

cleanup:
  if (process_token != INVALID_HANDLE_VALUE) {
    CloseHandle(process_token);
  }
  return err;
}

err_t enable_sleep_privileges(void) {
  err_t err = SUCCESS;
  HANDLE process_token = INVALID_HANDLE_VALUE;

  CHECK(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &process_token) != 0);

  // `SetSuspendState` requires the `SE_SHUTDOWN_NAME` privilege.
  CHECK_RETHROW(set_privilege(process_token, SE_SHUTDOWN_NAME, true));

cleanup:
  if (process_token != INVALID_HANDLE_VALUE) {
    CloseHandle(process_token);
  }
  return err;
}

/**
 * Converts a guid given in the abstract `struct guid` to WinAPI's string format, so it can be used with
 * `SetFirmwareEnvironmentVariableW`.
 */
static err_t guid_to_string(struct guid* guid, wchar_t* str, size_t str_size) {
  err_t err = SUCCESS;

  CHECK(StringFromGUID2((REFGUID)guid, str, str_size) != 0);

cleanup:
  return err;
}

err_t set_uefi_variable(struct guid* guid, const wchar_t* name, void* data, size_t data_size) {
  err_t err = SUCCESS;

  wchar_t guid_str[64];
  CHECK_RETHROW(guid_to_string(guid, guid_str, sizeof(guid_str)));

  CHECK(SetFirmwareEnvironmentVariableW(name, guid_str, data, data_size) != 0);

cleanup:
  return err;
}

err_t enter_sleep_s3(void) {
  err_t err = SUCCESS;

  CHECK(SetSuspendState(FALSE, FALSE, FALSE) != 0);

cleanup:
  return err;
}
