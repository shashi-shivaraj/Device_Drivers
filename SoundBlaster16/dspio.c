/*****************************************************************
* File Name 	: dspio.c
* Description	: A simple kernel module to retrieve SB16 card's
*                 DSP version.
*
* Platform	: Linux
*
* Date			Name			Reason
* 4th  April,2018	Shashi Shivaraju 	ECE_6680_Lab_07
*			[C88650674]
******************************************************************/

/*Header inclusions*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/io.h>

/*Module info*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shashi Shivaraju");
MODULE_DESCRIPTION("Kernel module to query SB16's DSP version");

/*Macro definations*/
#define	 SB16_BASE		0x220
#define  SB16_DSP_READ		SB16_BASE+0xA
#define  SB16_DSP_RSTATUS       SB16_BASE+0xE
#define  SB16_DSP_WRITE		SB16_BASE+0xC
#define  SB16_DSP_WSTATUS	SB16_BASE+0xC
#define  SB16_DSP_RESET		SB16_BASE+0x6

#define  SB16_RBUF_EMPTY()	(!(0x80 & inb(SB16_DSP_RSTATUS)))
#define  SB16_WBUF_EMPTY()	(0x80 & inb(SB16_DSP_WSTATUS))

/* Function prototypes */
void reset_dsp(void);
unsigned char read_dsp(void);
void write_dsp(unsigned char data);
void get_dsp_version(unsigned char* major,unsigned char* minor);

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

void get_dsp_version(unsigned char* major,unsigned char* minor)
{
	unsigned char data = 0;
	/* reset the dsp first */
	reset_dsp();

	/* Send 0xE1h command to DSP to get its version */
	data = 0xE1;
	write_dsp(data);

	/* Read two bytes to  get the version */
	*major = read_dsp();
	*minor = read_dsp();

	printk(KERN_DEBUG "[get_dsp_version] Major = %d Minor = %d",\
				*major,*minor);
}

/* Function to reset the dsp */
void reset_dsp()
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
	}
	else
	{
		printk(KERN_DEBUG "SB reset successful");
	}
}

static int dspio_init(void)
{
	unsigned char minor = 0,major = 0;
	printk(KERN_DEBUG "DSP_IO module loaded\n");
	get_dsp_version(&major,&minor);

	printk(KERN_DEBUG "Found SB16 card, DSP verion: %d.0%d\n",major,minor);
	return 0;
}

static void dspio_exit(void)
{
	printk(KERN_DEBUG "DSP_IO module is unloaded\n");
}

module_init(dspio_init);
module_exit(dspio_exit);
