GET_MEMORY_MAP_HOOK_OBJS += get_memory_map/get_memory_map.o
GET_MEMORY_MAP_HOOK_OBJS += $(addprefix shared/,$(SHARED_OBJS))

GET_MEMORY_MAP_HOOK_OBJS := $(addprefix obj/,$(GET_MEMORY_MAP_HOOK_OBJS))

build/get_memory_map.elf: script.ld.gen $(GET_MEMORY_MAP_HOOK_OBJS)
	@mkdir -p $(@D)
	$(CC) $(GET_MEMORY_MAP_HOOK_OBJS) -T $< -o $@ $(LDFLAGS)
