/*****************************************************************
* File Name 	: nothing.c
* Description	: A minimalistic kernel module.
*
* Platform	: Linux
*  
* Date			Name			Reason
* 2nd April,2018	Shashi Shivaraju 	ECE_6680_Lab_07
*			[C88650674]
******************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shashi Shivaraju");

static int donothing_init(void)
{
	printk(KERN_ALERT "This is a DoNothing module\n");
	return 0;
}

static void donothing_exit(void)
{
	printk(KERN_ALERT "Donothing module is unloaded\n");
}

module_init(donothing_init);
module_exit(donothing_exit);
