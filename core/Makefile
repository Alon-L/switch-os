CC ?= gcc
OBJCOPY ?= objcopy

OBJS := core.o

OBJ_DIR := obj
BUILD_DIR := build

OBJS_IN_OBJ_DIR := $(addprefix $(OBJ_DIR)/,$(OBJS))

STRIP_COUNT := 4096

all: $(BUILD_DIR)/$(CORE_OBJ)

$(BUILD_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

script.ld.gen: script.ld
	$(CC) -E -P -x c -I../include/ -o $@ $^

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) -I../include/ -c $^ -o $@ -ffreestanding -static -O0 -Wall -Wextra -DCORE

$(BUILD_DIR)/core.bin: $(OBJS_IN_OBJ_DIR) script.ld.gen | $(BUILD_DIR)
	$(CC) -I../include/ -T script.ld.gen -o $@ -ffreestanding -static -O0 -nostdlib $(OBJS_IN_OBJ_DIR) -lgcc

$(BUILD_DIR)/core.trimmed.bin: $(BUILD_DIR)/core.bin | $(BUILD_DIR)
	dd if=$^ of=$@ bs=1 skip=$(STRIP_COUNT)

$(BUILD_DIR)/$(CORE_OBJ): $(BUILD_DIR)/core.trimmed.bin | $(BUILD_DIR)
	$(OBJCOPY) -I binary -O elf64-x86-64 $^ $@
	$(OBJCOPY) --redefine-sym _binary_build_core_trimmed_bin_start=_core_bin_start $@
	$(OBJCOPY) --redefine-sym _binary_build_core_trimmed_bin_size=_core_bin_size $@
	$(OBJCOPY) --redefine-sym _binary_build_core_trimmed_bin_end=_core_bin_end $@

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BUILD_DIR)/*
	rm -rf script.ld.gen
	
.PHONY: all clean
