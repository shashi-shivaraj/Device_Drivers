#Makefile to build loadable kernel modules
#by Shashi Shivaraju


KERNEL_PATH:= /home/user/kernel/kernel_withsystemcall/linux-2.6.32.55-1

ccflags-y = -c -s -Wall -O3 -g

obj-m += dspio.o sb16driver.o

all:
	make -C  $(KERNEL_PATH) M=$(PWD) modules

clean:
	find . -type f | xargs -n 5 touch
	make -C $(KERNEL_PATH) M=$(PWD) clean


load1:
	sudo insmod ./dspio.ko
load2:
	sudo insmod ./sb16driver.ko

unload1:
	sudo rmmod ./dspio.ko
unload2:
	sudo rmmod ./sb16driver.ko
