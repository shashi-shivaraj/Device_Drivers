/*****************************************************************
* File Name 	: query_ioctl.h
* Description	: Header file
*
* Platform		: Linux
* 
* Date			    Name			    Reason
* 29th Sep,2016	    Shashi Shivaraju 	Kernel Programming.
******************************************************************/

#ifndef QUERY_IOCTL_H
#define QUERY_IOCTL_H

#include <linux/ioctl.h>
 
typedef struct
{
    int status, dignity, ego;
} query_arg_t;
 
#define QUERY_GET_VARIABLES _IOR('q', 1, query_arg_t *)
#define QUERY_CLR_VARIABLES _IO('q', 2)
#define QUERY_SET_VARIABLES _IOW('q', 3, query_arg_t *)
 
#endif
