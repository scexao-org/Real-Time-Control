/****************************************************************************
Copyright Notice:
  This computer software is proprietary to VMETRO. The use of this software
  is governed by a licensing agreement. VMETRO retains all rights under
  the copyright laws of the United States of America and other countries.
  This software may not be furnished or disclosed to any third party and
  may not be copied or reproduced by any means, electronic, mechanical, or
  otherwise, in whole or in part, without specific authorization in writing
  from VMETRO.
 
    Copyright (c) 1996-2006 by VMETRO, ASA.  All Rights Reserved.



Module      : vmosa-linux-user-space.h

Description : Header file declaring linux user space in vmosa

Revision History:
  01a,26jul2004,meh  Created.

*****************************************************************************/

#ifndef DPIO2_LINUX_USER_SPACE_INCLUDED
#define DPIO2_LINUX_USER_SPACE_INCLUDED


#include "vmosa-linux.h"
#include "dpio2-linux.h"


#if 1

#include "stdio.h"      /* FILE, printf, ... */
#include "pthread.h"    /* pthread_attr_init ,pthread_create, .... */ 
#include "sys/user.h"   /* PAGE_SIZE */
//#include "asm/page.h"   /* PAGE_SIZE */
#include "stdlib.h"     /* malloc,.. */
#include "unistd.h"     /* close, ... */
#include "sys/ioctl.h"  /* ioctl */
#include "sys/types.h"  /* socket, open, .. */
#include "sys/stat.h"   /* open, .. */
#include "fcntl.h"      /* open, .. */
#include "sys/mman.h"   /* mmap, munmap, ... */
#include "errno.h"      /* errno */
#include "string.h"     /* memset,.. */
#include "time.h"       /* nanosleep */

#else



#include "sys/socket.h" /* socket, .. */
#include "netinet/in.h" /* socket, struct sockaddr_in, .. */
#include "arpa/inet.h"  /* socket, inet_addr, ... */
#include "netdb.h"      /* socket, gethostbyname, ... */

#endif




#if 1
#ifdef __cplusplus
extern "C" {
#endif
#endif




#define MARK printf ("%s %d  Was Here\n", __FILE__, __LINE__); \
             vmosa_mdelay (100);

#if 0
#define DB(args) 
#else
#define DB(args) \
       printf ("DB: %s %d:\n", __FILE__, __LINE__); \
       printf args; \
       vmosa_mdelay (100);
#endif


#define DPIO2_ERR_MSG(args) \
       printf ("DPIO2: ERROR: %s %d\n", __FILE__, __LINE__); \
       printf ("DPIO2: ERROR: "); \
       printf args;



#define DPIO2_DEVICE_OPEN   1
#define DPIO2_DEVICE_FREE   0



typedef struct dpio2_isr_t
{
  pthread_t           pthread;
  pthread_attr_t      attribute;
  struct sched_param  pri;
  int                 exit_flag;

} dpio2_isr_t;


typedef struct dpio2_pci_adrs_t
{
  void    *ctrl_base;
  size_t  ctrl_mmap_size;
  void    *fifo_base;
  size_t  fifo_mmap_size;

} dpio2_pci_adrs_t;


typedef struct dpio2_callback_t
{
  FUNCPTR func_ptr;
  void    *arg;

} dpio2_callback_t;


typedef struct dpio2_handle_t
{
  int                   devno;
  int                   fd;
  dpio2_pci_adrs_t      user;
  dpio2_callback_t      callback;
  dpio2_pci_info_t      pci;
  dpio2_isr_t           isr;

} dpio2_handle_t;

typedef dpio2_handle_t*  HANDLE;





/* Functions
 */

int    dpio2_scan ( dpio2_hw_info_t* );
HANDLE dpio2_open ( int );
STATUS dpio2_close ( int );

STATUS dpio2_prepare_interrupt_notification ( HANDLE, FUNCPTR, void* );
STATUS dpio2_remove_interrupt_notification  ( HANDLE );

HANDLE dpio2_flash_loader_get_handle ( int );

STATUS dpio2_disable_route_intr_pci ( HANDLE );
STATUS dpio2_enable_route_intr_pci ( HANDLE );

STATUS dpio2_disable_route_intr_pci_save_atomic ( HANDLE, BOOL* );
STATUS dpio2_enable_route_intr_pci_save ( HANDLE, BOOL );

STATUS set_intel_31154_p2p_prefetch_policy ( int, int );

void busmarkn ( int devno, UINT32 dword );
void busmark ( HANDLE handle, UINT32 dword );



#if 1
#ifdef __cplusplus
}
#endif
#endif


#endif /* DPIO2_LINUX_USER_SPACE_INCLUDED */
