# Fetch an environmental file if exists.
ifneq (,$(wildcard ./.env))
  include .env
  export
endif

export ROOT_DIR := $(CURDIR)
export VPATH := $(ROOT_DIR)

export CC ?= gcc
export LD ?= ld
export OBJCOPY ?= objcopy
export IASL ?= iasl

QEMU ?= qemu-system-x86_64
QEMU_ADDITIONAL_FLAGS ?=

# `VM_GDB` allows to connect to the qemu VM via gdb and debug it. It also compiles core's parts with debugging information.
# See guide for connecting to qemu via gdb here: https://qemu-project.gitlab.io/qemu/system/gdb.html
ifdef VM_GDB
	QEMU_ADDITIONAL_FLAGS += -s -S
	CORE_GCC_DEBUG := 1
endif

# `QEMU_DEBUG` turns on core's debug traces and outputs them into `QEMU_DEBUGCON_FILE_PATH`.
ifdef QEMU_DEBUG
	QEMU_DEBUGCON_FILE_PATH ?= /var/log/switch-os.log
	QEMU_ADDITIONAL_FLAGS += -debugcon file:$(QEMU_DEBUGCON_FILE_PATH)
	TRACE_DEBUG := 1
endif

clean:
	rm -rf build/*
	$(MAKE) -C core clean
	$(MAKE) -C uefi clean
	$(MAKE) -C cli clean

.PHONY: clean

uefi/obj/%.o: core/build/%.o
	mkdir -p $(dir $@)
	cp $^ $@

core/build/core.o:
	$(MAKE) -C core

uefi/build/app.efi: uefi/obj/core.o
	$(MAKE) -C uefi

.PHONY: core/build/core.o uefi/build/app.efi

build/efi: uefi/build/app.efi
	mkdir -p $@
	mkdir -p $@/EFI/BOOT
	cp -f $^ $@/EFI/BOOT/BOOTX64.efi

build/cli:
	$(MAKE) -C cli
	@rm -rf $@
	@cp -r cli/build $@

build: build/efi build/cli

.PHONY: build build/efi build/cli

qemu: build
	$(QEMU) \
		-m 2G \
		-serial mon:stdio \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
		-drive if=pflash,format=raw,file=$(OVMF_VARS) \
		-hda fat:rw:build/efi \
		-hdb fat:rw:$(LINUX_DISK_PATH) \
		-drive id=buffer_drive,file=$(BUFFER_DRIVE_IMG),if=none,format=raw -device virtio-blk-pci,drive=buffer_drive \
		-enable-kvm \
		-vga virtio \
		$(QEMU_ADDITIONAL_FLAGS)

.PHONY: qemu
