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



Module      : vmosa-linux-kernel-space.h

Description : Header file declaring linux kernel space in vmosa

Revision History:
  01a,26jul2004,meh  Created.

*****************************************************************************/

#ifndef VMOSA_LINUX_KERNEL_SPACE_INCLUDED
#define VMOSA_LINUX_KERNEL_SPACE_INCLUDED



/* Defines
 */

#define   PCI_VENDOR_ID_VMETRO       (0x129A)
#define   PCI_DEVICE_ID_DPIO2        (0xDD11)
#define   PCI_DEVICE_ID_DPIO2_66MHZ  (0xDD12)

#define   PCI_VENDOR_ID_INTEL_31154  (0x8086)
#define   PCI_DEVICE_ID_INTEL_31154  (0x537C)

#define   DEVICE_FIRST_TIME_OPEN  0
#define   DEVICE_HAS_BEEN_OPENED  1


#define MARK \
       printk( KERN_DEBUG "%s, %d : WAS HERE\n", __FILE__,  __LINE__); \
       mdelay (100);

#define ERROR_MSG(args) \
       printk ("DPIO2: ERROR: %s %d\n", __FILE__, __LINE__); \
       printk ("DPIO2: ERROR: "); \
       printk args;

#define DB(args) \
       printk ("DPIO2: Debug msg: %s %d:\n", __FILE__, __LINE__); \
       printk ("DPIO2: "); \
       printk args; \
       mdelay (100);




/* Typedefs
 */

typedef struct dpio2_device_t
{
  int             devno;

  struct pci_dev  *pcidev;

  int             mmap_select;

  void            *ctrl_base;
  unsigned long   ctrl_size;

  void            *fifo_base;
  unsigned long   fifo_size;

  unsigned long   phys_ctrl_base;
  unsigned long   phys_ctrl_end;
  unsigned long   phys_ctrl_size;

  unsigned long   phys_fifo_base;
  unsigned long   phys_fifo_end;
  unsigned long   phys_fifo_size;

  UINT8           interrupt_line;
  UINT8           interrupt_pin;

  struct semaphore  callback_sem;
  struct semaphore  open_mutex;

  int open_status_flag;

  spinlock_t      irq_lock;
  unsigned long   irq_save_flag;

} dpio2_device_t;



typedef struct dpio2_user_map_t
{
  struct page **pages;
  int         nr_pages;

} dpio2_user_map_t;



typedef struct test_kiob
{
  unsigned long uin;
  int in;
  unsigned long uout;
  int out;
} test_kiob;


typedef struct dpio2_time
{
  unsigned long trigg;
  unsigned long isr;
  unsigned long sem;
} dpio2_time;



#endif /* VMOSA_LINUX_KERNEL_SPACE_INCLUDED */
