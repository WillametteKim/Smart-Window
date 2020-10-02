KERNELDIR = /lib/modules/$(shell uname -r)/build

obj-m := raintest_dev.o

PWD := $(shell pwd)

all: default 

rm: rmmod rmnod

ins: insmod mknod

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

rmmod:
	sudo rmmod raintest_dev

rmnod:
	sudo rm /dev/raintest_dev

insmod:
	sudo insmod raintest_dev.ko

mknod:
	sudo mknod -m 666 /dev/raintest_dev c 242 0
