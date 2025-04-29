PM_OBJS += pm_stub.o
PM_OBJS += pm.o
PM_OBJS += gdt.o
PM_OBJS += paging.o

PM_CFLAGS = $(CFLAGS) -m32 -march=i386 -fno-stack-protector
PM_LDFLAGS := $(LDFLAGS)

PM_OBJS_IN_OBJ_DIR := $(addprefix obj/pm/,$(PM_OBJS))

obj/pm/%.o: pm/%.c
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(PM_CFLAGS)

obj/pm/%.o: pm/%.S
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(PM_CFLAGS)

build/pm.bin: $(PM_OBJS_IN_OBJ_DIR) pm/script.ld.gen
	$(CC) $(PM_OBJS_IN_OBJ_DIR) -T pm/script.ld.gen -o $@ $(PM_LDFLAGS)
