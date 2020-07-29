/****************************************************************************
Module      : vmosa-dpio2-user-space.c

Description : 

Copyright(c): 2004 VMETRO asa. All Rights Reserved.

Revision History:
  01a,23jun2004,meh  Created.

*****************************************************************************/

#include "vmosa-linux.h"
#include "dpio2-linux.h"
#include "vmosa-linux-user-space.h"
#include "dpio2-linux-user-space.h"

#include "dpio2.h"
#include "Dpio2Defs.h"






/**
 * vmosa_reg_32_read
 *
 * Read a 32 bits dpio2 register from ( offset + PCI control base )
 *
 * @param HANDLE   handle
 * @param offset   dpio2 register offset from PCI control base 
 * @param dword    return value
 *
 * @return OK
 */
STATUS vmosa_reg_32_read ( HANDLE handle, UINT32 offset, UINT32 *dword )
{

  if ( offset > (UINT32) handle->user.ctrl_mmap_size )

    return ERROR;


  /* Read from register
   */
  *dword = *( (UINT32*) ( handle->user.ctrl_base + offset ) );


  return OK;

}



/**
 * vmosa_reg_32_write
 *
 * Write 32 bits to dpio2 register ( offset + PCI control base )
 *
 * @param HANDLE   handle
 * @param offset   dpio2 register offset from PCI control base 
 * @param dword    input value
 *
 * @return OK
 */
STATUS vmosa_reg_32_write ( HANDLE handle, UINT32 offset, UINT32 dword )
{

  if ( offset > (UINT32) handle->user.ctrl_mmap_size )
    
    return ERROR;

  /* Write to register
   */
  *( (UINT32*) ( handle->user.ctrl_base + offset ) ) = dword;

#ifndef NO_READ_AFTER_WRITE
  /* Read the same register to be sure that the write operation has been done
   */
  *( (UINT32*) ( handle->user.ctrl_base + offset ) );
#endif

  return OK;

}



/**
 * vmosa_reg_32_read_modify_write_atomic
 *
 * Set or clear bit(s) in dpio2 register ( offset + PCI control base )
 * according to a bit mask and a dword variable
 *
 * @param HANDLE    handle
 * @param offset    dpio2 register offset from PCI control base 
 * @param bit_mask  bit mask
 * @param dword     input value
 *
 * @return OK or ERROR
 */
STATUS vmosa_reg_32_read_modify_write_atomic ( HANDLE handle, UINT32 offset,
					       UINT32 bit_mask, UINT32 dword )
{

  dpio2_kio_t  kio;



  if ( offset > (UINT32) handle->user.ctrl_mmap_size )
    
    return ERROR;


  kio.offset = offset;
  kio.bit_mask = bit_mask;
  kio.u.dword = dword;


  return ( ioctl ( handle->fd, VMOSA_REG_32_READ_MODIFY_WRITE_ATOMIC, &kio ) );

}




/**
 * vmosa_reg_32_bit_toggle_atomic
 *
 * Toggle a bit in dpio2 register ( offset + PCI control base )
 * according to bit number
 *
 * @param HANDLE      handle
 * @param offset      dpio2 register offset from PCI control base 
 * @param bit_number  bit number
 *
 * @return OK or ERROR
 */
STATUS vmosa_reg_32_bit_toggle_atomic ( HANDLE handle, UINT32 offset, 
					int bit_number)
{

  dpio2_kio_t  kio;
  


  if ( offset > (UINT32) handle->user.ctrl_mmap_size )
    
    return ERROR;


  kio.offset = offset;

  
  if ( ( bit_number < 0 ) || ( bit_number > 31 ) )
    
    return ERROR;
  
  else
    
    kio.bit_mask = ( 0x00000001 << bit_number );
  

  return ( ioctl ( handle->fd, VMOSA_REG_32_BIT_TOGGLE_ATOMIC, &kio ) );

}




/**
 * vmosa_pci_config_8_read
 *
 * Read dpio2 PCI config register ( offset )
 *
 * @param HANDLE      handle
 * @param offset      dpio2 register offset from PCI control base 
 * @param byte        return value
 *
 * @return OK or ERROR
 */
STATUS vmosa_pci_config_8_read ( HANDLE handle, UINT8 offset, UINT8 *byte )
{

  STATUS     status;


  status = ioctl ( handle->fd, VMOSA_PCI_CONFIG_8_READ, offset );

  if ( status < 0 )

    return ERROR;

  else

    *byte = (UINT8) status;


  return OK;

}



/**
 * vmosa_pci_config_16_read
 *
 * Read dpio2 PCI config register ( offset )
 *
 * @param HANDLE      handle
 * @param offset      dpio2 register offset from PCI control base 
 * @param word        return value
 *
 * @return OK or ERROR
 */
STATUS vmosa_pci_config_16_read ( HANDLE handle, UINT8 offset, UINT16 *word )
{

  STATUS     status;


  status = ioctl ( handle->fd, VMOSA_PCI_CONFIG_16_READ, &offset );

  if ( status < 0 )

    return ERROR;

  else

    *word = (UINT16) status;

  return OK;

}





/**
 * vmosa_pci_config_8_write
 *
 * Write dpio2 PCI config register ( offset )
 *
 * @param HANDLE      handle
 * @param offset      dpio2 register offset from PCI control base 
 * @param byte        input value
 *
 * @return OK or ERROR
 */
STATUS vmosa_pci_config_8_write ( HANDLE handle, UINT8 offset, UINT8 byte)
{

  UINT16  u16;

  
  u16 = offset;
  u16 <<= 8;
  u16 |= byte;

  return ( ioctl ( handle->fd, VMOSA_PCI_CONFIG_8_WRITE, &u16 ) );

}





/**
 * vmosa_mdelay
 *
 * Waits for n miliseconds
 *
 * @param mili_seconds  number of miliseconds
 *
 * @return 
 */
void vmosa_mdelay ( UINT  mili_seconds )
{

  struct timespec  delay;


  if ( mili_seconds >= 1000 ) {

    delay.tv_sec = mili_seconds / 1000;
    delay.tv_nsec = (long) ( ( mili_seconds % 1000 ) * 1000 * 1000 );

  } else {

    delay.tv_sec = 0;
    delay.tv_nsec = (long) ( mili_seconds * 1000 * 1000 );

  }

  nanosleep ( &delay, NULL );

}



/**
 * vmosa_mdelay
 *
 * Waits for n miliseconds
 *
 * @param mili_seconds  number of miliseconds
 *
 * @return 
 */
void vmosa_udelay ( UINT  micro_seconds )
{

  struct timespec  delay;


  if ( micro_seconds >= (1000 * 1000) ) {

    delay.tv_sec = micro_seconds / (1000*1000);
    delay.tv_nsec = (long) ( ( micro_seconds % (1000*1000) ) * 1000 );

  } else {

    delay.tv_sec = 0;
    delay.tv_nsec = (long) ( micro_seconds * 1000 );

  }

  nanosleep ( &delay, NULL );

}




/**
 * vmosa_dpio2_dma_lock_memory
 *
 * Map and lock all memory pages.
 * Create list of PCI adrs to each pages.
 *
 */
STATUS vmosa_dpio2_dma_lock_memory ( dpio2_user_dma_page_t* pDma )
{

    STATUS   status;
  
    int  fd;

    fd = open ( DPIO2_DEVICE_MANAGER, O_RDONLY );

    if ( fd == ERROR )
    { /*errno?*/

        DPIO2_ERR_MSG (("open %s failed! fd = %d, errno = %d\n",
		        DPIO2_DEVICE_MANAGER, fd, errno ));
        return ERROR;
    
    }
#ifdef RSR_DEBUG
    printf("\nContents of dpio2_user_dma_page_t before kernel dma buffer creation:\n\n");
    printf("dpio2 device structure pointer = %p\n", pDma->handle);
    printf("kernel_dma_buffer_adrs = %p\n", pDma->kernel_dma_buffer_adrs);
    printf("kernel_dma_buffer_size = %d\n", pDma->kernel_dma_buffer_size);
    printf("dma_buffer_bus_adrs = %p\n", pDma->dma_buffer_bus_adrs);
#endif

  status = ioctl ( fd, VMOSA_DPIO2_DMA_LOCK_MEMORY, pDma );

//  Print the contents of dpio2_user_dma_page_t after the call to ioctl
    printf("\nContents of dpio2_user_dma_page_t after dma buffer created in kernel:\n\n");
    printf("dpio2 device structure pointer = %p\n", pDma->handle);
    printf("kernel_dma_buffer_adrs = %p\n", pDma->kernel_dma_buffer_adrs);
    printf("kernel_dma_buffer_size = %d\n", pDma->kernel_dma_buffer_size);
    printf("dma_buffer_bus_adrs = %p\n", pDma->dma_buffer_bus_adrs);

    close ( fd );

    if ( status == OK )
    {
        return OK;
    }
    else
    {
        DPIO2_ERR_MSG (("VMOSA_DPIO2_DMA_LOCK_MEMORY failed! status = %d, errno = %d\n",
		      status, errno ));
        return ERROR;
    }

}


/*
 * dpio2_copy_to_kernel
 *
 */
STATUS dpio2_copy_to_kernel (void* kernelAddress, void* userAddress, int iLength)
{

  STATUS                  status;
  int                     fd;
  dpio2_copy_from_user_t  copyValues;
  dpio2_copy_from_user_t* pCopyValues = &copyValues;

  fd = open ( DPIO2_DEVICE_MANAGER, O_RDONLY );
  
  if ( fd == ERROR )
  {
    DPIO2_ERR_MSG (("open %s failed! fd = %d, errno = %d\n",
		    DPIO2_DEVICE_MANAGER, fd, errno ));
    return ERROR;
  }
  pCopyValues->kernel_buffer_addr = kernelAddress;
  pCopyValues->user_buffer_addr = userAddress;
  pCopyValues->buffer_length = (unsigned long) iLength;

  status = ioctl (fd, VMOSA_DPIO2_DMA_COPY_BUFFER, pCopyValues);

  close (fd);

  if (status == OK)
  {
    return (OK);
  }
  else
  {
    DPIO2_ERR_MSG (("VMOSA_DPIO2_DMA_COPY_BUFFER failed! status = %d, errno = %d\n",
		    status, errno ));
    return ERROR;
  }
}


/**
 * vmosa_dpio2_dma_unlock_memory
 *
 * Unlock memory and free mapping vectors.
 *
 */
STATUS vmosa_dpio2_dma_unlock_memory ( dpio2_user_dma_page_t *pDma )

{

  STATUS   status;
  
  int  fd;



  fd = open ( DPIO2_DEVICE_MANAGER, O_RDONLY );
  
  if ( fd == ERROR ) {

    DPIO2_ERR_MSG (("open %s failed! fd = %d, errno = %d\n",
		    DPIO2_DEVICE_MANAGER, fd, errno ));
    return ERROR;

  }


  status = ioctl ( fd, VMOSA_DPIO2_DMA_UNLOCK_MEMORY, pDma );


  close ( fd );


  if ( status != OK ) {

    DPIO2_ERR_MSG (("VMOSA_DPIO2_DMA_UNLOCK_MEMORY failed! status = %d, errno = %d\n",
		    status, errno ));
    return ERROR;

  }


  return OK;

}


