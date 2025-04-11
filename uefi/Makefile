CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy

GNU_EFI ?= $(error Must pass GNU_EFI variable)

CFLAGS := -I$(GNU_EFI)/inc \
	-fpic -ffreestanding -fno-stack-protector \
	-fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
	-I../include/

LDFLAGS := -shared -Bsymbolic \
	-L$(GNU_EFI)/x86_64/lib -L$(GNU_EFI)/x86_64/gnuefi \
	-T$(GNU_EFI)/gnuefi/elf_x86_64_efi.lds

OBJCOPY_FLAGS := -j ".text" -j ".sdata" -j ".data" -j ".rodata" \
	-j ".dynamic" -j ".dynsym" -j ".rel" -j ".rela" \
	-j ".rel.*" -j ".rela.*" -j ".reloc" \
	--target efi-app-x86_64 --subsystem=10

OBJS += app.o

OBJS_IN_OBJ_DIR := $(addprefix obj/,$(OBJS))

obj:
	mkdir -p $@

obj/%.o: %.c | obj
	$(CC) -c $^ -o $@ $(CFLAGS)

obj/app.so: $(OBJS_IN_OBJ_DIR) $(GNU_EFI)/x86_64/gnuefi/crt0-efi-x86_64.o | obj
	$(LD) $(LDFLAGS) $^ -o $@ -lgnuefi -lefi

build:
	mkdir -p $@

build/app.efi: obj/app.so | build
	$(OBJCOPY) $(OBJCOPY_FLAGS) $^ $@

all: build/app.efi
.DEFAULT_GOAL := all

clean:
	rm -rf obj/*
	rm -rf build/*

.PHONY: all clean
