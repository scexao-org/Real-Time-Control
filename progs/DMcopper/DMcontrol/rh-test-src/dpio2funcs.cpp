#include "dpio2funcs.h"

/*                                                                                           rsr
* Declare overloaded fuction prototype for dpio2Ioctl                                        rsr
*/                                                                                         //rsr
STATUS dpio2Ioctl(int devno, int command, void* argument);                                 //rsr


/* Globals */
static DPIO2_INFO_T  info;


void dmaDoneNotifier (sem_t*  pDmaDoneSemaphore)
{
  sem_post (pDmaDoneSemaphore);
}

STATUS scanForDevices (int* pNumDpio2Modules)
{
  int           numOfDpio2Modules;

  numOfDpio2Modules = dpio2Scan (&info);
  if (numOfDpio2Modules ==  ERROR) {
    printf ("Failed to scan for DPIO2 Devices\n");
    *pNumDpio2Modules = 0;
    return (ERROR);
  }

  if (numOfDpio2Modules < 1) {
    printf("Failed find any DPIO2 Modules\n");
    *pNumDpio2Modules = numOfDpio2Modules;
    return (ERROR);
  }

  *pNumDpio2Modules = numOfDpio2Modules;
  
  return (OK);
}

STATUS openDpio2ForOutput (int dpio2DeviceNumber)
{
  STATUS   status;

  status = dpio2Open(dpio2DeviceNumber, DPIO2_OUTPUT);
  if (status != OK) {
    printf("Failed to initialize the DPIO2 for output\n");
    return (ERROR);
  }

  return (OK);
}


STATUS configureDmaDoneInterrupt (int dpio2DeviceNumber, FUNCPTR pFunction, 
				  sem_t* pDmaDoneSemaphore)
{

  STATUS   status;
  DPIO2_INTERRUPT_CALLBACK   callbackSetting;

  callbackSetting.condition = DPIO2_INT_COND_DMA_DONE_CHAIN;
  callbackSetting.pCallbackFunction = pFunction;
  callbackSetting.argument = pDmaDoneSemaphore;

  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_INTERRUPT_CALLBACK_ATTACH, 
                       &callbackSetting);
  if (status != OK) {
    printf("Failed to attach callback action!\n");
    return (ERROR);
  }

  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_INTERRUPT_ENABLE, 
		       DPIO2_INT_COND_DMA_DONE_CHAIN);
  if (status != OK) {
    printf("Failed to enable interrupt!\n");
    return (ERROR);
  }
  
  return (OK);

}

STATUS enableStrobeGeneration (int dpio2DeviceNumber, unsigned int* pResultingFrequency, int Mode)
{

  STATUS   status;

  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_STROBE_FREQUENCY_SET, 
//		       (int) pResultingFrequency);                                         //rsr
                       pResultingFrequency);                                               //rsr
  if (status != OK) {
    printf("Failed to program %dHz as strobe frequency\n", 
           *pResultingFrequency);
    return (ERROR);
  }

  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_STROBE_GENERATION_ENABLE, 
		       Mode);
  if (status != OK) {
    printf("Failed to enable strobe generation\n");
    return (ERROR);
  } 

#if 0
  dpio2Ioctl (dpio2DeviceNumber, 
	      DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT,
	      FALSE);
#endif

  return (OK);

}

STATUS activateFpdpInterface (int dpio2DeviceNumber)
{

  STATUS   status;

  status = dpio2Ioctl (dpio2DeviceNumber, DPIO2_CMD_FPDP_ACTIVATION_SELECT, TRUE);
  if (status != OK) {
    printf ("Failed to activate the FPDP interface\n");
    return (ERROR);
  }
  
  return (OK);

}


STATUS allocateAndLockDmaBuffer (size_t lengthInBytes, void** buffer, DPIO2_DMA** ppPageInfo)
{
    STATUS      status;
/*
    Allocate structure to store information about the dma buffers
*/
    *ppPageInfo = (DPIO2_DMA*) malloc (sizeof(DPIO2_DMA));
    if (*ppPageInfo == NULL)
    {
        printf("Failed to allocate Page Information structure\n");
        return (ERROR);
    }
/*
    Lock the buffer into physical memory.
*/
    (*ppPageInfo)->dwBytes = lengthInBytes;

#ifdef RSR_DEBUG
    printf("Just before dpio2DMALock\n");
#endif

    status = dpio2DMALock (*ppPageInfo);
    if (status == OK)
    {
        *buffer = (*ppPageInfo)->pUserAddr;
        return (OK);
    }
    else
    {
        printf("Failed to allocate and lock buffer into physical memory\n");
        free (*ppPageInfo);
        return (ERROR);
    }
}


STATUS configureDmaChain(int dpio2DeviceNumber, DPIO2_DMA* pPageInfo, int desc_num)
{
  STATUS   status;
  UINT32   iPage = 0;
  DPIO2_DMA_DESC   dmaDescriptor;

#ifdef RSR_DEBUG
  printf("Size of &dmaDescriptor = %ld; &dmaDescriptor = %p\n", sizeof(&dmaDescriptor), &dmaDescriptor);
  printf("Entering configureDmaChain; device number = %d,",dpio2DeviceNumber);
  printf(" pPageInfo = %p, desc_num = %d\n", pPageInfo, desc_num);
#endif

  if (( pPageInfo->dwBytes & 0x00000007 ) != 0) {
    printf("Length must be multiple of 8 bytes\n");
    return (ERROR);
  }
#ifdef RSR_DEBUG
  printf("Length is okay.\n");
#endif

  if (( pPageInfo->dmaBusAddress & 0x00000007) != 0) {
    printf("PCI Address must be aligned to 8 bytes boundary\n");
    return (ERROR);
  }
#ifdef RSR_DEBUG
  printf("Alignment is okay.\n");
#endif
/*  Shouldn't be needed.
  for (iPage = 0; iPage < (pPageInfo->dwPages - 1); iPage++) {

    dmaDescriptor.descriptorId        = iPage + desc_num;
    dmaDescriptor.nextDescriptorId    = iPage + desc_num + 1;
    dmaDescriptor.pciAddress       
      = (UINT32) pPageInfo->dmaBusAddress;
    dmaDescriptor.blockSizeInBytes    
      = pPageInfo->dwBytes;
    dmaDescriptor.lastBlockInChain    = FALSE;
    dmaDescriptor.useD64              = FALSE;
    dmaDescriptor.notEndOfFrame       = FALSE;
    dmaDescriptor.endOfBlockInterrupt = FALSE;

    printf("In the for loop in configureDmaChain before call to dpio2Ioctl\n");  //rsr
    printf("iPage = %d and desc_num = \n",iPage, desc_num);                      //rsr

    status = dpio2Ioctl(dpio2DeviceNumber, 
                        DPIO2_CMD_DMA_SET_DESC, 
//                      (int) &dmaDescriptor);                                   //rsr
                        &dmaDescriptor);                                         //rsr
    if (status != OK) {
      printf("Failed to set DMA Descriptor\n");
      return (ERROR);
    }
  }

  printf("After the loop; iPage = %d\n", iPage);                                             //rsr
*/
  
  dmaDescriptor.descriptorId        = iPage + desc_num;
  dmaDescriptor.nextDescriptorId    = 0;
  dmaDescriptor.pciAddress          = pPageInfo->dmaBusAddress;

#ifdef RSR_DEBUG
  printf("pPageInfo->dmaBusAddress = %p\n", pPageInfo->dmaBusAddress);
  printf("dmaDescriptor.pciAddress = %p\n", dmaDescriptor.pciAddress);
#endif

  dmaDescriptor.blockSizeInBytes    
    = pPageInfo->dwBytes;
  dmaDescriptor.lastBlockInChain    = TRUE;
  dmaDescriptor.useD64              = FALSE;
  dmaDescriptor.notEndOfFrame       = FALSE;
  dmaDescriptor.endOfBlockInterrupt = TRUE;

#ifdef RSR_DEBUG
  printf("Before the call to dpioIoctl. dpio2DeviceNumber = %d\n",dpio2DeviceNumber);
  printf("DPIO2_CMD_DMA_SET_DESC = %d\n", DPIO2_CMD_DMA_SET_DESC);
  printf("dmaDescriptor.descriptorId = %d\n", dmaDescriptor.descriptorId);
#endif

  status = dpio2Ioctl(dpio2DeviceNumber, 
                      DPIO2_CMD_DMA_SET_DESC, 
                      &dmaDescriptor);                                                       //rsr

  if (status != OK)
  {
    printf("Failed to set DMA Descriptor\n");
    return (ERROR);
  }

#ifdef RSR_DEBUG
  printf("After the call to dpio2Ioctl from configureDmaChain\n");
#endif

  return (OK);
}


STATUS copyToKernel (void* kernelAddress, void* userAddress, int iLength)
{
  STATUS status;

  status = dpio2CopyToKernel (kernelAddress, userAddress, iLength);

  if (status == OK)
  {
//     printf ("User buffer copied to kernel\n");
     return (OK);
  }
  else
  {
     printf ("Failed to copy user buffer to kernel\n");
     return (ERROR);
  }
}


STATUS startDmaTransfer (int dpio2DeviceNumber, int desc_num)
{
  STATUS   status;

  status = dpio2Ioctl (dpio2DeviceNumber, DPIO2_CMD_DMA_SET_START_DESCRIPTOR, desc_num);
  if (status != OK) {
    printf("Failed to set start descriptor\n");
    return (ERROR);

  }

  status = dpio2Ioctl (dpio2DeviceNumber, DPIO2_CMD_DMA_START, 0);
  if (status != OK) {
    printf("Failed to start DMA transfer\n");
    return (ERROR);
  }

  return (OK);
}


void unlockAndFreeDmaBuffer (DPIO2_DMA* pPageInfo)
{
/*
**  Unlock the physical memory that the DMA buffer maps.
*/
    dpio2DMAUnlock (pPageInfo);
/*
**  Free the page information structure.
*/
    free (pPageInfo);
}


void closeDevice(int dpio2DeviceNumber)
{
  dpio2Close(dpio2DeviceNumber);
}

