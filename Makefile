obj-m := hcsr501.o

PWD := $(shell pwd)
BUILD_DIR ?= $(PWD)/build

HOST_KDIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: host
host:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(HOST_KDIR) M=$(PWD) modules
	@mv -f $(PWD)/*.ko           $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.cmd          $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.o            $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.mod.c        $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.mod          $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/Module.symvers $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/modules.order  $(BUILD_DIR) 2>/dev/null || true

.PHONY: clean-host
clean-host:
	$(MAKE) -C $(HOST_KDIR) M=$(PWD) clean
	rm -rf $(BUILD_DIR)

RPI5_KDIR  ?= $(SDKTARGETSYSROOT)/usr/src/kernel
RPI5_ARCH  ?= arm64
RPI5_CROSS ?= $(CROSS_COMPILE)

.PHONY: rpi5
rpi5:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(RPI5_KDIR) \
		M=$(PWD) \
		ARCH=$(RPI5_ARCH) \
		CROSS_COMPILE=$(RPI5_CROSS) \
		modules
	@mv -f $(PWD)/*.ko           $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.o            $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.cmd          $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.mod.c        $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/*.mod          $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/Module.symvers $(BUILD_DIR) 2>/dev/null || true
	@mv -f $(PWD)/modules.order  $(BUILD_DIR) 2>/dev/null || true

.PHONY: clean-rpi5
clean-rpi5:
	$(MAKE) -C $(RPI5_KDIR) M=$(PWD) clean
	rm -rf $(BUILD_DIR)

.PHONY: clean
clean: clean-host
