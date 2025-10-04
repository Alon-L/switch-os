#include "help.h"

#include <argtable3.h>
#include <stdint.h>
#include <stdlib.h>

#include "app.h"
#include "commands.h"
#include "utils.h"

/**
 * Lists all the subcommands of this application along with their descriptions into the `res` dynamic string.
 */
static void help_list_commands(arg_dstr_t res) {
  arg_dstr_cat(res, "Usage:\n");
  arg_dstr_catf(res, "  %s <command> [options] [args]\n\n", APP_NAME);
  arg_dstr_cat(res, "Available commands:\n");

  arg_cmd_itr_t itr = arg_cmd_itr_create();
  do {
    arg_cmd_info_t* cmd_info = arg_cmd_itr_value(itr);
    arg_dstr_catf(res, "  %-23s  %s\n", cmd_info->name, cmd_info->description);
  } while (arg_cmd_itr_advance(itr));
  arg_cmd_itr_destroy(itr);
}

/**
 * Retrieves the detailed help information of a specific subcommand by calling its handler with `--help`, and filling
 * the dynamic string `res` with the result.
 */
static int help_command(arg_dstr_t res, const char* cmd_name) {
  if (arg_cmd_info(cmd_name) == NULL) {
    arg_dstr_catf(res, "Unknown command: %s\n", cmd_name);
    arg_make_get_help_msg(res);
    return EXIT_FAILURE;
  }

  arg_cmd_info_t* cmd_info = arg_cmd_info(cmd_name);
  char* cmd_argv[] = {APP_NAME, cmd_info->name, "--help"};
  return cmd_info->proc(ARRAY_SIZE(cmd_argv), cmd_argv, res, NULL);
}

static int help_handler(int argc, char* argv[], arg_dstr_t res, UNUSED_PARAM void* ctx) {
  int exitcode = 0;

  arg_str_t* cmd = arg_str1(NULL, NULL, g_help_cmd.name, NULL);
  arg_str_t* cmd_name = arg_str0(NULL, NULL, "<command>", NULL);
  arg_lit_t* help = arg_lit0("h", "help", "Output usage information");
  arg_end_t* end = arg_end(20);
  void* argtable[] = {cmd, cmd_name, help, end};

  if (arg_nullcheck(argtable) != 0) {
    exitcode = EXIT_FAILURE;
    goto cleanup;
  }

  int nerrors = arg_parse(argc, argv, argtable);
  if (arg_make_syntax_err_help_msg(res, g_help_cmd.name, help->count, nerrors, argtable, end, &exitcode)) {
    goto cleanup;
  }

  // The help command either receives another subcommand and prints specific information about it, or lists all the
  // subcommands with their descriptions.
  if (cmd_name->count == 0) {
    help_list_commands(res);
  } else {
    exitcode = help_command(res, cmd_name->sval[0]);
  }

cleanup:
  arg_freetable(argtable, ARRAY_SIZE(argtable));
  return exitcode;
}

struct cmd g_help_cmd = {
  .name = "help",
  .description = "Output usage information",
  .handler = help_handler,
};
