/*****************************************************************
* File Name 	: sb16driver.c
* Description	: A device driver to play audio using
*                 SB16 sound card.
*
* Platform	: Linux
*
* Date			Name			Reason
* 5th  April,2018	Shashi Shivaraju 	ECE_6680_Lab_07
*			[C88650674]
******************************************************************/

/* Header inclusions */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>		/* for memory allocation */
#include <asm/signal.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>		/* for port interface */
#include <asm/dma.h>		/* for DMA interface */

/* Module info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shashi Shivaraju");
MODULE_DESCRIPTION("Device Driver for SB16 sound card");

/* Macro definations */
#define  MAJOR_NUMBER		62 /* Device ID for /dev/sb16driver */
#define	 SB16_BASE		0x220
#define  SB16_DSP_READ		SB16_BASE+0xA
#define  SB16_DSP_RSTATUS       SB16_BASE+0xE
#define  SB16_DSP_WRITE		SB16_BASE+0xC
#define  SB16_DSP_WSTATUS	SB16_BASE+0xC
#define  SB16_DSP_RESET		SB16_BASE+0x6

#define  SB16_RBUF_EMPTY()	(!(0x80 & inb(SB16_DSP_RSTATUS)))
#define  SB16_WBUF_EMPTY()	(0x80 & inb(SB16_DSP_WSTATUS))

#define  DMA_BUFFER_SIZE	(64*1024)
/* Function prototypes */
int sb16drv_open(struct inode *inode,struct file *fp);
int sb16drv_release(struct inode *inode,struct file *fp);
ssize_t sb16drv_read(struct file *fp,char *buf,size_t count,loff_t *f_pos);
ssize_t sb16drv_write(struct file *fp,const char *buf,size_t count,loff_t *f_pos);
static int sb16driver_init(void);
static void sb16driver_exit(void);

int reset_dsp(void);
unsigned char read_dsp(void);
void write_dsp(unsigned char data);
int get_dsp_version(unsigned char* major,unsigned char* minor);
int dad_dma_prepare(int channel,int mode,unsigned int buf,unsigned int count);

/* File operations structure */
const struct file_operations sb16drv_fops = {
	read: sb16drv_read,
	write: sb16drv_write,
	open: sb16drv_open,
	release: sb16drv_release
};

/* DMA buffer pointer */
unsigned char *dma_buffer = NULL;

/* DMA buffer available */
static unsigned int dma_available = DMA_BUFFER_SIZE;

/* Driver deinit function */
static void sb16driver_exit(void)
{
	/* free DMA buffer */
	if(dma_buffer)
	  free_pages((long)dma_buffer,4);

	/* unregister the device */
        unregister_chrdev(MAJOR_NUMBER,"sb16driver");

	/* reset the DSP */
        reset_dsp();

	printk(KERN_DEBUG "SB16Driver module unloaded\n");
}

/* Function to prepare DMA transfer */
int dad_dma_prepare(int channel,int mode,unsigned int buf,unsigned int count)
{
	unsigned long flags;

	flags = claim_dma_lock();
	disable_dma(channel);
	clear_dma_ff(channel);
	set_dma_mode(channel,mode);
	set_dma_addr(channel,virt_to_bus((volatile void *)buf));
	set_dma_count(channel,count);
	enable_dma(channel);
	release_dma_lock(flags);

	return 0;
}

/* DMA controller configuration */
void init_dmactl(void)
{
	/* Prepare DMA for mono channel,write mode for buffer
	   of the size 64KB */
	dad_dma_prepare(1,DMA_MODE_WRITE,(void *)dma_buffer,DMA_BUFFER_SIZE);
}

/* Setup the DMA transfer */
void init_dspma(void)
{
	unsigned char high_byte = 0;
	unsigned char low_byte = 0;
	unsigned char data = 0;
	int sample_rate = 11025;
	int data_size = DMA_BUFFER_SIZE -1;

	/* DSP version is 4.x */
	/* Program DSP with actual smaple rate =  11025 Hz */
	high_byte = (sample_rate & 0xFF00) >> 8;
	low_byte  = (sample_rate & 0x00FF);

	/* Send 41h to set output sample rate */
	data = 0x41;
	write_dsp(data);
	write_dsp(0x2B);
	write_dsp(0x11);

	/* Send an I/O command */
	data = 0xC0;	/* 8 bit single-cycle output */
	write_dsp(data);
	data = 0x00;	/* Transfer Mode: 8bit Mono unsigned PCM */
	write_dsp(data);
	low_byte  = (data_size & 0x00FF);
	high_byte = (data_size & 0xFF00) >> 8;
	/*Send Data transfer count */
	write_dsp(low_byte);
	write_dsp(high_byte);
}

int dad_dma_isdone(int channel)
{
	int residue;
	unsigned long flags = claim_dma_lock();
	residue  = get_dma_residue(channel);
	release_dma_lock(flags);
	return (residue == 0);
}

/* Function to play the sound */
void start_play(void)
{

	init_dmactl();	/* init DMA controller */
	init_dspma();	/* init DSP DMA setting */

	/* To check the DMA status */
	//result = dad_dma_isdone(1);

	printk(KERN_DEBUG "Started  play!\n");
}

/* Function to read one byte of data from dsp */
unsigned char read_dsp()
{
 	/* Will exit only when bit 7 is set */
	while(SB16_RBUF_EMPTY());
	return inb(SB16_DSP_READ);
}

/* Function to write one byte of data to dsp */
void write_dsp(unsigned char data)
{
	/*If bit7 of Write Buffer Status is 0,then DSP buffer is empty
	and is ready to recieve comman */
	while(SB16_WBUF_EMPTY());
	outb(data,SB16_DSP_WRITE);
}

/* Function to get DSP version */
int get_dsp_version(unsigned char* major,unsigned char* minor)
{
	unsigned char data = 0;
	int 	      result = 0;
	/* reset the dsp first */
	result = reset_dsp();
	if(result < 0)
	{
	  return result;
	}

	/* Send 0xE1h command to DSP to get its version */
	data = 0xE1;
	write_dsp(data);

	/* Read two bytes to  get the version */
	*major = read_dsp();
	*minor = read_dsp();

	printk(KERN_DEBUG "[get_dsp_version] Major = %d Minor = %d",*major,*minor);
	return 0;
}

/* Function to reset the dsp */
int reset_dsp()
{
	/* Write 1 to reset port 2x6h */
	outb(1,SB16_DSP_RESET);

	/* Wait for 3 microseconds */
	udelay(3);

	/* Write 0 to reset port */
	outb(0,SB16_DSP_RESET);

	/*check for ready byte 0AAh from read data port*/
	if(0xAA != read_dsp())
	{
		printk(KERN_ALERT "[ERROR] SB reset unsuccessful");
		return -1;
	}
	else
	{
		printk(KERN_DEBUG "SB reset successful");
		return 0;
	}
}

/* Driver init function */
static int sb16driver_init(void)
{
	unsigned char minor = 0,major = 0;
	int result = 0;
	printk(KERN_DEBUG "SB16Driver module loaded\n");

	/* Reset the DSP and print the DSP version */
	result = get_dsp_version(&major,&minor);
	if(result < 0)
	{
		printk(KERN_ALERT "[get_dsp_version] FAILED\n");
		goto FAIL;
	}

	/* Register the device */
	result =  register_chrdev(MAJOR_NUMBER,"sb16driver",&sb16drv_fops);
	if(result < 0)
	{
		printk(KERN_ALERT "[register_chrdev] FAILED\n");
		goto FAIL;
	}

	printk(KERN_DEBUG "Found SB16 card, DSP verion: %d.0%d\n",major,minor);

	/* Allocate DMA Buffer */
	if(!dma_buffer)
	{
	   dma_buffer = (unsigned char*)__get_free_pages(GFP_DMA,4);
	   if(!dma_buffer)
	   {
	     printk(KERN_ALERT "dma_buffer allocation failed\n");
	     goto FAIL;
	   }
	}
	else
	{
	   printk(KERN_ALERT "dma_buffer already allocated\n");

	}

	return 0;
FAIL:
	printk(KERN_ALERT "[sb16_driverinit] FAILED\n");
	sb16driver_exit();
	return result;
}

/* Function to open device */
int sb16drv_open(struct inode *inode,struct file *fp)
{
	printk(KERN_DEBUG "Device Opened\n");
	return 0; /* Always return success */
}

/* Function to close device */
int sb16drv_release(struct inode *inode,struct file *fp)
{
	printk(KERN_DEBUG "Device Closed\n");

	//memset(dma_buffer,0,DMA_BUFFER_SIZE);
	dma_available = DMA_BUFFER_SIZE;
	return 0; /* Always return success */
}

/* Function for read device */
ssize_t sb16drv_read(struct file *fp,char* buf,size_t count,loff_t *f_pos)
{
	return 0; /* nothing to read;Always return success */
}

/* Function for write data too DMA buffer */
ssize_t sb16drv_write(struct file *fp,const char* buf,size_t count,loff_t *f_pos)
{
	int num_bytes = 0; /* Number of bytes written */

	printk(KERN_DEBUG "[sb16drv_write] enter offset = %lld,count = %d\n",*f_pos,count);

	if(0 == *f_pos)
	{
	    num_bytes = copy_from_user(dma_buffer,buf,count);
	    if(0 == num_bytes) /* copy success*/
	    {
		*f_pos = count;
		dma_available = dma_available - count;
		if(!dma_available)
		{
			printk(KERN_DEBUG "Calling start_play()");
			start_play();

		}
		return count;
	    }
	    else /* returns number of bytes that could not be copied */
	    {
		*f_pos = count - num_bytes;
		dma_available = dma_available - *f_pos;
		if(!dma_available)
		{
			printk(KERN_DEBUG "Calling start_play()");
			start_play();
		}
		return *f_pos;
	    }
	}
	else if(*f_pos >= DMA_BUFFER_SIZE || !dma_available)
	{
		return -ENOSPC;
	}
	else
	{
	    num_bytes = copy_from_user(dma_buffer+*f_pos,buf,count);
	    if(0 == num_bytes) /* copy success*/
	    {
		*f_pos +=  count;
		dma_available = dma_available - count;
		if(!dma_available)
		{
			printk(KERN_DEBUG "Calling start_play2()");
			start_play();
		}
		return count;
	    }
	    else /* returns number of bytes that could not be copied */
	    {
		*f_pos += count - num_bytes;
		dma_available = dma_available - *f_pos;
		if(!dma_available)
		{
			printk(KERN_DEBUG "Calling start_play1()");
			start_play();
		}
		return (count - num_bytes);
	    }
	}
	return num_bytes;  /* return number of bytes written */
}

module_init(sb16driver_init);
module_exit(sb16driver_exit);
