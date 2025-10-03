WINDOWS_CC ?= x86_64-w64-mingw32-gcc
WINDOWS_LD ?= x86_64-w64-mingw32-ld

WINDOWS_CFLAGS := $(CFLAGS)

WINDOWS_LDFLAGS := $(LDFLAGS) \
									 -lole32 -lpowrprof

WINDOWS_OBJS := $(OBJS)
WINDOWS_OBJS += osl/windows.o

obj/windows/%.o: %.c
	@mkdir -p $(dir $@)
	$(WINDOWS_CC) -c $^ -o $@ $(WINDOWS_CFLAGS)

build/$(APP_NAME).exe: $(addprefix obj/windows/,$(WINDOWS_OBJS))
	@mkdir -p $(dir $@)
	$(WINDOWS_CC) $^ -o $@ $(WINDOWS_LDFLAGS)

TARGETS += build/$(APP_NAME).exe
