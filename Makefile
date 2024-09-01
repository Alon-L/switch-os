QEMU ?= qemu-system-x86_64

DRIVER_ARCH ?= X64
DRIVER_DIR ?= SwitchOSPkg
DRIVER_BUILD_DIR ?= $(PWD)/$(DRIVER_DIR)/build/$(DRIVER_ARCH)

TEST_DIR ?= test
TEST_ROOT_DIR ?= $(TEST_DIR)/qemu_root

ifneq (,$(wildcard ./.env))
  include .env
  export
endif

clean:
	$(MAKE) -C $(DRIVER_DIR) clean \
		DRIVER_BUILD_DIR=$(DRIVER_BUILD_DIR)

.PHONY: clean

$(DRIVER_BUILD_DIR)/%.efi:
	$(MAKE) -C $(DRIVER_DIR) \
		ARCH=$(DRIVER_ARCH) \
		DRIVER_BUILD_DIR=$(DRIVER_BUILD_DIR)

prepare-qemu: $(DRIVER_BUILD_DIR)/SwitchOS.efi
	mkdir -p $(TEST_ROOT_DIR)/efi/boot/
	cp -f $^ $(TEST_ROOT_DIR)/efi/boot/

qemu: prepare-qemu
	$(QEMU) \
		-m 4G \
		-smp 2 \
		-drive if=pflash,format=raw,file=$(OVMF) \
		-hda $(LINUX_IMAGE) \
		-drive format=raw,file=fat:ronly:$(TEST_ROOT_DIR) \
		-boot menu=on \
		-enable-kvm \
		-vga virtio \
		-net nic -net user,hostfwd=tcp::2222-:22 \
		-snapshot

.PHONY: prepare-qemu qemu
