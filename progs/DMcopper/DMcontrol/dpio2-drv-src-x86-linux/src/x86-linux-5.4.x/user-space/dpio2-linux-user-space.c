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

#if 0
#include "signal.h"     /* SIGTERM */
#endif
#if 1
#include <signal.h>
#endif



/* Globals 
 */
static int number_of_dpio2_cards = 0;
static int dpio2_scan_done_flag = 0;

static int device_open_flag[MAX_DPIO2_DEVICE];

static HANDLE a_handle[MAX_DPIO2_DEVICE];





/* Functions
 */
static STATUS dpio2_give_isr_semaphore ( HANDLE handle );
static STATUS dpio2_interrupt_notify_thread (  HANDLE  handle );
static void   dpio2_exit_signal (int signo );



#if 0
void busmarkn ( int devno, UINT32 dword )
{

  /* Write to register
   */
  *( (UINT32*) ( a_handle[devno]->user.ctrl_base +
		 DPIO2_GENERAL_PURPOSE_REGISTER ) ) = dword;

#ifndef NO_READ_AFTER_WRITE
  /* Read the same register to be sure that the write operation has been done
   */
  *( (UINT32*) ( a_handle[devno]->user.ctrl_base + 
		 DPIO2_GENERAL_PURPOSE_REGISTER ) );
#endif

}


void busmark ( HANDLE handle, UINT32 dword )
{

  /* Write to register
   */
  *( (UINT32*) ( handle->user.ctrl_base +
		 DPIO2_GENERAL_PURPOSE_REGISTER ) ) = dword;

#ifndef NO_READ_AFTER_WRITE
  /* Read the same register to be sure that the write operation has been done
   */
  *( (UINT32*) ( handle->user.ctrl_base + 
		 DPIO2_GENERAL_PURPOSE_REGISTER ) );
#endif

}
#endif








/**
 * dpio2_scan
 *
 * Scan for dpio2 cards and initialize PCI interface.
 * 
 * Do:
 * (1) Open /dev/dpio2/manager
 * (2) Ask for number of DPIO2 cards
 * (3) Close /dev/dpio2/manager
 * (4) Clear all device flag to "closed"
 *
 *
 * @return Number of DPIO2 cards or ERROR
 */
int dpio2_scan (dpio2_hw_info_t* p_dpio2_hw_info)
{
  
  int     fd;

  int     iDev;




 /* Clear all open flags
   */
  if ( !dpio2_scan_done_flag )
    
    for ( iDev = 1; iDev <= MAX_DPIO2_CARDS; iDev++ )
      
      device_open_flag [iDev] = DPIO2_DEVICE_FREE;


  /* Open manager
   */
  fd = open ( DPIO2_DEVICE_MANAGER, O_RDONLY );

  if ( fd == ERROR ) {

    DPIO2_ERR_MSG (("open %s failed! errno = %d\n",
		    DPIO2_DEVICE_MANAGER, errno ));
    return ERROR;

  }


  /* Get number of DPIO2 cards and HW info
   */
  number_of_dpio2_cards = ioctl ( fd, DPIO2_SCAN, p_dpio2_hw_info );

  if ( number_of_dpio2_cards < 0 ) {

    DPIO2_ERR_MSG (("DPIO2_SCAN failed! status = %d, errno = %d\n", 
		    number_of_dpio2_cards, errno ));
    close ( fd );
    return ERROR;

  }

  
  close ( fd );


  /* Set scan done flag
   */
  dpio2_scan_done_flag = 1;


  /* Return number of DPIO2 cards
   */
  return ( number_of_dpio2_cards );

}





/**
 * dpio2_open
 *
 * Open kernel device. Map kernel and user space addres. 
 * 
 * Do:
 * (1) Allocate and clear handle struct
 * (2) Open device and prepare kernel resources
 * (3) Map PCI control and fifo base
 *
 * @param devno     device number
 * @param handle    pointer handle
 *
 * @return HANDLE or NULL
 */
HANDLE dpio2_open ( int devno )
{

  STATUS  status;

  HANDLE  handle;
  
  char    device_name[30];



  /* Check if device number exist 
   */
  if ( ( devno < 1 ) || ( devno > number_of_dpio2_cards ) ) {

    DPIO2_ERR_MSG (("The specified device number (%d) is not valid\n", devno));
    return NULL;

  }


  /* Check if device is opened
   */
  if ( device_open_flag [devno] == DPIO2_DEVICE_OPEN ) {
    
    DPIO2_ERR_MSG (("The DPIO2 device number %d is allready opened\n", devno));
    return NULL;
    
  } else {

    device_open_flag [devno] = DPIO2_DEVICE_OPEN;

  }


  /* Allocate handle struct 
   */
  handle = (HANDLE) malloc (  sizeof (dpio2_handle_t) );

  if ( handle == 0 ) {

    DPIO2_ERR_MSG (("malloc failed\n"));
    return NULL;

  }


  /* Clear hadle struct
   */
  memset ( (char*) handle, 0, sizeof ( dpio2_handle_t ) );


  /* Open kernel device:
     - Map kernel PCI addres
     - Allocate kiovec
     - initalize semaphore
   */
  sprintf ( device_name, "%s%d", DPIO2_DEVICE_PATH_NAME, devno ); 
  

  handle->fd = open ( device_name, O_RDWR );

  if ( handle->fd < 0 ) {
    
    DPIO2_ERR_MSG (("Open %s failed! fd = %d, errno = %d\n",
		    device_name, handle->fd, errno ));
    free (handle);
    return NULL;
    
  } 
  

  /* Get PCI configuaration info like : vendor, device and revision id, 
     pci bus and device number 
  */
  status = ioctl ( handle->fd, DPIO2_GET_PCI_CONFIG_INFO, &handle->pci );

  if ( status != OK ) {
    
    DPIO2_ERR_MSG (("DPIO2_GET_PCI_CONFIG_INFO failed! status = %d, errno =%d\n",
		    status, errno));
    close ( handle->fd );
    free (handle);
    return NULL;
    
  } 


  /* Map PCI resoures and addreses 
   */
  handle->user.ctrl_mmap_size = DPIO2_PCI_CTRL_WINDOW_SIZE;
  handle->user.fifo_mmap_size = DPIO2_PCI_FIFO_WINDOW_SIZE;


  status = ioctl ( handle->fd, DPIO2_MMAP_SELECT, DPIO2_MMAP_CTRL_BASE);

  if ( status != OK ) {
   
    DPIO2_ERR_MSG (("DPIO2_MMAP_SELECT failed, errno = %d\n", errno ));
    close ( handle->fd );
    free (handle);
    return NULL;
    
  }

  handle->user.ctrl_base =  mmap ( 0, DPIO2_PCI_CTRL_WINDOW_SIZE,
				   PROT_READ | PROT_WRITE, 
				   MAP_SHARED,
				   handle->fd, 0 );
  
  if (((unsigned long) handle->user.ctrl_base)== ERROR ) {
    
    DPIO2_ERR_MSG (("mmap failed, errno = %d\n", errno ));
    close ( handle->fd );
    free (handle);
    return NULL;
    
  } 


  status = ioctl ( handle->fd, DPIO2_MMAP_SELECT, DPIO2_MMAP_FIFO_BASE );

  if ( status != OK ) {

    DPIO2_ERR_MSG (("DPIO2_MMAP_SELECT failed, errno = %d\n", errno ));
    munmap ( handle->user.ctrl_base, DPIO2_PCI_CTRL_WINDOW_SIZE );
    close ( handle->fd );
    free (handle);
    return NULL;
    
  } 


  handle->user.fifo_base = mmap ( 0, DPIO2_PCI_FIFO_WINDOW_SIZE,
				  PROT_READ | PROT_WRITE, 
				  MAP_SHARED,
				  handle->fd, 0 );
  
  if ( (unsigned long) handle->user.fifo_base == ERROR ) {
    
    DPIO2_ERR_MSG (("mmap failed, errno = %d\n", errno ));
    munmap ( handle->user.ctrl_base, DPIO2_PCI_CTRL_WINDOW_SIZE );
    close ( handle->fd );
    free (handle);
    return NULL;

  } 


  /* Set devno
   */
  handle->devno = devno;


  /* Set handle pointer
   */
  a_handle[devno] = handle;


  /* Mark the device as open
   */
  device_open_flag [devno] = DPIO2_DEVICE_OPEN;


  /* Return HANDLE
   */
  return (handle);

}



/**
 * dpio2_close
 *
 * Open kernel device. Map kernel and user space addres. 
 * 
 * Do:
 * (1) Unmap PCI control and fifo base
 * (2) Close /dev/dpio2/devno device
 * (3) Deallocate handle struct
 *
 * @param devno     device number
 *
 * @return OK or ERROR
 */
STATUS dpio2_close (int  devno)
{

  HANDLE  handle;
  


  /* Check if device number exist 
   */
  if ( ( devno < 1 ) || ( devno > number_of_dpio2_cards ) ) {
    
    DPIO2_ERR_MSG (("The specified device number %d is not valid\n", devno));
    return ERROR;
    
  }


  /* Check if device is opened
   */
  if ( device_open_flag [devno] != DPIO2_DEVICE_OPEN ) {

    DPIO2_ERR_MSG (("The DPIO2 device number %d is not open\n", devno));
    return ERROR;
    
  }


  /* Find handle
   */
  if ( a_handle[devno] == NULL ) {

    DPIO2_ERR_MSG (("NULL pointer for Device #%d\n", devno));
    return ERROR;

  }


  handle = a_handle[devno];


  /* Unmap PCI resoures and addreses
   */
  if ( handle->user.ctrl_base ) {
    
    munmap ( handle->user.ctrl_base, DPIO2_PCI_CTRL_WINDOW_SIZE );
    handle->user.ctrl_base = 0;

  }
  
  if ( handle->user.fifo_base ) {

    
    munmap ( handle->user.fifo_base, DPIO2_PCI_FIFO_WINDOW_SIZE );
    handle->user.fifo_base = 0;

  }


  /* Close device
   */
  close (handle->fd);


  /* Mark device as closed
   */
  device_open_flag [devno] = DPIO2_DEVICE_FREE;

  
  /* Free handle struct 
   */
  free ( a_handle[devno] );


  a_handle[devno] = 0;   


  return OK;

}




/**
 * dpio2_prepare_interrupt_notification
 *
 */
STATUS dpio2_prepare_interrupt_notification ( HANDLE  handle, 
					      FUNCPTR dpio2_notify_function,
					      void*   arg )
{

  STATUS  status;


  /* Chech input param
   */ 
  if ( dpio2_notify_function == NULL ) {

    DPIO2_ERR_MSG (("pointer to isr notify routine is NULL\n"));
    return ERROR;

  }

  
  /* Disable interrupt route to pci 
   */
  dpio2_disable_route_intr_pci (handle);
  

  /* Connect interrupt to pthread
   */
  status = ioctl ( handle->fd, DPIO2_INTERRUPT_CONNECT, 0 );

  if ( status != OK ) {

    DPIO2_ERR_MSG (("Connect interrupt to pthread failed! status = %d, errno = %d\n",
		    status, errno ));
    return ERROR;

  }


  /* Set callback info
   */
  handle->callback.func_ptr = (FUNCPTR) dpio2_notify_function;
  handle->callback.arg = arg;



/* Need to be super-user to deal with scheduler and priority */
#ifdef SET_SCHEDULER_AND_PRIORITY

  /* Set thread attrubutes
   */
  status = pthread_attr_init ( &handle->isr.attribute );

  if ( status != OK ) {

    DPIO2_ERR_MSG (("pthread_attr_init failed, status = %d, errno %d\n", 
		    status, errno ));
    return ERROR;

  }


  status = pthread_attr_setschedpolicy ( &handle->isr.attribute, SCHED_RR );

  if ( status != OK ) {
    
    DPIO2_ERR_MSG (("pthread_attr_setschedpolicy failed,"
		    "status = %d, errno %d\n",
		    status, errno ));    
  }
  
  handle->isr.pri.__sched_priority = 3;

  status =  pthread_attr_setschedparam (  &handle->isr.attribute, 
					  &handle->isr.pri );

  if ( status != OK ) {
    
    DPIO2_ERR_MSG (("pthread_attr_setschedparam failed, status = %d, errno %d\n",
		    status, errno ));
    
  }

  
  /* Start interrupt notifier pthread
   */
  status = pthread_create ( &handle->isr.pthread, 
                            &handle->isr.attribute, /*NULL*/
			    (void*(*)(void*)) dpio2_interrupt_notify_thread,
			    (void*) handle);

  if ( status != OK ) {

    DPIO2_ERR_MSG (("pthread_create failed\n"));
    return ERROR;

  }


#else

  /* Start interrupt notifier pthread
   */
  status = pthread_create ( &handle->isr.pthread, 
                            NULL,
			    (void*(*)(void*)) dpio2_interrupt_notify_thread,
			    (void*) handle);

  if ( status != OK ) {

    DPIO2_ERR_MSG (("pthread_create failed\n"));
    return ERROR;

  }

#endif


  return OK;

}




STATUS dpio2_remove_interrupt_notification ( HANDLE  handle )
{

  STATUS  status;



  /* Disable interrupt route to pci 
   */
  dpio2_disable_route_intr_pci (handle);


  /* Disconnect interrupt to pthread
   */
  status = ioctl ( handle->fd, DPIO2_INTERRUPT_DISCONNECT, 0 );

  if ( status != OK ) {

    DPIO2_ERR_MSG (("Disconnect interrupt to pthread failed! status = %d, errno = %d\n",
		    status, errno ));
    return ERROR;

  }


  /* Stop interrupt pthread 
   */
  handle->isr.exit_flag = 1;

  status = dpio2_give_isr_semaphore (handle);


  /* Give away the CPU, sleep
   */ 
  vmosa_mdelay (100);


  /* Check if interrupt notifier pthread is still running after 100 mili seconds
   */
  status = pthread_kill ( handle->isr.pthread, 0 );

  if ( status == OK ) {

    vmosa_mdelay (100);

    /* Check if interrupt notifier pthread is still running after 200 mili seconds
     */
    status = pthread_kill ( handle->isr.pthread, 0 );

    if ( status == OK ) {


      /* Kill the thread 
       */
      status = pthread_kill ( handle->isr.pthread, SIGTERM );

      if ( status != OK ) {
      
	DPIO2_ERR_MSG (("pthread_kill failed\n"));
	return ERROR;
    
      }

    }

  }


  return OK;
}





/**
 * dpio2_disable_route_intr_pci
 *
 * Disable routing of interrupt from dpio2 to PCI
 *
 * @param handle      handle
 *
 * @return OK
 */
STATUS dpio2_disable_route_intr_pci ( HANDLE  handle )
{ 


  if ( handle->user.ctrl_base == NULL ) {

    DPIO2_ERR_MSG (("NULL pointer\n"));
    return ERROR;

  }


  /* Disable interrupt route to pci 
   */
  vmosa_reg_32_write ( handle, DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER, 1 );


  return OK;

}



/**
 * dpio2_enable_route_intr_pci
 *
 * Enable routing of interrupt from dpio2 to PCI
 *
 * @param handle      handle
 *
 * @return OK
 */
STATUS dpio2_enable_route_intr_pci ( HANDLE  handle )
{  


  if ( handle->user.ctrl_base == NULL ) {

    DPIO2_ERR_MSG (("NULL pointer\n"));
    return ERROR;

  }


  /* Enable interrupt route to pci 
   */
  vmosa_reg_32_write ( handle, DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER, 0 );


  return OK;

}




/**
 * dpio2_disable_route_intr_pci_save_atomic
 *
 * Disable routing of interrupt from dpio2 to PCI.
 * Saving the dpio2 interrupt route status
 *
 * @param handle             handle
 * @param save_route_status  save route status
 *
 * @return OK
 */
STATUS dpio2_disable_route_intr_pci_save_atomic ( HANDLE  handle, 
							BOOL    *save_route_status )
{ 

  STATUS   status;


  
  status = ioctl ( handle->fd, DPIO2_DISABLE_ROUTE_INTR_PCI_SAVE_ATOMIC, 0 );

  if ( status == 0 ) 

    *save_route_status = 0;

  else if ( status == 1 )

    *save_route_status = 1;

  else /* error */

    return ( status );


  return OK;

}




/**
 * dpio2_enable_route_intr_pci_save
 *
 * Ensable routing of interrupt from dpio2 to PCI
 * if the saved dpio2 interrupt route status is 0
 *
 * @param handle             handle
 * @param save_route_status  save route status
 *
 * @return OK
 */
STATUS dpio2_enable_route_intr_pci_save ( HANDLE  handle, 
					  BOOL    saved_route_status)
{  

  if (!saved_route_status) {

    /* Enable interrupt route to pci
     */
    dpio2_enable_route_intr_pci (handle) ;    

  }

  return OK;

}





/**
 * dpio2_give_isr_semaphore
 *
 * Used to up kernel semaphore for stoping the interrupt handle thread.
 *
 */ 
static STATUS dpio2_give_isr_semaphore ( HANDLE  handle )
{

  return ( ioctl ( handle->fd, DPIO2_GIVE_ISR_SEMAPHORE, 0 ) );

}




/**
 * dpio2_interrupt_notify_thread
 *
 * Interrupt handler for dpio2 devices
 * 
 */
static STATUS dpio2_interrupt_notify_thread (  HANDLE  handle )
{

  STATUS      status; 

  struct sigaction sa;
  sigset_t         sa_mask;


#if 0
  printf ("DPIO2: Device #%d Interrupt notifier pthread's PID = %d\n",
	  handle->devno, (int) getpid() );
#endif

  /* Connect handler if the thread is signaled
   */
  memset ( &sa, 0, sizeof ( struct sigaction ) );
  sigemptyset ( &sa_mask );
  sa.sa_flags     = 0;  /* SA_SIGINFO */
  sa.sa_mask      = sa_mask;
  sa.sa_handler   = &dpio2_exit_signal;

  sigaction (SIGTERM, &sa, NULL);



  while ( 1 ) {
    
    /* Wait for interrupt from dpio2
     */
    status = ioctl ( handle->fd, DPIO2_WAIT_FOR_INTERRUPT, 0 );

    if ( status != OK ) {

      DPIO2_ERR_MSG (("DPIO2_WAIT_FOR_INTERRUPT failed, status = %d, errno = %d\n",
		      status, errno ));
      break;

    } 

    
    /* Check for exit flag
     */  
    if ( handle->isr.exit_flag ) {

      break;

    }


    /* Check for NULL pointer 
     */
    if ( handle->callback.func_ptr ) {

      /* Call interrupt handler
       */
      handle->callback.func_ptr ( handle->callback.arg );

    }


    /* Enable interrupt is moved to kernel ioctl function 
       DPIO2_WAIT_FOR_INTERRUPT */

  }

#if 0
  printf ("DPIO2: Device #%d Interrupt notifier thread exit\n",
	  handle->devno);
#endif
      
  return OK;

}



void dpio2_exit_signal ( int signo )
{

  printf ("dpio2_exit_signal, signo = %d\n", signo);

  pthread_exit (NULL);

}






/**
 * set_intel_31154_p2p_prefetch_policy
 *
 * Set prefetch policy and cacheline size on a intel 31154 P2P
 *
 */
int set_intel_31154_p2p_prefetch_policy ( int prefetch_policy,
					  int cacheline_size )
{

  STATUS status;

  int    fd;

  UINT32 val32 = 0;


  
  fd = open ( DPIO2_DEVICE_MANAGER, O_RDONLY );

  if ( fd == ERROR ) {

    DPIO2_ERR_MSG (("open %s failed! errno = %d\n",
		    DPIO2_DEVICE_MANAGER, errno ));
    return ERROR;
    
  }

  val32 = cacheline_size;

  val32 <<= 16;

  val32 |= prefetch_policy;


  status = ioctl ( fd, SET_INTEL_31154_P2P_PREFETCH_POLICY, val32 );

  close ( fd );
  
  if ( status == ERROR ) {
    
    DPIO2_ERR_MSG (("SET_INTEL_31154_P2P_PREFETCH_POLICY failed! "
		    "status = %d, errno = %d\n", 
		    status, errno ));
    return ERROR;
    
  }


  return status;

}
