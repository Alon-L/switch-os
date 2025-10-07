# SwitchOS

Switch between two running operating systems without losing their state.

[![Build CI](https://github.com/Alon-L/switch-os/actions/workflows/build.yaml/badge.svg)](https://github.com/Alon-L/switch-os/actions/workflows/build.yaml)

> [!WARNING]
> While the project works on a test setup, it is still a work in progress and should not yet be used on real setups.

## About

The project's goal is to eliminate the problem of losing state when dual-booting and create a seamless transition between operating systems.
SwitchOS allows taking "snapshots" of the currently running OS, and then switch between these snapshots, even across multiple OS's.

### How does it work?

SwitchOS ships in two parts: an EFI application which loads before the bootloader and seamlessly lives along the OS, and a simple usermode CLI application for controlling it.
The EFI application is responsible for creating the snapshots on command, and accepting commands from the CLI application.
The CLI application communicates with the EFI application by sending commands for creating and switching between snapshots.

#### What are snapshots and how are they taken?

In its simplest form, a snapshot consists of a dump of the entire usable RAM, which allegedly encapsulates the entire state of the currently running OS.
The word allegedly is used, since the state of the running OS also includes the state of all physical hardware, which isn't represented in the RAM dump.
Therefore, the snapshot is taken at a time when the state of the physical hardware is completely known.
Such time is right before the machine enters _S3 sleep_ (suspend to RAM).
The definition of S3 states that (almost) all physical hardware except the RAM lose their power, and thus lose their state.
At S3 entry, the OS expects all physical hardware to lose all its state, which is the perfect state to create a memory dump.

The EFI application spreads itself to multiple strategic locations in the suspend entry procedure to ensure it catches the suspension flow, and executes right when the machine awakens, prior to the OS.
The snapshot is created or switched (depending on the CLI request) at that point - right after waking from S3, before the original OS starts executing.
After the snapshot is created or switched, our code enters S3 again and awakens the original OS as usual, which completes the snapshot flow.

#### Where are snapshots stored?

A designated disk is selected to hold the snapshots when they are created or switched.
Currently, the entirety of the disk is designated for this purpose, and the used disk is non-configurable (see [Future](future)).

## Current State

The currently supported OS's include Windows (Windows 10 release 1607 onwards & Windows 11) and Linux.
Currently, the only supported type of disk for storing snapshots is virtio-blk (part of the virtio virtual devices simulated by QEMU), though support for additional disk devices is planned to be added (see [Future](future)).

Only x86-64 is supported, and there are no plans to support other architectures.

## Future

While the project works on a test setup of Windows and Linux dual-boot, the project is still a work in progress and requires additional important features:

- Better support for the snapshots disk:
  - Support additional disk types. Specifically NVME and SATA.
  - The ability to choose the designated snapshots disk.
  - Parse the partitions table of the disk and only use a specific partition.
- Create an installer for the EFI application as part of the CLI.
- Add functionality tests.
