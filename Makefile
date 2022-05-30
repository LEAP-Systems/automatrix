WARN    := -W -Wall -Wstrict-prototypes -Wmissing-prototypes
INCLUDE := -isystem /lib/modules/`uname -r`/build/include
CFLAGS  := -O2 -DMODULE -D__KERNEL__ ${WARN} ${INCLUDE}
CC      := gcc

# running by kernel build system
ifneq ($(KERNELRELEASE),)
	$(TARGET)-objs := main.o
	obj-m := main.o
# running without kernel build system
else
	BUILD_SYSTEM_DIR:=/lib/modules/$(shell uname -r)/build

all: ## build kernel module
	make -C $(BUILD_SYSTEM_DIR) M=$(PWD) modules

clean: ## clean kernel module build artefacts
	make -C $(BUILD_SYSTEM_DIR) M=$(PWD) clean

load: ## load kernel module object
	insmod ./$(TARGET).ko

unload: ## unload kernel module object
	rmmod ./$(TARGET).ko

.PHONY: help	
help:
	@echo Usage:
	@echo "  make [target]"
	@echo
	@echo Targets:
	@awk -F ':|##' \
		'/^[^\t].+?:.*?##/ {\
			printf "  %-30s %s\n", $$1, $$NF \
		 }' $(MAKEFILE_LIST)

endif