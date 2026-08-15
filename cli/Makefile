PLATFORM ?=

OBJS := app.o

OBJS += commands.o
OBJS += commands/help.o
OBJS += commands/store.o
OBJS += commands/switch.o

OBJS += lib/argtable/argtable3.o

CFLAGS := -std=c11 -Wall -Wextra -Werror \
					-I $(ROOT_DIR)/shared/include/ \
					-I. \
					-I lib/argtable \
					-D TRACE_DEBUG

LDFLAGS := -lm

APP_NAME := switch-os-cli

TARGETS :=
ifneq ($(PLATFORM),)
include makefiles/$(PLATFORM).mk
else
include makefiles/*.mk
endif

all: $(TARGETS)
.DEFAULT_GOAL := all

clean:
	rm -rf obj/*
	rm -rf build/*

.PHONY: all clean
