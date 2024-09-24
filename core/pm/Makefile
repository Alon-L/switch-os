PM_OBJS := pm_stub.o paging.o

PM_CFLAGS = $(CFLAGS) -m32 -march=i386 -fno-stack-protector
PM_LDFLAGS := $(LDFLAGS)

PM_OBJS_IN_OBJ_DIR := $(addprefix $(OBJ_DIR)/pm/,$(PM_OBJS))

$(OBJ_DIR)/pm: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/pm

$(OBJ_DIR)/pm/%.o: pm/%.c | $(OBJ_DIR)/pm
	$(CC) -c $^ -o $@ $(PM_CFLAGS)

$(OBJ_DIR)/pm/%.o: pm/%.S | $(OBJ_DIR)/pm
	$(CC) -c $^ -o $@ $(PM_CFLAGS)

$(BUILD_DIR)/pm.bin: $(PM_OBJS_IN_OBJ_DIR) pm/script.ld.gen | $(BUILD_DIR)
	$(CC) $(PM_OBJS_IN_OBJ_DIR) -T pm/script.ld.gen -o $@ $(PM_LDFLAGS)
