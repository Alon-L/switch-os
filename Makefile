# Fetch an environmental file if exists.
ifneq (,$(wildcard ./.env))
  include .env
  export
endif

# This directory is added to the VM as a virtfs device. It contains the final kernel module.
VM_MOUNT_DIR ?= build

QEMU ?= qemu-system-x86_64
QEMU_ADDITIONAL_FLAGS ?=
# TODO: Replace the constant values in `-append` with configurable ones
QEMU_APPEND_FLAGS ?= console=ttyS0 memmap=64M$$1G,4K$$4K

# `VM_GDB` allows to connect to the qemu VM via gdb and debug it. It also compiles core's parts with debugging information.
# See guide for connecting to qemu via gdb here: https://qemu-project.gitlab.io/qemu/system/gdb.html
ifdef VM_GDB
	QEMU_ADDITIONAL_FLAGS += -s -S
	CORE_GCC_DEBUG_INFO := 1
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
	rm -rf module/*_shipped
	rm -rf $(VM_MOUNT_DIR)

.PHONY: clean

module/%.o_shipped: core/build/%.o
	cp $^ $@

core/build/core.o:
	$(MAKE) -C core

module/switch_os.ko: module/core.o_shipped
	$(MAKE) -C module

.PHONY: core/build/core.o module/switch_os.ko

$(VM_MOUNT_DIR): module/switch_os.ko
	mkdir -p $@
	cp -f $^ $@

qemu: $(VM_MOUNT_DIR)
	$(QEMU) \
		-m 2G \
		-serial mon:stdio \
		-drive if=pflash,format=raw,file=$(OVMF) \
		-kernel $(LINUX_IMAGE) \
		-initrd $(LINUX_INITRD) \
		-append '$(QEMU_APPEND_FLAGS)' \
		-virtfs local,path=$(VM_MOUNT_DIR),mount_tag=qemu_root,security_model=passthrough,id=qemu_root,readonly=on \
		-enable-kvm \
		-vga virtio \
		$(QEMU_ADDITIONAL_FLAGS)

.PHONY: qemu
