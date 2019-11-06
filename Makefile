ifneq ($(KERNELRELEASE),) 
	obj-m := rw_rcu.o
else 

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

default: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif 

