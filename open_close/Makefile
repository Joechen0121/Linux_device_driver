CCFLAGS-y += -Wall
CFILES = devone.c

obj-m += sample.o
sample-objs := $(CFILES:.c=.o)


all:
	make -C /lib/modules/4.15.0-47-generic/build M=$(PWD) modules

clean:
	make -C /lib/modules/4.15.0-47-generic/build M=$(PWD) clean

