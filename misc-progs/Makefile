FILE = setconsole nbtest

KERNELDIR ?= /lib/module/$(shell uname -r)/build
INCLUDEDIR = $(KERNELDIR)/INCLUDEDIR
CFLAGS = -O2 -fomit-frame-pointer -Wall -I$(INCLUDEDIR)

all: $(FILE)

clean:
	rm -f $(FILE) *~ core