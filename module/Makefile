# TODO: depend on the C files
all: 
	make -C /lib/modules/$(LINUX_VERSION) M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(LINUX_VERSION) M=$(CURDIR) clean

.PHONY: all clean
