/*

Copyright (C) 2006 VMETRO ASA 

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later 
version. 

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA. 

*/


/****************************************************************************
Module      : dpio2-linux-kernel-module.c

Description : 

Revision History:
  01b,07oct2004,meh  Removed devfs and in with udev.
  01a,06oct2004,meh  Created base on kernel module for 2.4.x 

*****************************************************************************/


#ifndef __KERNEL__
#  define __KERNEL__
#endif



/* Include 
 */
#include "linux/module.h"
#include "linux/kernel.h"  /* printk, .. */
#include "asm/io.h"        /* ioremap, writel, readl, .. */

// PATCH FOR NEW KERNEL
//#include "asm/uaccess.h"   /* copy_from/to_user, .. */
#include "linux/uaccess.h"   /* copy_from/to_user, .. */

#include "linux/pci.h"
#include "linux/dma-mapping.h"
#include "linux/types.h"
#include "linux/sched.h"
#include "linux/signal.h"
#include "linux/interrupt.h"
#include "linux/pagemap.h" /* page_cache_release, ... */
#include "linux/uaccess.h" /* copy_from_user */

#include "vmosa-linux.h"
#include "dpio2-linux.h"
#include "dpio2-linux-kernel.h"

#include "Dpio2Defs.h"  /* DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER */

#include "release.h"

// David added the following line
#include "irqreturn.h"



/* Set the right GPL license to avoid warrnings then loading the driver 
 */
MODULE_LICENSE("VMETRO DPIO2");
MODULE_AUTHOR("Morten Ebbell Hestnes");
MODULE_DESCRIPTION("DPIO2 Device Driver");



/* Defines
 */
#define DPIO2_MAX_USER_MAP ( MAX_DPIO2_CARDS * 10 )



/* Globals 
 */
static int dpio2_major = 0;
dpio2_device_t   dpio2_device[MAX_DPIO2_CARDS+1];
dpio2_device_t*  dpio2_device_irq_cmp[MAX_DEVICE_IRQ_CMP];
int dpio2_number_of_cards = 0;
dpio2_user_map_t dpio2_user_map_data[DPIO2_MAX_USER_MAP];
static int dpio2_bus_numbers[MAX_DPIO2_DEVICE];
dpio2_hw_info_t dpio2_hw_info[MAX_DPIO2_DEVICE];



/* Forward declaration 
 */
int dpio2_open ( struct inode *inode, struct file *filp );
int dpio2_release ( struct inode *inode, struct file *filp );

// PATCH FOR NEW KERNEL
//int dpio2_ioctl ( struct inode *inode, struct file *filp,
//		  unsigned int, unsigned long);
long dpio2_ioctl ( struct file *filp,
		  unsigned int, unsigned long);

int dpio2_mmap ( struct file *filp, struct vm_area_struct *vma );
static void dpio2_exit ( void );
void free_user_pages ( struct page **pages, int nr_pages );
void save_user_map_data ( struct page **pages, int nr_pages );
void remove_user_map_data ( struct page **pages );
void init_user_map_data ( void );




struct file_operations dpio2_fops = {
    open:    dpio2_open,
    release: dpio2_release,
// PATCH FOR NEW KERNEL
//    ioctl:   dpio2_ioctl,
	unlocked_ioctl: dpio2_ioctl,
    mmap:    dpio2_mmap,
};





#if 0
void busmark (dpio2_device_t  *device, unsigned long  u32)
{
  UINT32 input;
  writel ( u32, device->ctrl_base + DPIO2_GENERAL_PURPOSE_REGISTER );
  readl ( device->ctrl_base + DPIO2_GENERAL_PURPOSE_REGISTER );
}
#endif






/**
 * dpio2_kernel_interrupt_handler
 *
 * handle interrupt routed from a DPIO2 HW module by the kernel
 * 
 * Do:
 * (1) Check if legal interrupt.
 * (2) Disable dpio2 routing it's interrupt to PCI
 * (3) Give semaphore to waiting interrut handle pthread.
 *
 * @param irq
 * @param *dev_id
 * @param *regs (not used)
 *
 * @return IRQ_HANDLED or IRQ_NONE
 */
irqreturn_t dpio2_kernel_interrupt_handler (int irq, 
					    void *device
					    /*struct pt_regs *regs*/)
{


  /* Check interrupt line number against dpio2_device struvt address
   */ 
  if ( dpio2_device_irq_cmp [ irq ] != (dpio2_device_t*)device ) {
    
    ERROR_MSG (("device struct pointer (0x%p) value is corrupted, should be (0x%p)\n",
		device, dpio2_device_irq_cmp [ irq ] ));
    return IRQ_NONE;

  }


  /* Check for NULL pointer
   */
  if ( !device ) { 

    ERROR_MSG (( "NULL pointer!, device\n" ));
    return IRQ_NONE;

  }


  /* Check for NULL pointer
   */
  if ( ((dpio2_device_t*)device)->ctrl_base == NULL ) {

    ERROR_MSG (( "NULL pointer!, device->ctrl_base\n" ));
    return IRQ_NONE;
    
  }


  /* Check the validity of pointer to control base on the DPIO2
   */
  if ( (unsigned long) ((dpio2_device_t*)device)->ctrl_base & (PAGE_SIZE - 1) ){

    ERROR_MSG (( "device->ctrl_base is not PAGE_SIZE aligned!"
		 ", device->ctrl_base 0x%p\n",
		  ((dpio2_device_t*)device)->ctrl_base ));
    return IRQ_NONE;
    
  }


  /* Disable dpio2 routing it's interrupt to PCI
   */
  writel ( 1, ((dpio2_device_t*)device)->ctrl_base + 
	   DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER );

#ifdef NO_READ_AFTER_WRITE
  wmb();
#else
  /* Read the same register to be sure the previous write is done 
     before continue prossesing next instruction.
  */
  readl ( ((dpio2_device_t*)device)->ctrl_base + 
	  DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER );
#endif
  
  /* Give semaphore to the waiting interrupt handle pthread in user space
     which is hanging on a ioctl call 
  */
  up ( &((dpio2_device_t*)device)->callback_sem ); 


  return IRQ_HANDLED;

}





/**
 * dpio2_open
 *
 * Map dpio2 control base and allocate kiovec to speed up map of user kio buffer
 * 
 * Do:
 * (1) Get minor number and set filp->private_data

 * (2) Init kiovec semaphore
 * (3) Allocate kiovec

 * (4) Map DPIO2 control base
 *
 * @param inode
 * @param filp
 *
 * @return OK or ERROR
 */
int dpio2_open (struct inode *inode, struct file *filp) 
{ 

  STATUS status;

  int minor;



  /* Get minor number
   */
  minor = iminor ( inode );


  /* Set filp->private_data 
   */
  filp->private_data = &dpio2_device [minor];


  /* /dev/dpio2/x (x = 1..8) ???  
   */
  if ( dpio2_device [minor].devno > 0 ) {


    /* Mutex down.
       Prevent other threads or process to open the dpio2 device 
    */
    status = down_trylock ( &dpio2_device [minor].open_mutex );

    if ( status != OK ) {
      
      ERROR_MSG (( "DPIO2 Device #%d is busy, status = %d\n",
		   dpio2_device [minor].devno, status ));
      return ERROR;
      
    }


  }

  return OK;

}





/**
 * dpio2_release
 *
 * Map dpio2 control base and allocate kiovec to speed up map of user kio buffer
 * 
 * Do:
 * (1) UnMap control base
 *
 * @param inode
 * @param filp
 *
 * @return OK or ERROR
 */
int dpio2_release(struct inode *inode, struct file *filp)
{ 

  dpio2_device_t  *device;
  


  if ( !filp->private_data ) {
    
    ERROR_MSG (("filp->private_data = NULL pointer\n" ));
    return ERROR;
    
  } else

    device = filp->private_data;


  /* /dev/dpio2/x  (x = 1..8)  ???
   */
  if ( device->devno > 0 ) {
    
    
    /* Mutex up. 
       Permit outer threads or process to open the dpio2 device 
    */
    up ( &device->open_mutex );
    

  }  


  return OK;

}






/**
 * dpio2_mmap
 *
 * Map dpio2 PCI control base to user space adrs
 * 
 * Do:
 * (1) Map control or fifo PCI base to viritual user space adrs
 *
 * @param filp
 * @param vma
 *
 * @return OK or ERROR
 */
STATUS dpio2_mmap (struct file *filp, struct vm_area_struct *vma)
{ 

  int status;

  unsigned long  physical;
  unsigned long  phys_size;
  unsigned long  vsize;

  dpio2_device_t  *device;
 


  if ( !filp->private_data ) {

    ERROR_MSG (("filp->private_data = NULL pointer\n" ));
    return ERROR;

  } else 

    device = (dpio2_device_t*) filp->private_data;


  if ( device->mmap_select == DPIO2_MMAP_CTRL_BASE ) {

    physical = (unsigned long) device->phys_ctrl_base;
    phys_size = (unsigned long) device->phys_ctrl_size;
    vsize = vma->vm_end - vma->vm_start;

  } else {

    physical = (unsigned long) device->phys_fifo_base;
    phys_size = (unsigned long) device->phys_fifo_size;
    vsize = vma->vm_end - vma->vm_start;

  }

  if (vsize > phys_size) {

    ERROR_MSG (("vsize > phys_size. Span too high\n"));
    return ERROR;

  }


  /* Map DPIO2 ctrl base address to user space address
   */
  status = remap_pfn_range (vma, vma->vm_start, physical>>PAGE_SHIFT, 
			     vsize, vma->vm_page_prot);

  if ( status < 0 ) {
    
    ERROR_MSG (("remap_page_range faild, status = %d\n", status));
    return ERROR;

  }
  

  return OK;

}






/**
 * dpio2_ioctl
 *
 *
 * @param *inode
 * @param *filp
 * @param cmd
 * @param arg
 * 
 * @return OK or ERROR
 */

// PATCH FOR NEW KERNEL
//int dpio2_ioctl ( struct inode *inode, struct file *filp,
//		  unsigned int cmd, unsigned long arg )

long dpio2_ioctl (struct file *filp,
		  unsigned int cmd, unsigned long arg )
{ 
  int  status;

  unsigned long  input; 

  unsigned long  output; 

  dpio2_device_t *device;

  dpio2_kio_t  kio;



  /* Get dpio2 data struct pionter 
   */
  if ( !filp->private_data ) {

    ERROR_MSG (("filp->private_data = NULL pointer\n" ));
    return ERROR;

  } else

    device = (dpio2_device_t*) filp->private_data;



  /* Find and execute command
   */
  switch (cmd) {



  case VMOSA_REG_32_READ_MODIFY_WRITE_ATOMIC:
      
    /* Copy user buffer
     */
    status = copy_from_user ( (void*) &kio, 
			      (void*) arg,
			      sizeof (dpio2_kio_t) );
    
    if ( status != OK ) {
      
      ERROR_MSG (("copy_from_user failed\n"));
      return ERROR;
      
    }
    
    
    /* Spin and irq lock 
     */
    spin_lock_irqsave ( &device->irq_lock, device->irq_save_flag );
    
    
    /* Read modify write
     */
    input = readl ( device->ctrl_base +  kio.offset );
    
    output = ( ( input & ~kio.bit_mask ) | 
	       ( kio.u.dword & kio.bit_mask ));  
    
    writel ( output, device->ctrl_base + kio.offset );
    
#ifdef NO_READ_AFTER_WRITE
  wmb();
#else
    /* Read the same register to be sure that the write operation has been done
     */
    readl ( device->ctrl_base +  kio.offset );
#endif
    
    /* spin and irq unlock 
     */
    spin_unlock_irqrestore ( &device->irq_lock, device->irq_save_flag );
    
    
    return OK;
    
  


  case VMOSA_REG_32_BIT_TOGGLE_ATOMIC:
        
    /* Copy user buffer
     */
    status = copy_from_user ( (void*) &kio, 
			      (void*) arg,
			      sizeof (dpio2_kio_t) );
    
    if ( status != OK ) {
      
      ERROR_MSG (("copy_from_user failed\n"));
      return ERROR;
      
    }
    
    
    /* Spin and irq lock 
     */
    spin_lock_irqsave ( &device->irq_lock, device->irq_save_flag );
    
    
    /* Toggle bit
     */
    input = readl ( device->ctrl_base +  kio.offset );
    
    if ( input & kio.bit_mask)
      
      output = input & ~kio.bit_mask;
    
    else 
      
      output = input | kio.bit_mask;
    
    writel ( output, device->ctrl_base +  kio.offset );
    
#ifdef NO_READ_AFTER_WRITE
  wmb();
#else
    /* Read the same register to be sure that the write operation has been done
     */
    readl ( device->ctrl_base +  kio.offset );
#endif
    
    /* spin and irq unlock 
     */
    spin_unlock_irqrestore ( &device->irq_lock, device->irq_save_flag );
    
    
    return OK;
    



  case VMOSA_PCI_CONFIG_8_READ:
    {
      
      UINT8  u8;
   
       
      pci_read_config_byte ( device->pcidev, arg, &u8 );
        
      return u8;

    }



  case VMOSA_PCI_CONFIG_16_READ:
    {
      
      UINT16  u16;
    
	
      pci_read_config_word ( device->pcidev, arg, &u16 );
          
      return u16;

    }



  case VMOSA_PCI_CONFIG_8_WRITE:
    {

      UINT8  u8;

      /* Write to register
       */
      pci_write_config_byte ( device->pcidev, (arg >> 8), (UINT8)(arg & 0x00ff) );
      
      /* Read the same register to be sure that the write operation has been done
       */
      pci_read_config_byte ( device->pcidev, (arg >> 8), &u8 );
      
      
      return OK;

    }



  case DPIO2_DISABLE_ROUTE_INTR_PCI_SAVE_ATOMIC:

    /* Spin and irq lock 
     */
    spin_lock_irqsave ( &device->irq_lock, device->irq_save_flag );
    
    
    /* Save routing status 
     */
    input = readl ( device->ctrl_base + DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER );
    
    
    /* 0 = intr enabled, 1 = intr disabled
     */
    if ( ( input & 0x00000001 ) == 0 ) {
      
      /* Disable routing 
       */
      writel ( 1, device->ctrl_base + DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER );
      
      /* Read to be sure the write is finished before continue 
       */
      readl ( device->ctrl_base + DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER );
      
    }
    
    
    /* Spin and irq unlock 
     */
    spin_unlock_irqrestore ( &device->irq_lock, device->irq_save_flag );
    
    
    /* Return intr route status
     */
    if ( input & 0x00000001 ) {
      
      return 1;
      
    } else {
      
      return 0;
    }

    
    return OK;
    
  


  case DPIO2_WAIT_FOR_INTERRUPT:


    /* Enable dpio2 routing it's interrupt to PCI
     */
    writel ( 0, device->ctrl_base + DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER );

    readl ( device->ctrl_base + DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER );


    status = down_interruptible ( &device->callback_sem );

    if ( status != OK ) {
      
      ERROR_MSG (( "down_interruptible failed, ERESTARTSYS, status = %d\n",
		   status));
      
      if ( device->interrupt_line ) {
	
	free_irq ( device->pcidev->irq, device );

	dpio2_device_irq_cmp [ device->interrupt_line ] = 0;
	device->interrupt_line = 0;

	printk ("DPIO2: Removing IRQ (%d) handle for device %d\n",
		device->pcidev->irq, device->devno );

      }
      
      return ERROR;
      
    }
    
    return OK;

        


  case DPIO2_GIVE_ISR_SEMAPHORE:

    /* Give semaphore to waiting interrupt notifier pthread
     */
    up ( &device->callback_sem );

    return OK;
    



  case DPIO2_INTERRUPT_CONNECT:


    /* Create semaphore
     */
    sema_init ( &device->callback_sem, 0 ); /* locked */

    
    /* Init interrupt handler 
     */
    status = request_irq ( device->pcidev->irq,
			   dpio2_kernel_interrupt_handler,
			   IRQF_SHARED, /* pci.txt document */ 
			   "dpio2_interrupt", device);

    if (status != OK) {

      if ( status == -EBUSY ) {

	ERROR_MSG (( "request_irq failed with status = %d\n"
		     "The interrupt line %d is already in use\n", 
		     status, device->pcidev->irq ));

      } else {

	ERROR_MSG (( "request_irq line %d failed with status = %d\n",
		     device->pcidev->irq, status ));

      }

      return ERROR;
      
    }

    
    device->interrupt_line = device->pcidev->irq;
    dpio2_device_irq_cmp [ device->interrupt_line ] = device;


    printk ("DPIO2: Requested IRQ (%d) handle for device #%d\n",
	    device->pcidev->irq, device->devno );

      
    return OK;




  case DPIO2_INTERRUPT_DISCONNECT:


    /* Free interrupt handler
     */

    if ( device->interrupt_line ) {

      free_irq ( device->pcidev->irq, device );

      dpio2_device_irq_cmp [ device->interrupt_line ] = 0;
      device->interrupt_line = 0;

      printk ("DPIO2: Removing IRQ (%d) handle for device %d\n",
	      device->pcidev->irq, device->devno );

    }

    return OK;




  case DPIO2_SCAN:

    /* Copy pci info to user
     */
    status = copy_to_user ( (void*) arg, 
			    (void*) &dpio2_hw_info[0],
			    MAX_DPIO2_DEVICE * sizeof (dpio2_hw_info_t) );
    
    if ( status != OK ) {
      
      ERROR_MSG (("copy_to_user failed\n"));
      return ERROR;
      
    }      
    
    return dpio2_number_of_cards;


    

  case DPIO2_MMAP_SELECT:


    if ( arg == DPIO2_MMAP_CTRL_BASE ) {

      device->mmap_select = DPIO2_MMAP_CTRL_BASE;

    } else 
      if ( arg == DPIO2_MMAP_FIFO_BASE ) {

      device->mmap_select = DPIO2_MMAP_FIFO_BASE;

    } else {

      ERROR_MSG (("DPIO2_MMAP_SELECT: invalid arg = %d\n", (int) arg));
      return ERROR;

    }

    return OK;




  case DPIO2_GET_HW_INFO: 
    {
      
      /* Copy pci info to user
       */
      status = copy_to_user ( (void*) arg, 
			      (void*) &dpio2_hw_info[0],
			      MAX_DPIO2_DEVICE * sizeof (dpio2_hw_info_t) );
      
      if ( status != OK ) {
	
	ERROR_MSG (("copy_to_user failed\n"));
	return ERROR;
	
      }      

      return OK;

    }




  case DPIO2_GET_PCI_CONFIG_INFO: 
    {
      
      dpio2_pci_info_t   pci_info;
	

      /* Set PCI info
       */
      pci_info.vendor_id = device->pcidev->vendor;
      pci_info.device_id = device->pcidev->device;
      pci_read_config_byte ( device->pcidev,
			     PCI_REVISION_ID,
			     &pci_info.revision_id );
      pci_info.device_number = PCI_SLOT ( device->pcidev->devfn );
      pci_info.bus_number = device->pcidev->bus->number;


      /* Copy pci info to user
       */
      status = copy_to_user ( (void*) arg, 
			      (void*) &pci_info,
			      sizeof (dpio2_pci_info_t) );
      
      if ( status != OK ) {
	
	ERROR_MSG (("copy_to_user failed\n"));
	return ERROR;
	
      }      

      return OK;

    }



  case VMOSA_DPIO2_DMA_LOCK_MEMORY:
    {
      
      dpio2_user_dma_page_t*  pDma = (void*) arg;

      struct pci_dev*         pci_handle = NULL;

      struct device*          device_handle = NULL;

      size_t                  buffersize = 0;

      dma_addr_t              busAddress = 0;

      void*                   kernelVirtualAddress = NULL;

      void*                   memset_return = NULL;

/*
**    Get the address of the pci_dev structure of the dpio2 card.
*/
      pci_handle = pci_get_device (PCI_VENDOR_ID_VMETRO, PCI_DEVICE_ID_DPIO2, NULL);
      if ( pci_handle == NULL )
      {
	  ERROR_MSG (("pci_get_device failed\n"));
	  return -ENOMEM;	
      }

//      ERROR_MSG (("pci_handle = %p\n", pci_handle));
//      ERROR_MSG (("vendor = %d\n", pci_handle->vendor));
//      ERROR_MSG (("device = %d\n", pci_handle->device));

/*
**    Allocate memory and get it's virtual and bus addresses.
*/
      device_handle = &pci_handle->dev;

//      ERROR_MSG (("device_handle = %p\n", device_handle));
//      ERROR_MSG (("coherent_dma_mask = %lx\n", device_handle->coherent_dma_mask));

      buffersize = pDma->kernel_dma_buffer_size;
      kernelVirtualAddress = dma_alloc_coherent (device_handle, buffersize, &busAddress, GFP_ATOMIC);
      if ( kernelVirtualAddress == NULL )
      {
	  ERROR_MSG (("dma_alloc_coherent failed\n"));
	  return -ENOMEM;	
      }

//      ERROR_MSG (("bus address = %x\n", busAddress));
//      ERROR_MSG (("kernel virtual address = %p\n", kernelVirtualAddress));

      memset_return = memset(kernelVirtualAddress, 0, buffersize);

      memset_return = memset(kernelVirtualAddress, '=', 2);

/*
**    Pass the results.
*/
      pDma->handle = (void*) device_handle;
      pDma->kernel_dma_buffer_adrs = kernelVirtualAddress;
      pDma->dma_buffer_bus_adrs = (unsigned int) busAddress;

      return OK;
    }


  case VMOSA_DPIO2_DMA_COPY_BUFFER:
    {
      dpio2_copy_from_user_t* pCopyValues = (void*) arg;

      void* kernelAddress   = pCopyValues->kernel_buffer_addr;
      void* userAddress     = pCopyValues->user_buffer_addr;
      unsigned long iLength = pCopyValues->buffer_length;

      unsigned long uncopied = 0;

//      printk("kernel address = %p, user address = %p, length = %lu\n", kernelAddress, userAddress, iLength);

      uncopied = copy_from_user (kernelAddress, userAddress, iLength);

      return (int) uncopied;
    }


  case VMOSA_DPIO2_DMA_UNLOCK_MEMORY:
    {
      dpio2_user_dma_page_t*  pDma = (void*) arg;

      struct device*          device_handle = NULL;

      size_t                  buffersize = 0;

      dma_addr_t              busAddress = 0;

      void*                   kernelVirtualAddress = NULL;
/*
**    Free the dma buffer.
*/
      device_handle = (struct device*) pDma->handle;
      buffersize = pDma->kernel_dma_buffer_size;
      kernelVirtualAddress = pDma->kernel_dma_buffer_adrs;
      busAddress = (dma_addr_t) pDma->dma_buffer_bus_adrs;

//      ERROR_MSG (("Unlock memory data\n"));
//      ERROR_MSG (("device_handle = %p\n", device_handle));
//      ERROR_MSG (("buffersize = %lx\n", buffersize));
//      ERROR_MSG (("bus address = %x\n", busAddress));
//      ERROR_MSG (("kernel virtual address = %p\n", kernelVirtualAddress));

      dma_free_coherent (device_handle, buffersize, kernelVirtualAddress, busAddress);

      return OK;
    }



  case SET_INTEL_31154_P2P_PREFETCH_POLICY:
    {

      struct pci_dev  *p2p_dev = NULL;

      UINT16 prefetch_policy;
      UINT8  cacheline_size;

      int    count = 0;



      /* Check for Intel 31154 P2P Brigde
	 Set prefetch registers to optimal PCI <--> PCI-X performance.
      */
    
      while ( ( p2p_dev = pci_get_device ( PCI_VENDOR_ID_INTEL_31154,
					    PCI_DEVICE_ID_INTEL_31154,
					    p2p_dev ) ) ) {
    
	/* Set prefetch policy
	 */
	prefetch_policy = 0x0000ffff & arg;

	pci_write_config_word ( p2p_dev, 0x52, prefetch_policy );
     

	/* Set cache line size
	 */
	cacheline_size = 0x000000ff & ( arg >> 16);

	pci_write_config_byte ( p2p_dev, 0x0c, cacheline_size );
	
	pci_read_config_byte ( p2p_dev, 0x0c, &cacheline_size );


	printk ("DPIO2: Found Intel 31154 P2P Brigde. "
		"Prefetch policy 0x%x. Cacheline size 0x%x.\n",
		(int)prefetch_policy, (int)cacheline_size );
    
      }

      
      count++;
      
      if ( count == 0 ) {

	ERROR_MSG (("DPIO2: Did not found any Intel 31154 P2P Brigde!\n"));
	return ERROR;

      } else 

	return count;

    }




  default:

    ERROR_MSG (( "Ilegal ioctl() command\n" ));

    return -EINVAL;



  } /* switch */


  return OK;

}




void init_user_map_data (void)
{

  memset ( (void*) &dpio2_user_map_data[0], 
	   0, 
	   DPIO2_MAX_USER_MAP * sizeof (dpio2_user_map_t) );

}


void save_user_map_data ( struct page **pages, int nr_pages )
{

  int i;

  for ( i = 0; i < DPIO2_MAX_USER_MAP; i++ ) {

    if ( dpio2_user_map_data[ i ].pages == NULL ) {

      dpio2_user_map_data[ i ].pages = pages;
      dpio2_user_map_data[ i ].nr_pages = nr_pages;
      break;      
      
    }

  }  

}


void remove_user_map_data ( struct page **pages )
{

  int i;

  for ( i = 0; i < DPIO2_MAX_USER_MAP; i++ ) {

    if ( dpio2_user_map_data[ i ].pages == pages ) {

      dpio2_user_map_data[ i ].pages = NULL;
      dpio2_user_map_data[ i ].nr_pages = 0;
      break;      
      
    }

  }  



}


void free_user_pages ( struct page **pages, int nr_pages )
{

  int page_no;


  /* Unmap pages
   */
  for ( page_no = 0; page_no < nr_pages; page_no++ ) {
    
    kunmap ( pages[ page_no ] );
    
  }
  
  
  /* Set each cache page dirty
   */
  for ( page_no = 0; page_no < nr_pages; page_no++ ) {
    
    if ( !PageReserved ( pages[ page_no ] ) )
      
      SetPageDirty ( pages[ page_no ] );
    
  }
  
  
  /* Every mapped page must be released from the page cache
   */
  for ( page_no = 0; page_no < nr_pages; page_no++ )
    // PATCH FOR NEW KERNEL
    //page_cache_release ( pages[ page_no ] );
  put_page ( pages[ page_no ] );
  return;
  
}








/**
*/
STATUS dpio2_init_pci ( dpio2_device_t  *device )
{
  
  STATUS  status;

  int     devno = device->devno;



  /* Enable PCI device
   */
  status = pci_enable_device ( device->pcidev );
  
  if ( status != OK ) {
      
    ERROR_MSG (("pci_enable_device failed\n"));
    return ERROR;
    
  }


  /* Set latency timer to max  */
  pci_write_config_byte ( device->pcidev, 0x0c, 0xff );
   
  
  /* Get bus number */
  dpio2_bus_numbers [devno] = device->pcidev->bus->number;

    
  /* Get phys PCI base adrs info
   */
  device->phys_ctrl_base = pci_resource_start ( device->pcidev, 0);
  device->phys_ctrl_end = pci_resource_end ( device->pcidev, 0);
  device->phys_ctrl_size = device->phys_ctrl_end - device->phys_ctrl_base + 1;
  
  device->phys_fifo_base = pci_resource_start ( device->pcidev, 1);
  device->phys_fifo_end = pci_resource_end ( device->pcidev, 1);
  device->phys_fifo_size = device->phys_fifo_end - device->phys_fifo_base + 1;
  
#if 0
  printk ("device->phys_ctrl_base 0x%x\n",(int)device->phys_ctrl_base);
  printk ("device->phys_fifo_base 0x%x\n",(int)device->phys_fifo_base);
#endif
  
  /* Map PCI control base to virtual adrs 
   */
  device->ctrl_base = ioremap_nocache ( device->phys_ctrl_base,
					device->phys_ctrl_size );
  
  
  /* Get dpio2 info (pci, fpga,...)
   */
  dpio2_hw_info [devno].devno = devno;
  
  pci_read_config_word ( device->pcidev, 0x00, &dpio2_hw_info [devno].vendorId );
  
  pci_read_config_word ( device->pcidev, 0x02, &dpio2_hw_info [devno].deviceId );
  
  pci_read_config_byte ( device->pcidev, 0x08, &dpio2_hw_info [devno].revisionId );
  
  dpio2_hw_info [devno].pciBusNumber = device->pcidev->bus->number;
  
  dpio2_hw_info [devno].pciDeviceNumber = PCI_SLOT ( device->pcidev->devfn );
  
  dpio2_hw_info [devno].moduleType = 
    readb ( device->ctrl_base + DPIO2_FRONT_END_PERSONALITY_ID_REGISTER );

  dpio2_hw_info [devno].pciFpgaVersion = 
    readb ( device->ctrl_base +  DPIO2_PCI_VERSION_REGISTER );
  
  dpio2_hw_info [devno].frontEndFpgaVersion = 
    readb ( device->ctrl_base + DPIO2_FRONT_END_VERSION_REGISTER );

  
  return OK;

}











/**
 * dpio2_init
 *
 * Scan for dpio2 cards, create a device for each and initialize PCI interface.
 * 
 * Do:
 * (1) Clears data struct info
 * (2) Create /dev/dpio2
 * (3) Create /dev/dpio2/manager
 * (4) Scan for dpio2 cards
 * (5) Create /dev/dpio2/x device for each
 * (6) Set the global dpio2_number_of_cards variable
 *
 * @param cards_no      pointer to number of dpio2 cards.
 *
 * @return OK or ERROR
 */
static int dpio2_init(void)      
{ 

  int status;
  int num_device = 0;
  int count;

  struct pci_dev  *pcidev = NULL;
  struct pci_dev  *p2p_dev = NULL;
  struct pci_dev  *p2p_dev_any = NULL;

  unsigned short  prefetch_policy;
  unsigned char   cache_line_size;
  unsigned char   primary_latency_timer;
  unsigned char   primary_bus_number;
  unsigned char   secondary_bus_number;



  printk ("DPIO2: DPIO2 Device Driver - Release: %s\n"
	  "       Copyright (C) 2004 - VMETRO, Inc.  All rights reserved.\n"
	  "       Creation date: %s\n",
	  releaseAsText, __DATE__ ", " __TIME__ );
  

  /* Register DPIO2 char device driver.
     Get major number dynamical 
   */
  status = register_chrdev( 0, DPIO2_DEVICE_NAME, &dpio2_fops);

  if (status < 0) {
 
    ERROR_MSG (("register_chrdev failed, status %d\n", status));
    return status;

  } else {

    dpio2_major = status;
    printk ("DPIO2: Register char device driver.\n"
	    "DPIO2: Dynamical allocated major = %d\n", dpio2_major);
    
  }
  
  
  /* Clear dpio2_device struct
   */
  memset ( (void*) &dpio2_device[0], 0, 
	   MAX_DPIO2_CARDS * sizeof (dpio2_device_t) );


  /* Clear irq line compare arry
   */
  memset ( (void*) &dpio2_device_irq_cmp[0], 0, 
	   MAX_DEVICE_IRQ_CMP * sizeof (dpio2_device_t*) );


  /* Clear dpio2_hw_info
   */
  memset ( (void*) &dpio2_hw_info[0], 0, 
	   MAX_DPIO2_DEVICE * sizeof (dpio2_hw_info_t) );


  /* Init dpoi2 bus number table
   */
  for (count = 0; count <= MAX_DPIO2_CARDS; count++) 

    dpio2_bus_numbers [count] = -1;


  /* Init user map data array
   */
  init_user_map_data ();
  

  printk ("DPIO2: Scan for DPIO2 hardware devices:\n");



  /* Check for dpio2 33MHz 
   */
  while ( ( pcidev = pci_get_device ( PCI_VENDOR_ID_VMETRO,
				       PCI_DEVICE_ID_DPIO2,
				       pcidev ) ) ) {


    /* Add number of dpio2 modules found */
    num_device++;    


    printk ("DPIO2: Device #%d: DPIO2_33MHz\n", num_device );

 
    if ( num_device > MAX_DPIO2_CARDS ) {

      ERROR_MSG (("pci_enable_device failed\n"));
      dpio2_exit ();
      return ERROR;
      
    }

    
    /* Set device data */
    dpio2_device [num_device].devno = num_device;
    dpio2_device [num_device].pcidev = pcidev;
    

    /* Map PCI resources. Get HW info.
     */
    status = dpio2_init_pci ( &dpio2_device [num_device] );

    if ( status != OK ) {

      ERROR_MSG (("dpio2_init_pci failed\n"));
      dpio2_exit ();
      return ERROR;
    
    }

  }



  /* Check for dpio2 66MHz 
   */
  pcidev = NULL;
  
  while ( ( pcidev = pci_get_device ( PCI_VENDOR_ID_VMETRO,
				       PCI_DEVICE_ID_DPIO2_66MHZ,
				       pcidev ) ) ) {


    /* Add number of dpio2 modules found */
    num_device++;    


    printk ("DPOP2: Device #%d: DPIO2_66MHz\n", num_device );

    
    if ( num_device > MAX_DPIO2_CARDS ) {

      ERROR_MSG (("pci_enable_device failed\n"));
      dpio2_exit ();
      return ERROR;
      
    }

    
    /* Set device data */
    dpio2_device [num_device].devno = num_device;
    dpio2_device [num_device].pcidev = pcidev;
    

    /* Map PCI resourches. Get HW info.
     */
    status = dpio2_init_pci ( &dpio2_device [num_device] );

    if ( status != OK ) {

      ERROR_MSG (("dpio2_init_pci failed\n"));
      dpio2_exit ();
      return ERROR;
    
    }
    
  }
  

  /* Set global variable
   */
  dpio2_number_of_cards = num_device;
  



  /* Check for Intel 31154 P2P Brigde
     Set prefetch registers to optimal PCI <--> PCI-X performance.
   */
  while ( ( p2p_dev = pci_get_device ( PCI_VENDOR_ID_INTEL_31154,
					PCI_DEVICE_ID_INTEL_31154,
					p2p_dev ) ) ) {
    
    /* Enable PCI device
     */
    status = pci_enable_device ( p2p_dev );

    if (status != 0) {
      
      ERROR_MSG (("pci_enable_device failed\n"));
      dpio2_exit ();
      return ERROR;
    
    }


    /* Set prefetch policy
     */
    pci_read_config_word ( p2p_dev, 0x52, &prefetch_policy );
    
    prefetch_policy |= 0x03ff;

    pci_write_config_word ( p2p_dev, 0x52, prefetch_policy );

    pci_read_config_word ( p2p_dev, 0x52, &prefetch_policy );
 

    /* Set cache line size
     */
    pci_write_config_byte ( p2p_dev, 0x0c, 0x10 );
   
    pci_read_config_byte ( p2p_dev, 0x0c, &cache_line_size );


    /* Set primary latency timer
     */
    pci_write_config_byte ( p2p_dev, 0x0d, 0xff );

    pci_read_config_byte ( p2p_dev, 0x0d, &primary_latency_timer );
         
    
    /* Get bus number
     */
    pci_read_config_byte ( p2p_dev, 0x18, &primary_bus_number );

    pci_read_config_byte ( p2p_dev, 0x19, &secondary_bus_number );


    /* Set bus number
     */
    for ( count = 1; count <= MAX_DPIO2_CARDS; count++ ) {
      
      if ( dpio2_bus_numbers[count] == -1 )
	
	break;
      
      if ( dpio2_bus_numbers[count] == secondary_bus_number )
	
	dpio2_bus_numbers[count] = primary_bus_number;
      
    }


    printk ("DPIO2: Found Intel 31154 P2P Brigde: "
	    "Bus (Primary #%d, Secondary #%d)\n"
	    "       Prefetch policy 0x%x. Cacheline size 0x%x. "
	    "Primary latency timer 0x%x.\n",
	    (int)primary_bus_number, (int)secondary_bus_number,
	    (int)prefetch_policy, (int)cache_line_size, 
	    (int)primary_latency_timer );


  }





  /* Check for other P2P Brigde
   */
  while ( ( p2p_dev_any = pci_get_class ( 0x60400, p2p_dev_any ) ) ) {
    
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned char  cacheline_size;
    unsigned char  pri_latency_timer;
    unsigned char  sec_latency_timer;



    /* Enable PCI device
     */
    status = pci_enable_device ( p2p_dev_any );

    if (status != 0) {
      
      ERROR_MSG (("pci_enable_device failed\n"));
      dpio2_exit ();
      return ERROR;
    
    }


    pci_read_config_word ( p2p_dev_any, 0x00, &vendor_id );
    pci_read_config_word ( p2p_dev_any, 0x02, &device_id );
    pci_read_config_byte ( p2p_dev_any, 0x18, &primary_bus_number );
    pci_read_config_byte ( p2p_dev_any, 0x19, &secondary_bus_number );
    pci_read_config_byte ( p2p_dev_any, 0x0c, &cacheline_size );
    pci_read_config_byte ( p2p_dev_any, 0x0d, &pri_latency_timer );
    pci_read_config_byte ( p2p_dev_any, 0x1b, &sec_latency_timer );


    printk ("DPIO2: Found P2P: VendorID 0x%x, DeviceID 0x%x, "
	    "Bus (pri #%d, sec #%d)\n"
	    "       cacheline size 0x%x, latency timer (pri 0x%x, sec 0x%x)\n",
	    vendor_id, device_id, primary_bus_number, secondary_bus_number,
	    cacheline_size, pri_latency_timer, sec_latency_timer );


#if 0
    for ( count = 1; count <= MAX_DPIO2_CARDS; count++ ) {

      if ( dpio2_bus_numbers[count] == -1 )
	
	break;
 
      if ( dpio2_bus_numbers[count] == secondary_bus_number ) {

	/* Set primary latency timer to 0xff
	 */
	pci_write_config_byte ( p2p_dev_any, 0x0d, 0xf8 );


	pci_read_config_byte ( p2p_dev_any, 0x0d, &u8 );

	printk ("primary latency timer 0x%x\n",(int)u8);


	pci_write_config_byte ( p2p_dev_any, 0x0c, 0x10 );

	pci_read_config_byte ( p2p_dev_any, 0x0c, &u8 );

	printk ("cache line size 0x%x\n",(int)u8);


      }

    }
#endif

  }




  /* Initialize spin lock
   */
  for ( count = 0; count <= num_device; count++ )
    
    spin_lock_init ( &dpio2_device [count].irq_lock );
  

  /* Initialize mutex
   */
  for ( count = 1; count <= num_device; count++ )
    
    sema_init ( &dpio2_device [count].open_mutex, 1 ); /* free */




  return OK;

}



static void dpio2_exit(void)
{ 

  dpio2_device_t  *device;

  int i;


  for ( i = 1; i <= dpio2_number_of_cards; i++ ) {
    
    device = &dpio2_device[i];


    /* Free interrupt handler 
     */
    if ( device->interrupt_line ) {
    
      free_irq ( device->interrupt_line, device );

      dpio2_device_irq_cmp [ device->interrupt_line ] = 0;
      device->interrupt_line = 0;

      printk ("DPIO2: Removing IRQ (%d) handle for device %d\n",
	      device->pcidev->irq, device->devno);

    }


    /* Unmap DPIO2 control base
     */
    if ( device->ctrl_base ) {
      
      printk ("DPIO2: iounmap DPIO2 PCI control base for device %d\n",
	      device->devno);
      iounmap ( device->ctrl_base );
      device->ctrl_base = 0;      

    }
    
  }


  
  /* Free kmapped pages
   */
  for ( i = 0; i < DPIO2_MAX_USER_MAP; i++ ) {

    if ( dpio2_user_map_data[ i ].pages != NULL ) {

      free_user_pages ( dpio2_user_map_data[ i ].pages,
			dpio2_user_map_data[ i ].nr_pages );

      kfree ( (void*) dpio2_user_map_data[ i ].pages );

      dpio2_user_map_data[ i ].pages = NULL;
      dpio2_user_map_data[ i ].nr_pages = 0;
      
    }

  }  


  /* Unregister DPIO2 char device driver
   */
  if ( dpio2_major ) {
    
    printk ( "DPIO2: Unregister char device driver\n" );
    unregister_chrdev ( dpio2_major, DPIO2_DEVICE_NAME );

  }


  /* Clear dpio2_device struct
   */
  memset ( (void*) &dpio2_device[0], 0, 
	   MAX_DPIO2_DEVICE * sizeof (dpio2_device_t) );


  printk ("DPIO2: Exit\n");


  return;

}

 
module_init ( dpio2_init );
module_exit ( dpio2_exit );

