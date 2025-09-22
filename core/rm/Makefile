RM_OBJS := wakeup.o

RM_CFLAGS := $(CFLAGS) -m16 -march=i386
RM_LDFLAGS := $(LDFLAGS)

RM_OBJS_IN_OBJ_DIR := $(addprefix obj/rm/,$(RM_OBJS))

obj/rm/%.o: rm/%.S
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(RM_CFLAGS)

build/rm.elf: $(RM_OBJS_IN_OBJ_DIR) rm/script.ld.gen
	mkdir -p $(dir $@)
	$(CC) $(RM_OBJS_IN_OBJ_DIR) -T rm/script.ld.gen -o $@ $(RM_LDFLAGS)
