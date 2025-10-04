LINUX_CC ?= gcc
LINUX_LD ?= ld

LINUX_CFLAGS := $(CFLAGS)

LINUX_LDFLAGS := $(LDFLAGS) \
								 -lefivar

LINUX_OBJS := $(OBJS)
LINUX_OBJS += osl/linux.o

obj/linux/%.o: %.c
	@mkdir -p $(dir $@)
	$(LINUX_CC) -c $^ -o $@ $(LINUX_CFLAGS)

build/$(APP_NAME).elf: $(addprefix obj/linux/,$(LINUX_OBJS))
	@mkdir -p $(dir $@)
	$(LINUX_CC) $^ -o $@ $(LINUX_LDFLAGS)

TARGETS += build/$(APP_NAME).elf
