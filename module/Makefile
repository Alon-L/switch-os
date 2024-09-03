obj-m += switch_os.o

# TODO: depend on the C files
all: 
	make -C /lib/modules/$(LINUX_VERSION)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(LINUX_VERSION)/build M=$(CURDIR) clean

.PHONY: all clean
.DEFAULT: all
