obj-m += switch_os.o

all:
	make -C /lib/modules/$(LINUX_VERSION)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(LINUX_VERSION)/build M=$(CURDIR) clean
