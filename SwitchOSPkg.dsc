[Defines]
  PLATFORM_NAME                  = SwitchOSPkg
  PLATFORM_GUID                  = ca24de84-7d31-4549-81dd-c4c3c90d2c98
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SwitchOSPkg
  SUPPORTED_ARCHITECTURES        = IA32|IPF|X64|EBC|ARM
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugLib|MdePkg/Library/UefiDebugLibStdErr/UefiDebugLibStdErr.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf

[Components]
  SwitchOSPkg/Applications/SwitchOSTest/SwitchOSTest.inf
  SwitchOSPkg/Drivers/SwitchOS/SwitchOS.inf

[Components.IA32]

[Components.X64]

[Components.IPF]

[Components.EBC]

[Components.ARM]

