CC := GCC5

PACKAGE_NAME := SwitchOSPkg
PACKAGE_DSC := $(PACKAGE_NAME)/$(PACKAGE_NAME).dsc

EDK2_DIR ?= ~/src/edk2
EDK2_BUILD_DIR ?= $(EDK2_DIR)/Build

ifeq (, $(shell which build))
$(error "No build command found. Run: 'cd $(EDK2_DIR); . edksetup.sh; cd -'")
endif

ifeq (,$(DRIVER_BUILD_DIR))
$(error "Pass a DRIVER_BUILD_DIR param")
endif

.default: all

all:
	build -a $(ARCH) -t $(CC) -p $(PACKAGE_DSC)
	@mkdir -p $(DRIVER_BUILD_DIR)
	cp -rf $(EDK2_BUILD_DIR)/$(PACKAGE_NAME)/DEBUG_$(CC)/$(ARCH)/*.efi $(DRIVER_BUILD_DIR)/

clean:
	build clean
	rm -rf $(DRIVER_BUILD_DIR)

cleanall:
	build cleanall
	rm -rf $(DRIVER_BUILD_DIR)

.PHONY: all clean cleanall

compile_commands.json:
	$(MAKE) clean
	bear -- $(MAKE)

.PHONY: compile_commands.json