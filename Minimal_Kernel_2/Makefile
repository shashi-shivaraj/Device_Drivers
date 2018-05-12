# Makefile to build kernel module
# By Shashi Shivaraju

TARGET_MODULE:= hello

obj-m += $(TARGET_MODULE).o
	     
$(TARGET_MODULE)-objs := ./src/hello.o ./src/function.o

all:
	 make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	insmod ./$(TARGET_MODULE).ko

unload:
	rmmod ./$(TARGET_MODULE).ko
