#include <Uefi.h>
#include <Library/UefiLib.h>

#include <Debug/Debug.h>
#include <Protocol/SwitchOS.h>

extern EFI_BOOT_SERVICES *gBS;

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
) {
    Print(L"EFI-app SwitchOSTest starts\n");
    EFI_STATUS Status = EFI_SUCCESS;

    SWITCH_OS_PROTOCOL *SwitchOSProtocol;

    Status = gBS->LocateProtocol(&gSwitchOSProtocolGuid, NULL, (VOID **)&SwitchOSProtocol);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = SwitchOSProtocol->SwitchOS(0);
    Print(L"Dispatched SwitchOS service\n");

    return Status;
}
