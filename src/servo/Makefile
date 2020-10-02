KERNELDIR = /lib/modules/$(shell uname -r)/build

obj-m := servotest_dev.o

PWD := $(shell pwd)

all: default 

rm: rmmod rmnod

ins: insmod mknod

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

rmmod:
	sudo rmmod servotest_dev

rmnod:
	sudo rm /dev/servotest_dev

insmod:
	sudo insmod servotest_dev.ko

mknod:
	sudo mknod -m 666 /dev/servotest_dev c 240 0
