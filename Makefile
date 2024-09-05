QEMU ?= qemu-system-x86_64

CORE_DIR ?= core
CORE_OBJ ?= core.o
CORE_OBJ_PATH ?= $(CORE_DIR)/build/$(CORE_OBJ)

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
	$(MAKE) -C $(CORE_DIR) clean \
		CORE_OBJ=$(CORE_OBJ)
	rm -rf $(MODULE_DIR)/$(CORE_OBJ)_shipped
	rm -rf $(TEST_ROOT_DIR)

.PHONY: clean

$(CORE_OBJ_PATH):
	$(MAKE) -C $(CORE_DIR) \
		CORE_OBJ=$(CORE_OBJ)

$(MODULE_KO_PATH): $(CORE_OBJ_PATH)
	cp $^ $(MODULE_DIR)/$(CORE_OBJ)_shipped
	$(MAKE) -C $(MODULE_DIR) \
		LINUX_VERSION=$(LINUX_VERSION) \
		CORE_OBJ=$(CORE_OBJ)

# This is required so we always try to compile everything.
.PHONY: $(CORE_OBJ_PATH) $(MODULE_KO_PATH)

prepare-qemu: $(CORE_OBJ_PATH) $(MODULE_KO_PATH)
	mkdir -p $(TEST_ROOT_DIR)
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
