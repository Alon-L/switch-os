# Fetch an environmental file if exists.
ifneq (,$(wildcard ./.env))
  include .env
  export
endif

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
	DEBUG := 1
endif

clean:
	$(MAKE) -C module clean
	$(MAKE) -C core clean
	$(MAKE) -C uefi clean
	rm -rf module/*_shipped
	rm -rf $(VM_MOUNT_DIR)

.PHONY: clean

module/%.o_shipped: core/build/%.o
	touch module/.$*.o.cmd
	cp $^ $@

uefi/obj/%.o: core/build/%.o
	cp $^ $@

core/build/core.o:
	$(MAKE) -C core

module/switch_os.ko: module/core.o_shipped
	$(MAKE) -C module

uefi/build/app.efi: uefi/obj/core.o
	$(MAKE) -C uefi

.PHONY: core/build/core.o module/switch_os.ko uefi/build/app.efi

build/vm_mount: module/switch_os.ko
	mkdir -p $@
	cp -f $^ $@

build/efi: uefi/build/app.efi
	mkdir -p $@
	mkdir -p $@/EFI/BOOT
	cp -f $^ $@/EFI/BOOT/BOOTX64.efi

build: build/vm_mount build/efi

qemu: build
	$(QEMU) \
		-m 2G \
		-serial mon:stdio \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
		-drive if=pflash,format=raw,file=$(OVMF_VARS) \
		-hda fat:rw:build/efi \
		-hdb fat:rw:$(LINUX_DISK_PATH) \
		-virtfs local,path=build/vm_mount,mount_tag=qemu_root,security_model=passthrough,id=qemu_root,readonly=on \
		-drive id=buffer_drive,file=$(BUFFER_DRIVE_IMG),if=none,format=raw -device virtio-blk-pci,drive=buffer_drive \
		-enable-kvm \
		-vga virtio \
		$(QEMU_ADDITIONAL_FLAGS)

.PHONY: qemu
