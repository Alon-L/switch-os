CFLAGS := -Wall -Wextra -Werror \
					-I $(ROOT_DIR)/shared/include/ -I. -ffreestanding -static -O0 -fno-zero-initialized-in-bss \
					-D CORE \
					-fno-pic \
					-ffunction-sections -fno-stack-protector -mno-sse -mno-red-zone -mgeneral-regs-only
LDFLAGS := -ffreestanding -static -O0 -nostdlib
LD_SCRIPT_FLAGS := -I $(ROOT_DIR)/shared/include/

include rm/Makefile
include pm/Makefile

# Realmode and protected mode should not be compiled using libgcc.
LDFLAGS += -lgcc

OBJS += core.o
OBJS += alloc.o
OBJS += dump.o
OBJS += acpi.o

include drivers/Makefile
OBJS += $(addprefix drivers/,$(DRIVERS_OBJS))

include ../shared/src/pci/Makefile
# `TRACE_DEBUG` allows `TRACE` to print to qemu's debugcon device.
ifdef TRACE_DEBUG
	CFLAGS += -D TRACE_DEBUG
ifdef QEMU_DEBUG
	include ../shared/src/qemu_print/Makefile
	CFLAGS += -D trace=qemu_print
endif
endif

OBJS += $(addprefix shared/,$(SHARED_OBJS))

ifdef CORE_GCC_DEBUG
	CFLAGS += -gdwarf -g3
endif

# -----------------
# ----- UACPI -----
# -----------------
UACPI_PATH := lib/uacpi
UACPI_SOURCES := $(wildcard $(UACPI_PATH)/source/*.c)
OBJS += $(UACPI_SOURCES:.c=.o) uacpi_glue.o

UACPI_HEADERS_DIR := $(UACPI_PATH)/include
CFLAGS += -I$(UACPI_HEADERS_DIR) -DUACPI_USE_BUILTIN_STRING
LDFLAGS += -I$(UACPI_HEADERS_DIR)

# ----------------
# ----- CORE -----
# ----------------
OBJS_IN_OBJ_DIR := $(addprefix obj/,$(OBJS))

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(CFLAGS)

build/core.elf: $(OBJS_IN_OBJ_DIR) script.ld.gen
	mkdir -p $(dir $@)
	$(CC) $(OBJS_IN_OBJ_DIR) -T script.ld.gen -o $@ $(LDFLAGS)

# ---------------------
# ---- CORE PARTS -----
# ---------------------
build/%.raw.bin: build/%.elf
	@mkdir -p $(dir $@)
	$(OBJCOPY) --only-section=.header --only-section=.text --only-section=.data -O binary $^ $@

build/%.raw.o: build/%.raw.bin
	@mkdir -p $(dir $@)
	$(OBJCOPY) -I binary -O elf64-x86-64 $^ $@

build/core.o: build/core.raw.o build/pm.raw.o build/rm.raw.o
	@mkdir -p $(dir $@)
	$(LD) -r $^ -o $@ 

# -----------------
# ----- UTILS -----
# -----------------
%.ld.gen: %.ld
	$(CC) -E -P -x c $(LD_SCRIPT_FLAGS) -o $@ $^

all: build/core.o
.DEFAULT_GOAL := all

clean:
	rm -rf obj/*
	rm -rf build/*
	rm -rf */*.ld.gen
	rm -rf *.ld.gen

.PHONY: all clean
