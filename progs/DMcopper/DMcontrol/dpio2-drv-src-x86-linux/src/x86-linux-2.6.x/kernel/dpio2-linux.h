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



Module      : vmosa-linux.h

Description : Header file declaring linux in vmosa

Revision History:
  01a,06jul2004,meh  Created.

*****************************************************************************/

#ifndef DPIO2_LINUX_INCLUDED
#define DPIO2_LINUX_INCLUDED



#include "vmosa-linux.h"



#if 1
#ifdef __cplusplus
extern "C" {
#endif
#endif


#if 1
#define NO_READ_AFTER_WRITE
#endif

#define IOCTL_CODE  ( 0xAAAA0000 )

#define VMOSA_REG_32_READ_MODIFY_WRITE_ATOMIC ( 1 + IOCTL_CODE )
#define VMOSA_REG_32_BIT_TOGGLE_ATOMIC        ( 2 + IOCTL_CODE )

#define VMOSA_PCI_CONFIG_8_READ               ( 10 + IOCTL_CODE )
#define VMOSA_PCI_CONFIG_8_WRITE              ( 11 + IOCTL_CODE )
#define VMOSA_PCI_CONFIG_16_READ              ( 12 + IOCTL_CODE )

#define VMOSA_DPIO2_DMA_LOCK_MEMORY      (71 + IOCTL_CODE )
#define VMOSA_DPIO2_DMA_UNLOCK_MEMORY    (72 + IOCTL_CODE )
#define VMOSA_DPIO2_DMA_COPY_BUFFER      (73 + IOCTL_CODE )


#define DPIO2_SCAN                      ( 20 + IOCTL_CODE )
#define DPIO2_GET_PCI_CONFIG_INFO       ( 21 + IOCTL_CODE )
#define DPIO2_GET_HW_INFO               ( 22 + IOCTL_CODE )

#define DPIO2_DISABLE_ROUTE_INTR_PCI_SAVE_ATOMIC  ( 30 + IOCTL_CODE )
#define DPIO2_ENABLE_ROUTE_INTR_PCI_SAVE          ( 31 + IOCTL_CODE )

#define DPIO2_INTERRUPT_CONNECT         ( 40 + IOCTL_CODE )
#define DPIO2_INTERRUPT_DISCONNECT      ( 41 + IOCTL_CODE )

#define DPIO2_WAIT_FOR_INTERRUPT        ( 50 + IOCTL_CODE )
#define DPIO2_GIVE_ISR_SEMAPHORE        ( 51 + IOCTL_CODE )

#define DPIO2_MMAP_SELECT         (61 + IOCTL_CODE )
#define    DPIO2_MMAP_CTRL_BASE   (0)
#define    DPIO2_MMAP_FIFO_BASE   (1)

#define SET_INTEL_31154_P2P_PREFETCH_POLICY (81 + IOCTL_CODE )




#define DPIO2_DEVICE_NAME       "dpio2"
#define DPIO2_DEVICE_PATH_NAME  "/dev/dpio2/"

#define DPIO2_MANAGER_NAME      "manager"
#define DPIO2_DEVICE_MANAGER    "/dev/dpio2/manager"

#define DPIO2_PCI_CTRL_WINDOW_SIZE 0x00800000
#define DPIO2_PCI_FIFO_WINDOW_SIZE 0x00800000


#define MAX_DPIO2_CARDS     (8)
#define MAX_DPIO2_DEVICE    (MAX_DPIO2_CARDS + 1)
#define MAX_DEVICE_IRQ_CMP  (1024)




typedef struct dpio2_kio_t
{
  int     devno;
  UINT32  offset;
  UINT32  bit_mask;
  int     bit_number;
  union {
    UINT8   byte;
    UINT16  word;
    UINT32  dword;
  } u;
} dpio2_kio_t;


typedef struct dpio2_pci_info_t
{
  UINT16  vendor_id;
  UINT16  device_id;
  UINT8   revision_id;

  UINT8   device_number;
  UINT8   bus_number;
} dpio2_pci_info_t;


typedef struct dpio2_user_dma_page_t
{
  void*             handle;                  /* pointer to the dpio2 device structure in the kernel */
  void*             kernel_dma_buffer_adrs;  /* start adrs to dma buffer in the kernel*/
  unsigned long     kernel_dma_buffer_size;  /* size of dma buffer in bytes */
  unsigned int      dma_buffer_bus_adrs;     /* bus adrs - this should be defined as type dma_addr_t, but we
                                                           have to define it as unsigned intsince dma_addr_t
                                                           isn't defined outside the kernel */
} dpio2_user_dma_page_t;


typedef struct dpio2_copy_from_user_t
{
  void*          kernel_buffer_addr;
  void*          user_buffer_addr;
  unsigned long  buffer_length;

} dpio2_copy_from_user_t;


typedef struct dpio2_kio_dma_t
{
  void* dma_page_struct_user_adrs;
  int   dma_page_struct_size;

} dpio2_kio_dma_t;




typedef struct dpio2_hw_info_t {

  int     devno;
  UINT16  deviceId;
  UINT16  vendorId;
  UINT8   revisionId;
  UINT8   moduleType;
  UINT8   pciBusNumber;
  UINT8   pciDeviceNumber;
  UINT8   pciFpgaVersion;
  UINT8   frontEndFpgaVersion;

} dpio2_hw_info_t;




#if 1
#ifdef __cplusplus
}
#endif
#endif

#endif /* DPIO2_LINUX_INCLUDED */


