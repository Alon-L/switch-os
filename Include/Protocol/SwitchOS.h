#ifndef EDK2_PROTOCOL_SWITCHOS_H
#define EDK2_PROTOCOL_SWITCHOS_H

#include <Uefi.h>

extern EFI_GUID gSwitchOSProtocolGuid;

typedef struct {
    EFI_STATUS (EFIAPI *SwitchOS) (IN UINTN Arg);
} SWITCH_OS_PROTOCOL;

#endif //EDK2_PROTOCOL_SWITCHOS_H
