CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy

CFLAGS := -I../include/ -ffreestanding -static -O0 -Wall -Wextra -DCORE
LDFLAGS := -ffreestanding -static -O0 -nostdlib -lgcc
LD_SCRIPT_FLAGS := -E -P -x c -I../include/ 

OBJ_DIR := obj
BUILD_DIR := build

include rm/Makefile

OBJS := core.o
OBJS_IN_OBJ_DIR := $(addprefix $(OBJ_DIR)/,$(OBJS))

TRIM_COUNT := 4096

all: $(BUILD_DIR)/core_final.o
.DEFAULT_GOAL := all

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BUILD_DIR)/*
	rm -rf script.ld.gen
	
.PHONY: all clean

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/$(dir $^)
	$(CC) -c $^ -o $@ $(CFLAGS)

$(BUILD_DIR)/core.bin: $(OBJS_IN_OBJ_DIR) script.ld.gen | $(BUILD_DIR)
	$(CC) $(OBJS_IN_OBJ_DIR) -T script.ld.gen -o $@ $(LDFLAGS)

# ---- CORE PARTS -----

$(BUILD_DIR)/%.bin.trimmed: $(BUILD_DIR)/%.bin | $(BUILD_DIR)
	dd if=$^ of=$@ bs=1 skip=$(TRIM_COUNT)

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.bin.trimmed | $(BUILD_DIR)
	$(OBJCOPY) -I binary -O elf64-x86-64 $^ $@

$(BUILD_DIR)/core_final.o: $(BUILD_DIR)/core.o $(BUILD_DIR)/rm.o
	$(LD) -r $^ -o $@ 

# ---- UTILS -----

$(BUILD_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

script.ld.gen: script.ld
	$(CC) $(LD_SCRIPT_FLAGS) -o $@ $^ 
