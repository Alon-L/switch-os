#include "store.h"

#include <argtable3.h>
#include <stdint.h>
#include <stdlib.h>

#include "commands.h"
#include "core/header.h"
#include "osl.h"
#include "utils.h"

/**
 * Performs the store operation. This updates the `core_header->action` value to `CORE_ACTION_STORE` so next time an S3
 * occurs, core will store the current state of the OS to the disk.
 *
 * @param is_lazy If false, the function will immediately enter S3 after updating `core_header->action` to switch the
 * OS's state. If true, the S3 doesn't occur, so the switch will happen next time the user enters S3.
 */
static err_t execute_store_op(bool is_lazy) {
  err_t err = SUCCESS;

  CHECK_RETHROW(enable_uefi_privileges());
  CHECK_RETHROW(enable_sleep_privileges());

  enum core_action action = CORE_ACTION_STORE;
  CHECK_RETHROW(send_uefi_command(L"SET_CORE_ACTION", &action, sizeof(action)));

  if (!is_lazy) {
    CHECK_RETHROW_TRACE(enter_sleep_s3(), "Failed to enter sleep S3\n");
  }

cleanup:
  return err;
}

static int store_handler(int argc, char* argv[], arg_dstr_t res, UNUSED_PARAM void* ctx) {
  int exitcode = 0;

  arg_str_t* cmd = arg_str1(NULL, NULL, g_store_cmd.name, NULL);
  arg_str_t* cmd_name = arg_str0(NULL, NULL, "<command>", NULL);
  arg_lit_t* help = arg_lit0("h", "help", "Output usage information");
  arg_lit_t* lazy = arg_lit0(
    NULL, "lazy",
    "Don't immediately enter suspend mode. The store will happen the next time the user enters suspend themselves");
  arg_end_t* end = arg_end(20);
  void* argtable[] = {cmd, cmd_name, help, lazy, end};

  if (arg_nullcheck(argtable) != 0) {
    exitcode = 1;
    goto cleanup;
  }

  int nerrors = arg_parse(argc, argv, argtable);
  if (arg_make_syntax_err_help_msg(res, g_store_cmd.name, help->count, nerrors, argtable, end, &exitcode) != 0) {
    goto cleanup;
  }

  bool is_lazy = lazy->count > 0;
  if (!IS_SUCCESS(execute_store_op(is_lazy))) {
    exitcode = EXIT_FAILURE;
    goto cleanup;
  }

cleanup:
  arg_freetable(argtable, ARRAY_SIZE(argtable));
  return exitcode;
}

struct cmd g_store_cmd = {
  .name = "store",
  .description =
    "Store the current state of the OS to the disk. This command immediately enters sleep (S3) unless the --lazy flag "
    "is passed.",
  .handler = store_handler,
};
