ifneq ($(KERNELRELEASE),)
obj-m := hello_version.o
else
KDIR := $(HOME)/linux-kernel-labs/src/linux
#KDIR:= $(HOME)/bbbkernel/kernel/kernel
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
