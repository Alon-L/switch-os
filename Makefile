QEMU ?= qemu-system-x86_64

MODULE_DIR ?= module
MODULE_KO_PATH ?= $(MODULE_DIR)/switch_os.ko

TEST_DIR ?= test
TEST_ROOT_DIR ?= $(TEST_DIR)/qemu_root

QEMU_FLAGS ?= 

ifneq (,$(wildcard ./.env))
  include .env
  export
endif

ifndef PERSISTENT_VM
	QEMU_FLAGS += -snapshot
	FAT_DRIVE_PERM := ronly
else
	FAT_DRIVE_PERM := rw
endif

clean:
	$(MAKE) -C $(MODULE_DIR) clean \
		LINUX_VERSION=$(LINUX_VERSION)

.PHONY: clean

$(MODULE_KO_PATH):
	$(MAKE) -C $(MODULE_DIR) \
		LINUX_VERSION=$(LINUX_VERSION)

# This is required so we always try to compile the module.
# Fortunately the linux's module build-system has a dependency list and will only recompile when needed.
.PHONY: $(MODULE_KO_PATH)

prepare-qemu: $(MODULE_KO_PATH)
	cp -f $^ $(TEST_ROOT_DIR)/
	
qemu: prepare-qemu
	$(QEMU) \
		-m 4G \
		-smp 2 \
		-drive if=pflash,format=raw,file=$(OVMF) \
		-hda $(LINUX_IMAGE) \
		-drive format=raw,file=fat:$(FAT_DRIVE_PERM):$(TEST_ROOT_DIR) \
		-enable-kvm \
		-vga virtio \
		-net nic -net user,hostfwd=tcp::2222-:22 \
		$(QEMU_FLAGS)

.PHONY: prepare-qemu qemu
