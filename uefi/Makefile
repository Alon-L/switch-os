GNU_EFI ?= $(error Must pass GNU_EFI variable)

CFLAGS := -I$(GNU_EFI)/inc \
	-fpic -ffreestanding -fno-stack-protector \
	-fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
	-I . \
	-I obj/include \
	-I $(ROOT_DIR)/shared/include/ \
	-D GNU_EFI_USE_MS_ABI \
	-D UEFI

LDFLAGS := -shared -Bsymbolic \
	-L$(GNU_EFI)/x86_64/lib -L$(GNU_EFI)/x86_64/gnuefi \
	-T$(GNU_EFI)/gnuefi/elf_x86_64_efi.lds

OBJCOPY_FLAGS := -j ".text" -j ".sdata" -j ".data" -j ".rodata" \
	-j ".dynamic" -j ".dynsym" -j ".rel" -j ".rela" \
	-j ".rel.*" -j ".rela.*" -j ".reloc" \
	--target efi-app-x86_64 --subsystem=10

ifdef TRACE_DEBUG
	CFLAGS += -D TRACE_DEBUG
ifdef QEMU_DEBUG
	include $(ROOT_DIR)/shared/src/qemu_print/Makefile
	CFLAGS += -D trace=qemu_print
else
	CFLAGS += -D trace=AsciiPrint
endif
endif

OBJS += app.o
OBJS += core_header_utils.o
OBJS += core_loader.o

include acpi/Makefile

OBJS += hooks/hooks_loader.o
OBJS += hooks/loaders/get_memory_map.o
OBJS += hooks/loaders/set_variable.o
OBJS += hooks/loaders/virtual_address_map_event.o

# Shipped core binary
OBJS += core.o

include ../shared/src/pci/Makefile
OBJS += $(addprefix shared/,$(SHARED_OBJS))

OBJS := $(addprefix obj/,$(OBJS))

hooks/raw/build/hooks.o:
	@mkdir -p $(@D)
	$(MAKE) -C hooks/raw

.PHONY: hooks/raw/build/hooks.o

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

obj/app.so: $(OBJS) $(GNU_EFI)/x86_64/gnuefi/crt0-efi-x86_64.o hooks/raw/build/hooks.o
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $^ -o $@ -lgnuefi -lefi

build/app.efi: obj/app.so
	mkdir -p $(dir $@)
	$(OBJCOPY) $(OBJCOPY_FLAGS) $^ $@

all: build/app.efi
.DEFAULT_GOAL := all

clean:
	$(MAKE) -C hooks/raw clean
	rm -rf obj/*
	rm -rf build/*

.PHONY: all clean
