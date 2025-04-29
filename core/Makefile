CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy
VPATH := ../

CFLAGS := -Wall -Wextra -Werror \
					-I../shared/include/ -I. -ffreestanding -static -O0 -fno-zero-initialized-in-bss \
					-D CORE \
					-fno-pic \
					-ffunction-sections -fno-stack-protector -mno-sse -mno-red-zone -mgeneral-regs-only
LDFLAGS := -ffreestanding -static -O0 -nostdlib
LD_SCRIPT_FLAGS := -E -P -x c -I../shared/include/ 

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

include ../shared/Makefile
OBJS += $(addprefix shared/,$(SHARED_OBJS))

ifdef CORE_GCC_DEBUG
	CFLAGS += -gdwarf -g3
endif

# `TRACE_DEBUG` allows `TRACE` to print to qemu's debugcon device.
ifdef TRACE_DEBUG
	OBJS += core_trace.o
	CFLAGS += -D TRACE_DEBUG
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

build/core.bin: $(OBJS_IN_OBJ_DIR) script.ld.gen
	mkdir -p $(dir $@)
	$(CC) $(OBJS_IN_OBJ_DIR) -T script.ld.gen -o $@ $(LDFLAGS)

# ---------------------
# ---- CORE PARTS -----
# ---------------------
TRIM_COUNT := 4096

# Trim the header of the ELF.
build/%.bin.trimmed: build/%.bin
	mkdir -p $(dir $@)
	dd if=$^ of=$@ bs=$(TRIM_COUNT) skip=1

# Create a dump for every part of core (core, pm, rm).
# This extracts the text and data sections into a binary blob in an object file.
# NOTE:The bss section is embedded into the data section.
build/%.dump.o: build/%.bin.trimmed
	mkdir -p $(dir $@)
	$(OBJCOPY) -I binary --only-section=.text --only-section=.data -O elf64-x86-64 $^ $@

# Build the final core binary combining every part of core.
build/core.o: build/core.dump.o build/rm.dump.o build/pm.dump.o
	mkdir -p $(dir $@)
	$(LD) -r $^ -o $@ 

# -----------------
# ----- UTILS -----
# -----------------
%.ld.gen: %.ld
	$(CC) $(LD_SCRIPT_FLAGS) -o $@ $^ 

all: build/core.o
.DEFAULT_GOAL := all

clean:
	rm -rf obj/*
	rm -rf build/*
	rm -rf */*.ld.gen
	rm -rf *.ld.gen

.PHONY: all clean
