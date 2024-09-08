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

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) -I../include/ -c $^ -o $@ -ffreestanding -O0 -Wall -Wextra

$(BUILD_DIR)/core.bin: $(OBJS_IN_OBJ_DIR) | $(BUILD_DIR)
	$(CC) -I../include/ -T script.ld -o $@ -ffreestanding -O0 -nostdlib $^ -lgcc

$(BUILD_DIR)/$(CORE_OBJ): $(BUILD_DIR)/core.bin | $(BUILD_DIR)
	$(OBJCOPY) -I binary -O elf64-x86-64 $^ $@

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BUILD_DIR)/*
	
.PHONY: all clean
