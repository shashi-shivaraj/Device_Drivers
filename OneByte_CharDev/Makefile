# Makefile to build loadable kernel module
# by Shashi Shivaraju
 
KERNEL_PATH:= /home/user/kernel/kernel_withsystemcall/linux-2.6.32.55-1

ccflags-y = -c -s -Wall -O3 -g

obj-m += onebyte.o

all:
	make -C  $(KERNEL_PATH) M=$(PWD) modules

clean:
	find . -type f | xargs -n 5 touch
	make -C $(KERNEL_PATH) M=$(PWD) clean


load:
	sudo insmod ./onebyte.ko


unload:
	sudo rmmod ./onebyte.ko
