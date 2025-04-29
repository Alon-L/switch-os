CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy
VPATH := ../

GNU_EFI ?= $(error Must pass GNU_EFI variable)

CFLAGS := -I$(GNU_EFI)/inc \
	-fpic -ffreestanding -fno-stack-protector \
	-fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
	-I../shared/include/ \
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
endif

OBJS += app.o
OBJS += core_header_utils.o
OBJS += core_loader.o

# Shipped core binary
OBJS += core.o

include ../shared/Makefile
OBJS += $(addprefix shared/,$(SHARED_OBJS))

OBJS_IN_OBJ_DIR := $(addprefix obj/,$(OBJS))

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(CFLAGS)

obj/app.so: $(OBJS_IN_OBJ_DIR) $(GNU_EFI)/x86_64/gnuefi/crt0-efi-x86_64.o
	mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) $^ -o $@ -lgnuefi -lefi

build/app.efi: obj/app.so
	mkdir -p $(dir $@)
	$(OBJCOPY) $(OBJCOPY_FLAGS) $^ $@

all: build/app.efi
.DEFAULT_GOAL := all

clean:
	rm -rf obj/*
	rm -rf build/*

.PHONY: all clean
