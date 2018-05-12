/*****************************************************************
* File Name 	: onebyte.c
* Description	: A simple onebyte character driver.
*
* Platform	: Linux
*
* Date			Name			Reason
* 3rd April,2018	Shashi Shivaraju 	ECE_6680_Lab_07
*			[C88650674]
******************************************************************/

#include <linux/module.h>	/* core header for kernel modules */
#include <linux/kernel.h>	/* core kernel header */
#include <linux/init.h>		/* used for mark up functions */
#include <linux/errno.h>	/* used for error codes */
#include <linux/fs.h>		/* used for linux filesystem support */
#include <linux/types.h>
#include <asm/uaccess.h>	/* used for memory access */
#include <linux/proc_fs.h>
#include <linux/slab.h>

/* MACRO defination */
#define MAJOR_NUMBER 61

/* MODULE info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shashi Shivaraju");
MODULE_DESCRIPTION("A onebyte character device");

/* Global Declaration */
char *onebyte_data = NULL;

/* Function Prototypes */
static int onebyte_init(void);
static void onebyte_exit(void);
int onebyte_open(struct inode *inode,struct file *filep);
int onebyte_release(struct inode *inode,struct file *filep);
ssize_t onebyte_read(struct file *filep,char *buf,
		     size_t count,loff_t *f_pos);
ssize_t onebyte_write(struct file *filep,const char *buf,
		      size_t count,loff_t *f_pos);

/* File operation structure */
struct file_operations onebyte_fops = 
{
	read:		onebyte_read,
	write:		onebyte_write,
	open:		onebyte_open,
	release:	onebyte_release
};

/* Fucntion Definations */
int onebyte_open(struct inode *inode,struct file *filep)
{
	return 0;
}
int onebyte_release(struct inode *inode,struct file *filep)
{
	return 0;
}
ssize_t onebyte_read(struct file *filep,char *buf,
		     size_t count,loff_t *f_pos)
{
	printk(KERN_DEBUG "Onebyte read called\n");

	if(*f_pos == 0)
	{
		copy_to_user(buf,onebyte_data,1);
		*f_pos += 1;
		return  1;
	}
	else
	{
		return 0;
	}
}

ssize_t onebyte_write(struct file *filep,const char *buf,
		      size_t count,loff_t *f_pos)
{
	printk(KERN_DEBUG "Onebyte write called\n");

	if(*f_pos == 0)
	{
		copy_from_user(onebyte_data,buf,1);
		*f_pos += 1;
		return 1;
	}
	else
	{
		return -ENOSPC;
	}

}
/* Module initialization function */
static int onebyte_init(void)
{
	int result;
	/* register the device */
	result = register_chrdev(MAJOR_NUMBER,"onebyte",&onebyte_fops);
	if(result < 0)
	{
		return result;
	}

	if(!onebyte_data)
	{
		onebyte_data = kmalloc(sizeof(char),GFP_KERNEL);
		if(!onebyte_data)
		{
			onebyte_exit();
			return -ENOMEM;
		}
	}
	else
	{
		onebyte_exit();
		return -1;
	}

	/* initialize the value to be X */
	*onebyte_data = 'x';

	printk(KERN_ALERT "This is a onebyte device module\n");
	return 0;
}

/* Module deinitialization function */
static void onebyte_exit(void)
{
	/*deallocate memory*/
	if(onebyte_data)
	{
		kfree(onebyte_data);
		onebyte_data = NULL;
	}
	/*unregister the device*/
	unregister_chrdev(MAJOR_NUMBER,"onebyte");
	printk(KERN_ALERT "Onebyte device module is unloaded\n");
}

module_init(onebyte_init);
module_exit(onebyte_exit);
