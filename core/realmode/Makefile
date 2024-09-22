REALMODE_OBJS := wakeup.o

REALMODE_CFLAGS = $(CFLAGS) -m16
REALMODE_LDFLAGS := -ffreestanding -static -O0 -nostdlib

REALMODE_OBJS_IN_OBJ_DIR := $(addprefix $(OBJ_DIR)/realmode/,$(REALMODE_OBJS))

$(OBJ_DIR)/realmode: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/realmode

$(OBJ_DIR)/realmode/%.o: realmode/%.S | $(OBJ_DIR)/realmode
	$(CC) -c $^ -o $@ $(REALMODE_CFLAGS)

$(BUILD_DIR)/realmode.bin: $(REALMODE_OBJS_IN_OBJ_DIR) realmode/script.ld.gen | $(BUILD_DIR)
	$(CC) $(REALMODE_OBJS_IN_OBJ_DIR) -T realmode/script.ld.gen -o $@ $(REALMODE_LDFLAGS)
