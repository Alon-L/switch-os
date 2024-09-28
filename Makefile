QEMU ?= qemu-system-x86_64

CORE_DIR ?= core

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

ifdef GDB
	QEMU_FLAGS += -s -S
endif

clean:
	$(MAKE) -C $(MODULE_DIR) clean
	$(MAKE) -C $(CORE_DIR) clean
	rm -rf $(MODULE_DIR)/*_shipped
	rm -rf $(TEST_ROOT_DIR)

.PHONY: clean

$(MODULE_DIR)/%.o_shipped: $(CORE_DIR)/build/%.o
	cp $^ $@

$(CORE_DIR)/build/core_final.o:
	$(MAKE) -C $(CORE_DIR)

$(MODULE_KO_PATH): $(MODULE_DIR)/core_final.o_shipped
	$(MAKE) -C $(MODULE_DIR)

# This is required so we always try to compile everything.
.PHONY: $(CORE_DIR)/build/core_final.o $(MODULE_KO_PATH)

prepare-qemu: $(MODULE_KO_PATH)
	mkdir -p $(TEST_ROOT_DIR)
	cp -f $^ $(TEST_ROOT_DIR)/
	
qemu: prepare-qemu
	$(QEMU) \
		-m 6G \
		-serial mon:stdio \
		-drive if=pflash,format=raw,file=$(OVMF) \
		-hda $(LINUX_IMAGE) \
		-drive format=raw,file=fat:$(FAT_DRIVE_PERM):$(TEST_ROOT_DIR) \
		-enable-kvm \
		-vga virtio \
		-net nic -net user,hostfwd=tcp::2222-:22 \
		$(QEMU_FLAGS)

.PHONY: prepare-qemu qemu
