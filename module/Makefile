# TODO: depend on the C files
all: 
	make -C $(LINUX_SOURCE) M=$(CURDIR) modules

clean:
	make -C $(LINUX_SOURCE) M=$(CURDIR) clean

.PHONY: all clean
