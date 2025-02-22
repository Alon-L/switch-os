Switch between two running operating systems without emulation.

#### ⚠️ The project is a Work In Progress and should not yet be used on real setups.

## Functionality
Switch OS works by loading code into a fixed physical address, and pointing the *ACPI waking vector* to it.
It then enters *suspend to RAM* (ACPI S3 sleep state) and expects the user to wake the machine.
The loaded code now *runs separately from the previous kernel*. It communicates with a disk device and either stores
or switches a dump of the entire RAM on the disk.

## Project Layout
The project contains two parts: core and module.
Module is a lightweight kernel module which loads core and points the ACPI waking vector to it.
Core contains all the functionality for storing and switching the dumps of the RAM.
It runs as a mini-kernel and can not use the previous kernel's functions or data.  
When the module loads core, it also fills the *core header*.
The core header serves as communication between the module and core.

## Future Goals
Switch OS currently only works on Linux. Therefore, it can only switch two running instances of Linux.
A main goal for the project is supporting Windows. Windows is far more difficult to work with than Linux, since it is closed source
and does not feature easy ways to perform some of the actions required for module.  
Another goal is adding tests. The tests should run inside VMs with arbitrary kernels.  
Core currently assumes the disk device to be *virtio-blk*. This assumption allows core to only work in virtualized environments.
A far more complex implementation for either SATA or NVMe is required to work on real hardware.
