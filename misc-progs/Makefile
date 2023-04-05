FILE = datasize dataalign setconsole nbtest inp outp mapper

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
INCLUDEDIR = $(KERNELDIR)/include
CFLAGS = -O2 -fomit-frame-pointer -Wall -I$(INCLUDEDIR)

all: $(FILE)

clean:
	rm -f $(FILE) *~ core
