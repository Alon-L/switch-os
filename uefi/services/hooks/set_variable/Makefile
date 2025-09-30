SET_VARIABLE_HOOK_OBJS += set_variable/set_variable.o
SET_VARIABLE_HOOK_OBJS += set_variable/commands.o
SET_VARIABLE_HOOK_OBJS += $(addprefix shared/,$(SHARED_OBJS))

SET_VARIABLE_HOOK_OBJS := $(addprefix obj/,$(SET_VARIABLE_HOOK_OBJS))

build/set_variable.elf: script.ld.gen $(SET_VARIABLE_HOOK_OBJS)
	@mkdir -p $(@D)
	$(CC) $(SET_VARIABLE_HOOK_OBJS) -T $< -o $@ $(LDFLAGS)
