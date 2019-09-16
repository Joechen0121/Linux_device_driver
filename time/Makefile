obj-m += main.o


all:
	make -C /lib/modules/4.15.0-47-generic/build M=$(PWD) modules

clean: 

	make -C /lib/modules/4.15.0-47-generic/build M=$(PWD) clean
