CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy

CFLAGS := -Wall -Wextra -Werror \
					-I../include/ -I. -ffreestanding -static -O0 -fno-zero-initialized-in-bss \
					-D CORE \
					-fno-pic -masm=intel \
					-ffunction-sections -fno-stack-protector -mno-sse -mno-red-zone -mgeneral-regs-only
LDFLAGS := -ffreestanding -static -O0 -nostdlib
LD_SCRIPT_FLAGS := -E -P -x c -I../include/ 

OBJ_DIR := obj
BUILD_DIR := build

include rm/Makefile
include pm/Makefile

# Realmode and protected mode should not be compiled using libgcc.
LDFLAGS += -lgcc

OBJS += core.o
OBJS += pci.o pci_utils.o
OBJS += alloc.o
OBJS += dump.o

include drivers/Makefile
OBJS += $(addprefix drivers/,$(DRIVERS_OBJS))

ifdef CORE_GCC_DEBUG
	CFLAGS += -gdwarf -g3
endif

# `DEBUG` allows `TRACE` to print to qemu's debugcon device.
ifdef DEBUG
	OBJS += core_trace.o
	CFLAGS += -D DEBUG
endif

# -----------------
# ----- UACPI -----
# -----------------
UACPI_PATH := lib/uacpi
UACPI_SOURCES := $(wildcard $(UACPI_PATH)/source/*.c)
OBJS += $(UACPI_SOURCES:.c=.o) uacpi_glue.o

UACPI_HEADERS_DIR := $(UACPI_PATH)/include
CFLAGS += -I$(UACPI_HEADERS_DIR)
LDFLAGS += -I$(UACPI_HEADERS_DIR)

# ----------------
# ----- CORE -----
# ----------------
OBJS_IN_OBJ_DIR := $(addprefix $(OBJ_DIR)/,$(OBJS))

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/$(dir $^)
	$(CC) -c $^ -o $@ $(CFLAGS)

$(BUILD_DIR)/core.bin: $(OBJS_IN_OBJ_DIR) script.ld.gen | $(BUILD_DIR)
	$(CC) $(OBJS_IN_OBJ_DIR) -T script.ld.gen -o $@ $(LDFLAGS)

# ---------------------
# ---- CORE PARTS -----
# ---------------------
TRIM_COUNT := 4096

# Trim the header of the ELF.
$(BUILD_DIR)/%.bin.trimmed: $(BUILD_DIR)/%.bin | $(BUILD_DIR)
	dd if=$^ of=$@ bs=$(TRIM_COUNT) skip=1

# Create a dump for every part of core (core, pm, rm).
# This extracts the text and data sections into a binary blob in an object file.
# NOTE:The bss section is embedded into the data section.
$(BUILD_DIR)/%.dump.o: $(BUILD_DIR)/%.bin.trimmed | $(BUILD_DIR)
	$(OBJCOPY) -I binary --only-section=.text --only-section=.data -O elf64-x86-64 $^ $@

# Build the final core binary combining every part of core.
$(BUILD_DIR)/core.o: $(BUILD_DIR)/core.dump.o $(BUILD_DIR)/rm.dump.o $(BUILD_DIR)/pm.dump.o
	$(LD) -r $^ -o $@ 

# -----------------
# ----- UTILS -----
# -----------------
$(BUILD_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

%.ld.gen: %.ld
	$(CC) $(LD_SCRIPT_FLAGS) -o $@ $^ 

all: $(BUILD_DIR)/core.o
.DEFAULT_GOAL := all

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BUILD_DIR)/*
	rm -rf */*.ld.gen
	rm -rf *.ld.gen

.PHONY: all clean
