/****************************************************************************
Module      : dpio2Vmosa.cpp

Description : Implementation of DLL functions for initializing DPIO2s in
              a Windows environment and doing cleanup when an application is
              finished using the DPIO2s.  In addition this file implements
              functions for handling interrupts from the DPIO2 modules.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01b,05nov2004,meh  Removed dpio2Create and dpio2Delete.
                     Added dpio2Scan
                     Moved functionality from dpio2Create and dpio2Delete
                     into dpio2Open and dpio2Close.
  01a,30jun2004,meh  Ported to Linux from dpio2Win.cpp

*****************************************************************************/

/* includes */
#include "vmosa.h"
#include "release.h"
#include "vmosaCInterruptManager.h"
#include "dpio2.h"
#include "dpio2Ioctl.h"
#include "string.h"


/* Defines */


enum {INVALID_RES = 0xffff};


typedef struct
{

  HANDLE   hDpio2;
  HANDLE   hStopEvent;
  HANDLE   hAckStopEvent;
  CInterruptManager*   pManager;

} DPIO2_INTERRUPT_ARGUMENT;


typedef enum {

  BIDIRECTIONAL,
  UNIDIRECTIONAL_INPUT,
  UNIDIRECTIONAL_OUTPUT

} DPIO2_DIRECTIONALITY;




/* Globals 
 */
int   numberOfDpio2Devices = 0;

int   dpio2ScanDone = 0;

int   deviceOpenFlag[MAX_DPIO2_DEVICE];

DPIO2_DEVICE  dpio2Device[MAX_DPIO2_DEVICE];

HANDLE        hVmosa[MAX_DPIO2_DEVICE];

DPIO2_INFO    dpio2Info[MAX_DPIO2_DEVICE];

static void dpio2DisplayModuleInfo(DPIO2_INFO* info);
static DPIO2_DIRECTIONALITY dpio2CheckDirectionality(CDpio2HwInterface* pInterface);

/*                                                                                           rsr
* Declare overloaded fuction prototype for dpio2Ioctl                                        rsr
*/                                                                                         //rsr
STATUS dpio2Ioctl(int devno, int command, void* argument);                                 //rsr




/****************************************************************************
* dpio2DisplayRelease -
*
* Prints release information about the DPIO2 driver
*
* RETURNS:  nothing
*/

static void dpio2DisplayRelease(void)
{
  printf ("DPIO2: DPIO2 Device Driver - Release: %s\n"
	  "       Copyright (C) 2005 - VMETRO, Inc.  All rights reserved.\n"
	  "       Creation Date: %s, Time: %s\n",
	  releaseAsText, __DATE__ , __TIME__ );
  return;
}




/******************************************************************************
*
* dpio2CheckDirectionality - check the directionality of a DPIO2 module 
*
* Checks whether a DPIO2 module is bi- or unidirectional. 
* If it is unidirectional, this function determines whether it is an
* input or an output module.
*
* RETURNS: BIDIRECTIONAL
*          UNIDIRECTIONAL_INPUT
*          UNIDIRECTIONAL_OUTPUT
*/

DPIO2_DIRECTIONALITY dpio2CheckDirectionality(CDpio2HwInterface* pInterface)
{

  CDpio2FrontEnd*   pFrontEnd;

  int   personalityModuleId;
  int   mainBoardId;
  int   moduleType;



  /* Get pointer to the FrontEnd object, and use it to read 
   * the IDs of the personality module and the main board.
   */
  pFrontEnd = pInterface->getFrontEnd();
  personalityModuleId = pFrontEnd->getPersonalityId();
  mainBoardId = pFrontEnd->getMainBoardId();
  moduleType = (mainBoardId << 4) | personalityModuleId;

  /* Determine whether the module type is one of the types known to the driver.
   */
  switch ( moduleType ) {

  case DPIO2_FB_MODULE:
  case DPIO2_LB_MODULE:
    return (BIDIRECTIONAL);
    
  case DPIO2_EI_MODULE:
  case DPIO2_DI_MODULE:
    return (UNIDIRECTIONAL_INPUT);

  case DPIO2_EO_MODULE:
  case DPIO2_DO_MODULE:
    return (UNIDIRECTIONAL_OUTPUT);

  default:
    DPIO2_ERROR_MSG (("Unknown module type\n"));
    throw;

  }

}




/******************************************************************************
* dpio2Isr - C Interrupt service routine of DPIO2 driver.
*
* Notifies the Interrupt Manager module in the DPIO2 core driver
* about the interrupt.
*
* RETURNS: OK if DPIO2 was interrupt source, ERROR if not.
*/

static STATUS dpio2Isr (void *arg)
{

  CInterruptManager *pManager = (CInterruptManager*) arg;

  return ( pManager->notifyAboutInterrupt() );

}






/******************************************************************************
* dpio2Scan - Scan PCI for all available DPIO2 Cards.
* 
* Scan the PCI bus for DPIO2 cards and returns HW info.
*
* RETURNS: number of dpio2 cards found or ERROR if not.
*/
int dpio2Scan ( DPIO2_INFO_T* pInfoArray )
{

  int   iDev;

  int   nCards;

  dpio2_hw_info_t  dpio2_hw_info[MAX_DPIO2_DEVICE];




  /* Display HW info and set all open flag to FREE
   */
  if ( !dpio2ScanDone ) {

    dpio2DisplayRelease();

    for ( iDev = 0; iDev <= MAX_DPIO2_CARDS; iDev++ ) {

      deviceOpenFlag [iDev] = DPIO2_DEVICE_FREE;

    }

  }


  if ( dpio2ScanDone ) {

    DPIO2_ERROR_MSG (("DPIO2 Devices is allready created!\n"));
    return ERROR;

  }


  /* Scan PCI for DPIO2 modules and returns number of DPIO2's 
   */
  nCards = dpio2_scan ( &dpio2_hw_info[0] );

  if ( nCards == ERROR ) {

    DPIO2_ERROR_MSG(("Failed to scan for DPIO2 Modules\n"));
    return ERROR;

  }


  if ( nCards == 0 ) {

    DPIO2_ERROR_MSG(("None DPIO2 Modules where found!\n"));
    return 0;
    
  }


  if ( nCards > MAX_DPIO2_CARDS ) {

    DPIO2_ERROR_MSG(("Found %d DPIO2 modules, but only %d can be used!\n", 
		     nCards, MAX_DPIO2_CARDS));
    nCards = MAX_DPIO2_CARDS;

  } else {
    
    DEBUG_MSG (("DPIO2: Scan PCI for DPIO2 modules. Found %d\n", nCards), TRUE);

  }


  /* Set the global numberOfDpio2Devices variable
   */
  numberOfDpio2Devices = nCards;


  /* Clear info data
   */
  bzero ( (char*) dpio2Info, (MAX_DPIO2_DEVICE) * sizeof (DPIO2_INFO) );


  /* Set user info pointers
   */ 
  for ( iDev = 1; iDev <= MAX_DPIO2_CARDS; iDev++ )

      pInfoArray->device[iDev-1] = &dpio2Info [iDev];



  /* Fill in information that are returned to the calling application.
   */
  for (iDev = 1; iDev <= numberOfDpio2Devices; iDev++) {

    dpio2Info [iDev].devno = dpio2_hw_info [iDev].devno;

    dpio2Info [iDev].deviceId = dpio2_hw_info [iDev].deviceId;

    dpio2Info [iDev].revisionId = dpio2_hw_info [iDev].revisionId;

    dpio2Info [iDev].pciBusNumber = dpio2_hw_info [iDev].pciBusNumber;

    dpio2Info [iDev].pciDeviceNumber = dpio2_hw_info [iDev].pciDeviceNumber;

    dpio2Info [iDev].moduleType = dpio2_hw_info [iDev].moduleType;

    dpio2Info [iDev].pciFpgaVersion = dpio2_hw_info [iDev].pciFpgaVersion;

    dpio2Info [iDev].frontEndFpgaVersion = dpio2_hw_info [iDev].frontEndFpgaVersion;

  }


  /* Display some information about each DPIO2 modules.
   */
  for (iDev = 1; iDev <= numberOfDpio2Devices; iDev++)

    dpio2DisplayModuleInfo( &dpio2Info [iDev] );


  /* Print proccess ID */
#if 0
  printf ("DPIO2: PID = %d\n", (int) getpid() );
#endif

  /* Set scan done */
  dpio2ScanDone = 1;

  
  /* Return number of dpio2 cards found
   */
  return nCards;

}








/******************************************************************************
* dpio2Open - Open and Initializes a DPIO2 Device
* 
* This function Open and Initializes the specified DPIO2 device to operate
* as either input or output.  
* The direction is specified by the mode parameter.
* 
* If the the device has been initialized previously, call the function dpio2Close
* to clear the device before reinitialize it.
*
*
* RETURNS: OK if successful, ERROR if not.
*/
STATUS dpio2Open(int devno, int mode)
{

  STATUS  status;

  DPIO2_DIRECTIONALITY   directionality;

  DPIO2_DEVICE*   pDevice;

  void  *arg;

#ifdef RSR_DEBUG
  printf("Entering the function dpio2Open in dpio2Vmosa.cpp\n");
#endif

  /* Check is dpio2Scan is called  */
  if ( !dpio2ScanDone ) {
    DPIO2_ERROR_MSG(("Call dpio2Scan before dpio2Open\n"));
    return (ERROR);
  }

  /* Check that the specified device number is valid */
  if ( (devno < 1) || (devno > numberOfDpio2Devices) ) {
    DPIO2_ERROR_MSG(("The specified device number (%d) is not valid\n", devno));
    return (ERROR);
  }

  /* Check if the DPIO2 device is already opened */
  if ( deviceOpenFlag [devno] == DPIO2_DEVICE_OPEN ) {
    DPIO2_ERROR_MSG(("The specified device number (%d) is in use by other user\n",
		     devno));
    return (ERROR);
  } else {
    /* Set device opened flag */
    deviceOpenFlag [devno] = DPIO2_DEVICE_OPEN;
  }

  /* Open vmosa dpio2 handle. 
     Initialize DPIO2 PCI interface and build the handle struct 
  */
  hVmosa [devno] =  dpio2_open (devno);
  
  if ( hVmosa[devno] == NULL ) {
    
    DPIO2_ERROR_MSG(("Failed to open vmosa handle!\n"));
    deviceOpenFlag [devno] = DPIO2_DEVICE_FREE;
    return ERROR;
    
  }

  /* Create HW interface
   */
  dpio2Device [devno].pHwInterface =  new CDpio2HwInterface ( hVmosa [devno] );
  if ( dpio2Device [devno].pHwInterface == NULL ) {
    DPIO2_ERROR_MSG(("Failed to create a DPIO2 HW interface object\n"));
    dpio2_close (devno);
    deviceOpenFlag [devno] = DPIO2_DEVICE_FREE;
    return (ERROR);
  } else 
    pDevice = &dpio2Device [devno];

  /* Initialize the DPIO2 module.
   */
  pDevice->pHwInterface->initialize();

  /* Determine the directionality of the DPIO2 module
   * and check that the specified mode flag is valid.
   * Configure the direction of the DPIO2 module.
   * This must be done before the driver controller is created,
   * because that object is direction dependent.
   */
  directionality = dpio2CheckDirectionality ( pDevice->pHwInterface );
  if ( directionality == BIDIRECTIONAL ) {

    if ( mode == DPIO2_INPUT ) {

      pDevice->pHwInterface->enableAsInput();

    } else if ( mode == DPIO2_OUTPUT ) {
    
      pDevice->pHwInterface->enableAsOutput();

    } else {

      DPIO2_ERROR_MSG(("Failed to initialize the specified DPIO2 device (%d)\n"
		       " - mode flag must be DPIO2_INPUT or DPIO2_OUTPUT\n", 
		       devno));
      
      return (ERROR);

    }

  } else if ( directionality == UNIDIRECTIONAL_INPUT ) {

    if ( mode == DPIO2_INPUT ) {

      pDevice->pHwInterface->enableAsInput();

    } else {
      
      DPIO2_ERROR_MSG(("Failed to initialize the specified DPIO2 device (%d)\n"
		       "mode flag must be DPIO2_INPUT since device is an unidirectional input\n", 
		       devno));
      dpio2_close (devno);
      deviceOpenFlag [devno] = DPIO2_DEVICE_FREE;
      return (ERROR);
      
    }

  } else {

    if ( mode == DPIO2_OUTPUT ) {

      pDevice->pHwInterface->enableAsOutput();

    } else {

      DPIO2_ERROR_MSG(("Failed to initialize the specified DPIO2 device (%d)\n"
		       "mode flag must be DPIO2_OUTPUT since device is an unidirectional output\n", 
		       devno));
      dpio2_close (devno);
      deviceOpenFlag [devno] = DPIO2_DEVICE_FREE;      
      return (ERROR);

    }

  }

  /* Create a driver controller for the device.
   */
  pDevice->pController = new CDpio2DriverController ( pDevice->pHwInterface );

  /* Create all interrupt actions.
   */
  if ( dpio2InitInterruptActions ( pDevice ) != OK ) {

    DPIO2_ERROR_MSG(("Failed to initialize interrupt actions!\n"));
    dpio2_close (devno);
    deviceOpenFlag [devno] = DPIO2_DEVICE_FREE;
    return (ERROR);

  }


  /* Prepare interrupt handling 
   */
  arg = (void*) dpio2Device [devno].pHwInterface->getInterruptManager();
  
  status = dpio2_prepare_interrupt_notification ( hVmosa [devno],
						  (FUNCPTR) dpio2Isr,
						  arg );

  if ( status == ERROR ) {
    
    DPIO2_ERROR_MSG(("Failed to prepare interrupt notificator!\n"));
    dpio2_close (devno);
    deviceOpenFlag [devno] = DPIO2_DEVICE_FREE;   
    return (ERROR);
    
  }


  /* Enable dpio2 interrupt routing to PCI
   */ 
  dpio2_enable_route_intr_pci ( hVmosa[devno] );

  /* Set device opened flag 
   */
  deviceOpenFlag[devno] = 1;

#ifdef RSR_DEBUG
  printf("Leaving dpio2Open\n");
#endif

  return (OK);

}







/******************************************************************************
* dpio2Close - Close DPIO2 Devices
* 
* Disable interrupt routing and transfers. 
* Delete created control objects.
*
*
* RETURNS: OK if successfull, ERROR if not.
* 
*/

STATUS dpio2Close ( int devno )
{


  /* Check that the specified device number is valid. 
   */
  if ( (devno < 1) || (devno > numberOfDpio2Devices) ) {

    DPIO2_ERROR_MSG(("The specified device number (%d) is not valid\n", devno));
    return (ERROR);

  }


  /* Check if the DPIO2 device is already opened 
   */
  if ( deviceOpenFlag [devno] == DPIO2_DEVICE_FREE ) {

    DPIO2_ERROR_MSG(("The specified device number (%d) is not open\n", devno));
    return (ERROR);
    
  }


  /* Disable dpio2 interrupt routing to PCI
   */ 
  dpio2_disable_route_intr_pci ( hVmosa [devno] );


  /* Remove interrupt notification 
   */
  dpio2_remove_interrupt_notification ( hVmosa [devno] );


  /* Delete the device if it has previously been initialized.
   * A device has been initialized, if a device control object is already created.
   */
  if ( dpio2Device [devno].pController != NULL ) {

    /* Make sure strobe generation is disabled */
    dpio2Device [devno].pController->disableStrobeGeneration();


    /* Make sure the FPDP interface is deactivated  */
    dpio2Device [devno].pController->stopTransfer();

    /* Delete the driver controller */
    delete dpio2Device [devno].pController;
    dpio2Device [devno].pController = NULL;
    
  }


  /* Delete HwInterface
   */
  if (dpio2Device [devno].pHwInterface) {
    
    delete dpio2Device [devno].pHwInterface;
    dpio2Device [devno].pHwInterface = NULL;

  }


  /* Close dpio2 vmosa handle 
   */
  dpio2_close (devno);


  /* Clear device opened flag 
   */
  deviceOpenFlag [devno] = DPIO2_DEVICE_FREE;
  

  return ( OK );

}




/******************************************************************************
*
* dpio2CopyToKernel - sends parameters to kernel needed for buffer copy
*
*
* RETURNS: OK if the transfer succeeds, ERROR if not.
*/
STATUS dpio2CopyToKernel (void* kernelAddress, void* userAddress, int iLength)
{
  return ( dpio2_copy_to_kernel (kernelAddress, userAddress, iLength) );
}





/******************************************************************************
*
* dpio2Ioctl - sends commands to a DPIO2 Device
*              in cases where the command requires an integer argument                                        //rsr
*
*
* RETURNS: OK if the specified command succeeds, ERROR if not.
*/
STATUS dpio2Ioctl ( int devno, int command, int argument )
{

#ifdef RSR_DEBUG
  printf("In dpio2Ioctl - dpio2Vmosa (int) version. devno = %d", devno);                                      //rsr
  printf(" command = %d, argument = %d\n", command, argument);                                                //rsr
#endif

  /* Check that the specified device number is valid.
   */
   if ( (devno < 0) || (devno > numberOfDpio2Devices) ) {

    DPIO2_ERROR_MSG(("The specified device number (%d) is not valid\n", devno));
    return (ERROR);

  }


  /* Check if the DPIO2 device is opened 
   */
  if ( !deviceOpenFlag[devno] ) {

    DPIO2_ERROR_MSG(("The specified device number (%d) is not valid\n", devno));
    return (ERROR);
    
  }

#ifdef RSR_DEBUG
  printf("Leaving dpio2Ioctl - dpio2Vmosa (int) version.  Calling dpio2Ioctl - dpio2Ioctl version.\n");       //rsr
#endif

  return (   dpio2Ioctl ( &dpio2Device[devno], command, argument ) );

}





/******************************************************************************
*
* dpio2Ioctl - sends commands to a DPIO2 Device
*              in cases where the command requires a pointer, which is                                         //rsr
*              passed as an unsigned long integer (a holdover from C)                                          //rsr
*
*
* RETURNS: OK if the specified command succeeds, ERROR if not.
*/
STATUS dpio2Ioctl ( int devno, int command, void* argument )
{

#ifdef RSR_DEBUG
  printf("In dpio2Ioctl - dpio2Vmosa (void*) version. devno = %d", devno);
  printf(" command = %d, argument = %p\n", command, argument);
#endif

  /* Check that the specified device number is valid.
   */
   if ( (devno < 0) || (devno > numberOfDpio2Devices) ) {

    DPIO2_ERROR_MSG(("The specified device number (%d) is not valid\n", devno));
    return (ERROR);

  }


  /* Check if the DPIO2 device is opened 
   */
  if ( !deviceOpenFlag[devno] ) {

    DPIO2_ERROR_MSG(("The specified device number (%d) is not valid\n", devno));
    return (ERROR);
    
  }

#ifdef RSR_DEBUG
  printf("Leaving dpio2Ioctl - dpio2Vmosa (void*) version.\n");  
  printf("Calling dpio2Ioctl - dpio2Ioctl version.\n");
#endif

  return (   dpio2Ioctl ( &dpio2Device[devno], command, argument ) );

}





/* Determines personality, HW revision, and FPGA versions,
 * and displays this information.
 */
static void dpio2DisplayModuleInfo ( DPIO2_INFO *info )
{

  const char*   pModuleTypeName;
  
  char    name[10];



  /* Determine whether the module type is one of the types known to the driver.
   */
  switch ( info->moduleType ) {

  case DPIO2_FB_MODULE:
    pModuleTypeName = "FB";
    break;

  case DPIO2_LB_MODULE:
    pModuleTypeName = "LB";
    break;

  case DPIO2_EI_MODULE:
    pModuleTypeName = "EI";
    break;

  case DPIO2_EO_MODULE:
    pModuleTypeName = "EO";
    break;

  case DPIO2_DI_MODULE:
    pModuleTypeName = "DI";
    break;

  case DPIO2_DO_MODULE:
    pModuleTypeName = "DO";
    break;

  default:
    pModuleTypeName = "Unknown";
    break;

  }


  /* Determine which version of the FPGA code the DPIO2 is running.
   */
  if ( info->deviceId == PCI_DEVICE_ID_DPIO2_66MHZ ) {

    sprintf ( name, "%s", "DPIO2_66" );

  } else {

    sprintf ( name, "%s", "DPIO2" );

  }

  
  DEBUG_MSG(("DPIO2: Device #%d (bus %d, device %d) is a %s-%s with FPGA code %c%02d%02d\n",
             info->devno, info->pciBusNumber, info->pciDeviceNumber, 
	     name, pModuleTypeName,('A' + info->revisionId), 
	     info->frontEndFpgaVersion, info->pciFpgaVersion), TRUE);
 
}




/******************************************************************************
*
* dpio2DMALock - Lock a linear memory region, and return the 
*                corresponding physical address
*
* INCLUDE FILES: dpio2.h
*
* RETURNS: OK if memory is locked, ERROR if not.
*/
STATUS dpio2DMALock ( DPIO2_DMA* pDma )
{
    STATUS                  status;
    dpio2_user_dma_page_t   kioDma  = {NULL, NULL, 0, 0};
    dpio2_user_dma_page_t*  pKioDma = &kioDma;
/*
**  Check input param
*/
    if ( pDma == NULL )
    {
        DPIO2_ERROR_MSG (("NULL pionter!\n"));
        return ERROR;
    }
/*
**  Copy the buffer size input parameter
*/
    pKioDma->kernel_dma_buffer_size = pDma->dwBytes;

/*
**  Lock memory
*/
    status = vmosa_dpio2_dma_lock_memory ( pKioDma );

    if ( status == OK )
    {
/*
**      Copy output parameters.
*/
        pDma->hDma = (device*) pKioDma->handle;
        pDma->pUserAddr = pKioDma->kernel_dma_buffer_adrs;
        pDma->dmaBusAddress = pKioDma->dma_buffer_bus_adrs;

        return OK;
    }
    else
    {
        DPIO2_ERROR_MSG (("Failed to lock 0x%x bytes\n", pDma->dwBytes));

        return ERROR;
    }
}





/******************************************************************************
*
**  dpio2DMAUnlock - Unock a DMA buffer
*
**  INCLUDE FILES: dpio2.h
*
**  RETURN: None.
*/
void dpio2DMAUnlock ( DPIO2_DMA *pDma )
{
    STATUS  status;
    dpio2_user_dma_page_t   kioDma  = {NULL, NULL, 0, 0};
    dpio2_user_dma_page_t*  pKioDma = &kioDma;
/*
**  Copy input param
*/
    pKioDma->handle = pDma->hDma;
    pKioDma->kernel_dma_buffer_adrs = pDma->pUserAddr;
    pKioDma->kernel_dma_buffer_size = pDma->dwBytes;
    pKioDma->dma_buffer_bus_adrs = pDma->dmaBusAddress;
/*
**  Unlock memory
*/
    status = vmosa_dpio2_dma_unlock_memory ( pKioDma );

    if ( status != OK )
    {
        DPIO2_ERROR_MSG (("Failed to unlock 0x%x bytes starting at %p\n", 
		          pDma->dwBytes, pDma->pUserAddr));
    }

    return;
}

