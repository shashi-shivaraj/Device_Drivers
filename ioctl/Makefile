# Makefile for building loadable kernel module
# By Shashi Shivaraju 

TARGET_MODULE:= ioctl_query

obj-m+= $(TARGET_MODULE).o

$(TARGET_MODULE)-objs := ./src/query_ioctl.o

all:
	 make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules -I ./src/
	 $(CC) src/query_app.c -o querytest -I ./src/
clean:
	 make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	 rm querytest

load:
	insmod ./$(TARGET_MODULE).ko

unload:
	rmmod ./$(TARGET_MODULE).ko
