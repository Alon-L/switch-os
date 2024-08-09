#include <Uefi.h>

#include <Debug/Debug.h>
#include <Protocol/SwitchOS.h>

extern EFI_BOOT_SERVICES *gBS;

EFI_STATUS
EFIAPI
SwitchOS(
    IN UINTN    Arg
) {
    EFI_STATUS Status = EFI_SUCCESS;
    Print(L"Inside SwitchOS service...\n");
    return Status;
}

EFI_STATUS
EFIAPI
UefiDriverUnload (
  IN EFI_HANDLE ImageHandle
) {
    return EFI_SUCCESS;
}

/**
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiDriverMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
) {
    EFI_STATUS Status = EFI_SUCCESS;
    Print(L"Driver starts..\n");

    SWITCH_OS_PROTOCOL *SwitchOSProtocol;

    Status = gBS->AllocatePool(EfiRuntimeServicesData, sizeof(SWITCH_OS_PROTOCOL), (VOID**)&SwitchOSProtocol);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    SwitchOSProtocol->SwitchOS = SwitchOS;

    Status = gBS->InstallProtocolInterface(
      &ImageHandle,
      &gSwitchOSProtocolGuid,
      EFI_NATIVE_INTERFACE,
      SwitchOSProtocol
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Print(L"Driver finishes..\n");
    return Status;
}

