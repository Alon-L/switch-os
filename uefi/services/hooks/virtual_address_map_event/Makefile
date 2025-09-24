VIRTUAL_ADDRESS_MAP_EVENT_OBJS += virtual_address_map_event/virtual_address_map_event.o
VIRTUAL_ADDRESS_MAP_EVENT_OBJS += $(addprefix shared/,$(SHARED_OBJS))

VIRTUAL_ADDRESS_MAP_EVENT_OBJS := $(addprefix obj/,$(VIRTUAL_ADDRESS_MAP_EVENT_OBJS))

build/virtual_address_map_event.elf: script.ld.gen $(VIRTUAL_ADDRESS_MAP_EVENT_OBJS)
	@mkdir -p $(@D)
	$(CC) $(VIRTUAL_ADDRESS_MAP_EVENT_OBJS) -T $< -o $@ $(LDFLAGS)
