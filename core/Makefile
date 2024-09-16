CC ?= gcc
OBJCOPY ?= objcopy

OBJS := core.o

OBJ_DIR := obj
BUILD_DIR := build

OBJS_IN_OBJ_DIR := $(addprefix $(OBJ_DIR)/,$(OBJS))

all: $(BUILD_DIR)/$(CORE_OBJ)

$(BUILD_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

script.ld.gen: script.ld
	$(CC) -E -P -x c -I../include/ -o $@ $^

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) -I../include/ -c $^ -o $@ -ffreestanding -O0 -Wall -Wextra -DCORE

$(BUILD_DIR)/core.bin: $(OBJS_IN_OBJ_DIR) script.ld.gen | $(BUILD_DIR)
	$(CC) -I../include/ -T script.ld.gen -o $@ -ffreestanding -O0 -nostdlib $(OBJS_IN_OBJ_DIR) -lgcc

$(BUILD_DIR)/$(CORE_OBJ): $(BUILD_DIR)/core.bin | $(BUILD_DIR)
	$(OBJCOPY) -I binary -O elf64-x86-64 $^ $@

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BUILD_DIR)/*
	rm -rf script.ld.gen
	
.PHONY: all clean
