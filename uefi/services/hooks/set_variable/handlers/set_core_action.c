#include "set_core_action.h"

#include "core/header.h"
#include "services/headers.h"

extern struct set_variable_hook_header g_hook_header;

err_t set_core_action_handler(void* data, size_t data_size) {
  err_t err = SUCCESS;

  CHECK_TRACE(data_size == sizeof(enum core_action), "Invalid data size! Expecting data of type `core_action`\n");

  enum core_action action = *(enum core_action*)data;
  CHECK_TRACE(is_core_header_action_valid(action), "Invalid action value\n");

  g_hook_header.core_header->action = action;

cleanup:
  return err;
}
