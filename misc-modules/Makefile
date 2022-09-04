# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then include this
# Make file once again
# This conditional select whether we are being included from the 
# kernel Makefile or not.

LDDINC = $(PWD)/../include

EXTRA_CFLAGS += -I$(LDDINC)

ifeq ($(KERNELRELEASE),)
	# Assuse that the source tree is where the running kernel was build
	# You should set KERNELDIR in the enviroment if it's elsewhere
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	# The current directory is passed to sub-make as argument
	PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.mod modules.order *.symvers

.PHONY: modules modules_install clean

else
	# called from kernel build system:just declare what our modles are
	obj-m := hello.o hellop.o jit.o jiq.o

endif
