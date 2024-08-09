ARCH ?= X64
CC := GCC5

PACKAGE_NAME := SwitchOSPkg
PACKAGE_DSC := $(PACKAGE_NAME)/$(PACKAGE_NAME).dsc

EDK2_BUILD_DIR ?= ~/src/edk2/Build
BUILD_DIR ?= build/$(ARCH)

ifeq (, $(shell which build))
$(error "No build command found. Run '. ../edksetup.sh'")
endif

.default: all

all:
	build -a $(ARCH) -t $(CC) -p $(PACKAGE_DSC)
	@mkdir -p $(BUILD_DIR)
	cp -rf $(EDK2_BUILD_DIR)/$(PACKAGE_NAME)/DEBUG_$(CC)/$(ARCH)/*.efi $(BUILD_DIR)/

clean:
	build clean
	rm -rf $(BUILD_DIR)

cleanall:
	build cleanall
	rm -rf $(BUILD_DIR)

.PHONY: all clean cleanall

compile_commands.json:
	$(MAKE) clean
	bear -- $(MAKE)

.PHONY: compile_commands.json