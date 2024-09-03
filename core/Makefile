CC ?= gcc

OBJS := core.o

%.o: %.c
	$(CC) -c $^ -o $@ -ffreestanding -O0 -Wall -Wextra

$(CORE_BIN): $(OBJS)
	$(CC) -o $@ -ffreestanding -O0 -nostdlib $^ -lgcc

clean:
	rm -rf $(OBJS)
	rm -rf $(CORE_BIN)

all: $(CORE_BIN)
	
.PHONY: all clean
.DEFAULT: all
