RM_OBJS := wakeup.o

RM_CFLAGS = $(CFLAGS) -m16
RM_LDFLAGS := -ffreestanding -static -O0 -nostdlib

RM_OBJS_IN_OBJ_DIR := $(addprefix $(OBJ_DIR)/rm/,$(RM_OBJS))

$(OBJ_DIR)/rm: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/rm

$(OBJ_DIR)/rm/%.o: rm/%.S | $(OBJ_DIR)/rm
	$(CC) -c $^ -o $@ $(RM_CFLAGS)

$(BUILD_DIR)/rm.bin: $(RM_OBJS_IN_OBJ_DIR) rm/script.ld.gen | $(BUILD_DIR)
	$(CC) $(RM_OBJS_IN_OBJ_DIR) -T rm/script.ld.gen -o $@ $(RM_LDFLAGS)
