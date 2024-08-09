ARCH ?= X64
CC := GCC5

QEMU ?= qemu-system-x86_64

PACKAGE_NAME := SwitchOSPkg
PACKAGE_DSC := $(PACKAGE_NAME)/$(PACKAGE_NAME).dsc

BUILD_DIR ?= ../Build
TEST_ROOT_DIR ?= test/root

ifeq (, $(shell which build))
$(error "No build command found. Run '. ../edksetup.sh'")
endif

.default: all

all:
	build -a $(ARCH) -t $(CC) -p $(PACKAGE_DSC)

clean:
	build clean

cleanall:
	build cleanall

.PHONY: all clean cleanall

prepare-qemu:
	mkdir -p $(TEST_ROOT_DIR)/efi/boot/
	cp -f $(BUILD_DIR)/$(PACKAGE_NAME)/DEBUG_$(CC)/$(ARCH)/*.efi $(TEST_ROOT_DIR)/efi/boot/

qemu: prepare-qemu
	$(QEMU) \
		-m 4G \
		-smp 2 \
		-drive if=pflash,format=raw,file=$(OVMF) \
		-hda $(LINUX_IMAGE) \
		-drive format=raw,file=fat:rw:$(TEST_ROOT_DIR) \
		-boot menu=on
		-enable-kvm \
		-vga virtio \
		-net nic -net user,hostfwd=tcp::2222-:22 \

.PHONY: prepare-qemu qemu

compile_commands.json:
	$(MAKE) clean
	bear -- $(MAKE)

.PHONY: compile_commands.json