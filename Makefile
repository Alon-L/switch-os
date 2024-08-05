ARCH ?= X64
CC := GCC5

PACKAGE_DSC := SwitchOSPkg/SwitchOSPkg.dsc

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

compile_commands.json:
	$(MAKE) clean
	bear -- $(MAKE)

.PHONY: all clean cleanall compile_commands.json