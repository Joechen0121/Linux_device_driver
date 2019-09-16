CFLAGES += -Wall
CFILES = main.c

obj-m +=sample.o
sample-objs := $(CFILES:.c=.o)

all:
	make -C /lib/modules/4.15.0-46-generic/build M=$(PWD) modules

clean:

	make -C /lib/modules/4.15.0-46-generic/build M=$(PWD) clean
