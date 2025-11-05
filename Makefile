obj-m := hcsr501.o

PWD := $(shell pwd)


KDIR ?= $(KERNEL_SRC)

KDIR ?= /lib/modules/$(shell uname -r)/build


all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean



.PHONY: host
host:               
	$(MAKE) KDIR=/lib/modules/$(shell uname -r)/build all

.PHONY: rpi5sdk
rpi5sdk:            
	$(MAKE) \
		KDIR=$$SDKTARGETSYSROOT/usr/src/kernel \
		ARCH=arm64 \
		CROSS_COMPILE=$$CROSS_COMPILE \
		all
