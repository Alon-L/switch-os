LINUX_HEADERS ?= $(error Must pass LINUX_HEADERS variable)

all: 
	make -C $(LINUX_HEADERS) M=$(CURDIR) modules

clean:
	make -C $(LINUX_HEADERS) M=$(CURDIR) clean

.PHONY: all clean
