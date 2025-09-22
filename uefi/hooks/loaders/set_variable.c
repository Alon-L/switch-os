#include "set_variable.h"

#include <efi.h>
#include <efilib.h>
#include <stdint.h>

#include "acpi/tables.h"
#include "hooks/headers.h"
#include "hooks/hooks_loader.h"

DECLARE_HOOK_BINARY(set_variable);

struct loaded_hook g_loaded_set_variable;

static EFI_SET_VARIABLE g_original_set_variable = NULL;

err_t hook_set_variable(void) {
  err_t err = SUCCESS;

  CHECK_RETHROW(load_hook(HOOK_START(set_variable), HOOK_SIZE(set_variable), &g_loaded_set_variable));

  *(struct set_variable_hook_header*)g_loaded_set_variable.header = (struct set_variable_hook_header){
    .original_set_variable = gRT->SetVariable,
    .waking_vector_addr = &g_facs->firmware_waking_vector,
  };

  g_original_set_variable = gRT->SetVariable;

  gRT->SetVariable = (EFI_SET_VARIABLE)g_loaded_set_variable.entry;
  SetCrc(&gRT->Hdr);

cleanup:
  return err;
}

void unhook_set_variable(void) {
  if (g_loaded_set_variable.is_loaded) {
    gRT->SetVariable = g_original_set_variable;
    SetCrc(&gRT->Hdr);

    free_hook(&g_loaded_set_variable, HOOK_SIZE(set_variable));
  }
}
