WARN    := -W -Wall -Wstrict-prototypes -Wmissing-prototypes
INCLUDE := -isystem /lib/modules/`uname -r`/build/include
CFLAGS  := -O2 -DMODULE -D__KERNEL__ ${WARN} ${INCLUDE}
CC      := gcc

SOURCE	:= src
MOD_NAME:= automtx

# running by kernel build system
ifneq ($(KERNELRELEASE),)
	$(TARGET)-objs := $(SOURCE)/$(MOD_NAME).o
	obj-m := $(SOURCE)/$(MOD_NAME).o
# running without kernel build system
else
	BUILD_SYSTEM_DIR:=/lib/modules/$(shell uname -r)/build

all: ## build kernel module
	make -C $(BUILD_SYSTEM_DIR) M=$(PWD) modules

clean: ## clean kernel module build artefacts
	make -C $(BUILD_SYSTEM_DIR) M=$(PWD) clean

load: ## load kernel module object
	sudo insmod $(SOURCE)/$(MOD_NAME).ko

unload: ## unload kernel module object
	sudo rmmod $(SOURCE)/$(MOD_NAME).ko

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