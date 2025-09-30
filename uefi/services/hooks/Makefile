CFLAGS := -I$(GNU_EFI)/inc \
	-fpic -ffreestanding -fno-stack-protector \
	-fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args \
	-mno-sse -mgeneral-regs-only \
	-fno-zero-initialized-in-bss \
	-ffunction-sections \
	-D GNU_EFI_USE_MS_ABI \
	-I . \
	-I $(ROOT_DIR)/uefi \
	-I $(ROOT_DIR)/shared/include/

LDFLAGS := -ffreestanding -nostdlib -nostartfiles

LD_SCRIPT_FLAGS := -I $(ROOT_DIR)/shared/include/

ifdef TRACE_DEBUG
	CFLAGS += -D TRACE_DEBUG
ifdef QEMU_DEBUG
	include $(ROOT_DIR)/shared/src/qemu_print/Makefile
	CFLAGS += -D trace=qemu_print
endif
endif

include set_variable/Makefile
include get_memory_map/Makefile
include virtual_address_map_event/Makefile

%.ld.gen: %.ld
	$(CC) -E -P -x c $(LD_SCRIPT_FLAGS) -o $@ $^

obj/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $^ -o $@ $(CFLAGS)

build/%.raw.bin: build/%.elf
	@mkdir -p $(dir $@)
	$(OBJCOPY) --only-section=.header --only-section=.text --only-section=.data -O binary $^ $@

build/%.raw.o: build/%.raw.bin
	@mkdir -p $(dir $@)
	$(OBJCOPY) -I binary -O elf64-x86-64 $^ $@

build/hooks.o: build/set_variable.raw.o build/get_memory_map.raw.o build/virtual_address_map_event.raw.o
	@mkdir -p $(dir $@)
	$(LD) -r $^ -o $@

all: build/hooks.o
.DEFAULT_GOAL := all

clean:
	rm -rf obj/*
	rm -rf build/*

.PHONY: all clean
