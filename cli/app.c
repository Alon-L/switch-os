#include "app.h"

#include <argtable3.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "commands/help.h"
#include "commands/store.h"
#include "commands/switch.h"

unsigned long trace(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  unsigned long res = vprintf(fmt, args);
  va_end(args);
  return res;
}

struct arg_lit* help_arg;
struct arg_lit* store_arg;
struct arg_lit* switch_arg;
struct arg_end* end_arg;

void register_cmd(const struct cmd* cmd, void* ctx) {
  arg_cmd_register(cmd->name, cmd->handler, cmd->description, ctx);
}

int main(int argc, char* argv[]) {
  int exitcode = EXIT_SUCCESS;

  arg_set_module_name(APP_NAME);
  arg_set_module_version(APP_VER_MAJOR, APP_VER_MINOR, APP_VER_PATCH, APP_VER_TAG);

  arg_cmd_init();
  register_cmd(&g_help_cmd, NULL);
  register_cmd(&g_store_cmd, NULL);
  register_cmd(&g_switch_cmd, NULL);

  arg_dstr_t res = arg_dstr_create();

  if (argc == 1 || arg_cmd_info(argv[1]) == NULL) {
    arg_make_get_help_msg(res);
    printf("%s", arg_dstr_cstr(res));
    goto cleanup;
  }

  exitcode = arg_cmd_dispatch(argv[1], argc, argv, res);
  printf("%s", arg_dstr_cstr(res));

cleanup:
  arg_dstr_destroy(res);
  arg_cmd_uninit();

  return exitcode;
}
