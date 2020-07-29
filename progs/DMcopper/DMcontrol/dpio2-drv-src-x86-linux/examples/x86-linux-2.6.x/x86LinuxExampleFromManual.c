/********************r********************************************************

Module      : example.c

Copyright(c): 2005 VMETRO asa. All Rights Reserved.

Revision History:
  01a,01nov2005,meh  created

*****************************************************************************/

/* Includes */
#include "stdio.h"      /* printf, ... */
#include "stdlib.h"     /* malloc,.. */
#include "asm/page.h"   /* PAGE_SIZE */
#include "pthread.h"    /* pthread_attr_init ,pthread_create, .... */ 
#include "time.h"       /* nanosleep */
#include "unistd.h"     /* sleep, close, ... */
#include <malloc.h>     /* memalign */
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

#include "vmosa.h"
#include "dpio2.h"




/* Globals */
static DPIO2_INFO_T  info;




/* Functions */

void dmaDoneNotifier (sem_t*  pDmaDoneSemaphore)
{
  sem_post ((sem_t*)pDmaDoneSemaphore);
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


STATUS openDpio2ForInput (int dpio2DeviceNumber)
{
  STATUS   status;

  status = dpio2Open (dpio2DeviceNumber, DPIO2_INPUT);
  if (status != OK) {
    printf("Failed to initialize the DPIO2 for input\n");
    return (ERROR);
  }

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
				  sem_t *pDmaDoneSemaphore)
{

  STATUS   status;
  DPIO2_INTERRUPT_CALLBACK   callbackSetting;

  callbackSetting.condition = DPIO2_INT_COND_DMA_DONE_CHAIN;
  callbackSetting.pCallbackFunction = pFunction;
  callbackSetting.argument = pDmaDoneSemaphore;                                            //rsr removed (int)

  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_INTERRUPT_CALLBACK_ATTACH, 
//		       (int) &callbackSetting);
                       (unsigned long) &callbackSetting);                                  //rsr
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


STATUS enableStrobeGeneration (int dpio2DeviceNumber, UINT32 wantedFrequency)
{

  STATUS   status;
  UINT32   resultingFrequency;

  resultingFrequency = wantedFrequency;
  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_STROBE_FREQUENCY_SET, 
//		       (int) &resultingFrequency);
                       (unsigned long) &resultingFrequency);                               //rsr
  if (status != OK) {
    printf("Failed to program %dHz as strobe frequency\n", 
           (int) wantedFrequency);
    return (ERROR);
  } else {
    printf("Strobe frequency programmed to %dMHz\n", 
           (int)resultingFrequency/1000000);
  }

  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_STROBE_GENERATION_ENABLE, 
		       DPIO2_PROGRAMMABLE_OSCILLATOR);
  if (status != OK) {
    printf("Failed to enable strobe generation\n");
    return (ERROR);
  } 

  dpio2Ioctl (dpio2DeviceNumber, 
	      DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT,
	      TRUE);

  return (OK);

}


STATUS enableStrobeReception (int dpio2DeviceNumber, UINT32 strobeFrequency)
{
  STATUS   status;
  int   range;

  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_STROBE_RECEPTION_ENABLE, 
		       DPIO2_PRIMARY_STROBE);
  if (status != OK) {
    printf("Failed to enable strobe reception\n");
    return (ERROR);
  } 

  /* Determine which of the predefined ranges,
   * the specfied strobe frequency falls within.
   */
  if (strobeFrequency <= (5*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_BELOW_OR_EQUAL_TO_5MHZ;  
  } else  if (strobeFrequency <= (10*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ;
  } else  if (strobeFrequency <= (15*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ;
  } else  if (strobeFrequency <= (20*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ;
  } else  if (strobeFrequency <= (25*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ;
  } else  if (strobeFrequency <= (30*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ;
  } else  if (strobeFrequency <= (35*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ;
  } else  if (strobeFrequency <= (40*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ;
  } else  if (strobeFrequency <= (45*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ;
  } else  if (strobeFrequency <= (50*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ;
  } else  if (strobeFrequency <= (55*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ;
  } else  if (strobeFrequency <= (60*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ;
  } else  if (strobeFrequency <= (65*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ;
  } else  if (strobeFrequency <= (70*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ;
  } else  if (strobeFrequency <= (75*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ;
  } else  if (strobeFrequency <= (80*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ;
  } else  if (strobeFrequency <= (85*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ;
  } else  if (strobeFrequency <= (90*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ;
  } else  if (strobeFrequency <= (95*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ;
  } else  if (strobeFrequency <= (100*1000*1000)) {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ;
  } else {
    range = DPIO2_STROBE_FREQUENCY_ABOVE_100MHZ;
  }


  status = dpio2Ioctl (dpio2DeviceNumber, 
		       DPIO2_CMD_STROBE_FREQUENCY_RANGE_SET, 
		       range);
  if (status != OK) {
    printf("Failed to set %d as strobe frequency range\n", range);
    return (ERROR);
  }

  dpio2Ioctl (dpio2DeviceNumber, 
	      DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT,
	      TRUE);

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



//STATUS allocateAndLockBuffer (UINT32 lengthInBytes, UINT32 **buffer,                     //rsr
STATUS allocateAndLockBuffer (size_t lengthInBytes, unsigned long** buffer,                //rsr
			      DPIO2_DMA** ppPageInfo)

{
  STATUS   status;
  int      maxNumPages;

  /* Allocate aligned memory buffer */
//*buffer = memalign (0x100, (size_t)lengthInBytes);                                       //rsr
  *buffer = memalign (0x100, lengthInBytes);                                               //rsr
  if (*buffer == NULL) 
  {
    printf("Failed to allocate %ld bytes\n", lengthInBytes);                               //rsr
    return (ERROR);                                                                        //rsr
  }


  /* Determine the maximum number of pages the buffer can be mapped to.
   * One is added at the end of the computation to account for the fact
   * that the buffer may not be aligned to a page boundary. */
  maxNumPages = ((lengthInBytes + PAGE_SIZE - 1) / PAGE_SIZE) + 1;

  /* Allocate structure to store information about all the physical pages
   * the buffer maps to.
   */
  *ppPageInfo = malloc (sizeof(DPIO2_DMA) + maxNumPages * sizeof(DPIO2_DMA_PAGE));
  if (*ppPageInfo == NULL) 
  {
    printf("Failed to allocate Page Information structure\n");
    free (*buffer);
    return (ERROR);
  }

  /* Lock the buffer into physical memory.
   */
  (*ppPageInfo)->pUserAddr = *buffer;
  (*ppPageInfo)->dwBytes = lengthInBytes;
  (*ppPageInfo)->dwPages = maxNumPages;
  status = dpio2DMALock (*ppPageInfo);
  if (status != OK) {
    printf("Failed to lock buffer into physical memory\n");
    free (*ppPageInfo);
    free (*buffer);
    return (ERROR);
  }

  return (OK);
}


STATUS configureDmaChain(int dpio2DeviceNumber, DPIO2_DMA* pPageInfo, int desc_num)
{
  STATUS   status;
  UINT32   iPage;
  DPIO2_DMA_DESC   dmaDescriptor;


  if (( pPageInfo->dwBytes & 0x00000003 ) != 0) {
    printf("Length must be multiple of 4 bytes\n");
    return (ERROR);
  } 

  if (( pPageInfo->Page[0].pPhysicalAddr & 0x00000003) != 0) {
    printf("PCI Address must be aligned to 4 bytes boundary\n");
    return (ERROR);
  }
  
  for (iPage = 0; iPage < (pPageInfo->dwPages - 1); iPage++) {
    dmaDescriptor.descriptorId        = iPage + desc_num;
    dmaDescriptor.nextDescriptorId    = iPage + desc_num + 1;
    dmaDescriptor.pciAddress       
      = (UINT32) pPageInfo->Page[iPage].pPhysicalAddr;
    dmaDescriptor.blockSizeInBytes    
      = pPageInfo->Page[iPage].dwBytes;
    dmaDescriptor.lastBlockInChain    = FALSE;
    dmaDescriptor.useD64              = TRUE;
    dmaDescriptor.notEndOfFrame       = FALSE;
    dmaDescriptor.endOfBlockInterrupt = FALSE;

    status = dpio2Ioctl(dpio2DeviceNumber, 
                        DPIO2_CMD_DMA_SET_DESC, 
//                      (int) &dmaDescriptor);
                        (unsigned long) &dmaDescriptor);                                    //rsr
    if (status != OK) {
      printf("Failed to set DMA Descriptor\n");
      return (ERROR);
    }
  }
  
  dmaDescriptor.descriptorId        = iPage + desc_num;
  dmaDescriptor.nextDescriptorId    = 0;
  dmaDescriptor.pciAddress          
    = (UINT32) pPageInfo->Page[iPage].pPhysicalAddr;
  dmaDescriptor.blockSizeInBytes    
    = pPageInfo->Page[iPage].dwBytes;
  dmaDescriptor.lastBlockInChain    = TRUE;
  dmaDescriptor.useD64              = TRUE;
  dmaDescriptor.notEndOfFrame       = FALSE;
  dmaDescriptor.endOfBlockInterrupt = TRUE;

  status = dpio2Ioctl(dpio2DeviceNumber, 
                      DPIO2_CMD_DMA_SET_DESC, 
//                    (int) &dmaDescriptor);
                      (unsigned long) &dmaDescriptor);                                     //rsr
  if (status != OK) {
    printf("Failed to set DMA Descriptor\n");
    return (ERROR);
  }

  return (OK);
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


void unlockAndFreeBuffer (DPIO2_DMA* pPageInfo, unsigned long *buffer)                     //changed UINT32 to unsigned long
{
  /* Unlock the pages of physical memory that the buffer maps. */
  dpio2DMAUnlock (pPageInfo);

  /* Free the page information structure and the buffer. */
  free (pPageInfo);

  free (buffer);
}


void closeDevice(int dpio2DeviceNumber)
{
  dpio2Close(dpio2DeviceNumber);
}





int main(int argc, char **argv)
{
  STATUS       status;
  DPIO2_DMA    dmaInfo_in;
  DPIO2_DMA    dmaInfo_out;
  DPIO2_DMA   *pPageInfo_in = &dmaInfo_in;
  DPIO2_DMA   *pPageInfo_out = &dmaInfo_out;
  int          devno_in;
  int          devno_out;
  unsigned long      *buffer_in;                                                           //rsr changed UINT32 to unsigned long
  unsigned long      *buffer_out;                                                          //rsr changed UINT32 to unsigned long
  sem_t        dmaDoneSemaphore_in;
  sem_t        dmaDoneSemaphore_out;
  int          freq;
  size_t       size;                                                                       //rsr
  int          numberOfDpio2Modules;
  
  int          dummy_int;
  char        *dummy_str;



  devno_in  = 2;
  devno_out = 1;
  freq = 20 * 1000 * 1000;
  size = 0x10000;

  dummy_int = argc;
  dummy_str = argv[0];

  status = scanForDevices (&numberOfDpio2Modules);
  if (status != OK) {
    return (-1);
  }
  printf("Found %d DPIO2 modules in system\n", numberOfDpio2Modules);
  if ((numberOfDpio2Modules < devno_in) || (numberOfDpio2Modules < devno_out)) {
    printf ("no dpio2 cards where found\n");
    return ERROR;
  }

  /* Initialize the device for input.*/
  status = openDpio2ForInput(devno_in);
  if (status != OK) {
    printf("Failed to initialize device number %d for input\n", devno_in);
    return (-1);
  }

  /* Initialize device for output. */
  status = openDpio2ForOutput(devno_out);
  if (status != OK) {
    printf("Failed to initialize device number %d for output\n", devno_out);
    return (-1);
  }

  /* Init semaphore */
  sem_init (&dmaDoneSemaphore_in,  0, 0);
  sem_init (&dmaDoneSemaphore_out, 0, 0);

  /* Configure interrupts  */
  status = configureDmaDoneInterrupt(devno_in, 
				     (FUNCPTR) dmaDoneNotifier,
				     &dmaDoneSemaphore_in);
  if (status != OK) {
    printf("Failed to configure DMA Done Interrupt\n");
    return (-1);
  }
  status = configureDmaDoneInterrupt(devno_out, 
				     (FUNCPTR) dmaDoneNotifier,
				     &dmaDoneSemaphore_out);
  if (status != OK) {
    printf("Failed to configure DMA Done Interrupt\n");
    return (-1);
  }

  /* Configure strobe reception. */
  status = enableStrobeReception(devno_in, freq);
  if (status != OK) {
    printf("Failed to configure Strobe Reception\n");
    return (-1);
  }

  /* Configure strobe generation. */
  status = enableStrobeGeneration(devno_out, freq);
  if (status != OK) {
    printf("Failed to configure Strobe Generation\n");
    return (-1);
  }

  /* Flush FIFO */
  dpio2Ioctl (devno_in,  DPIO2_CMD_FIFO_FLUSH, 0 );
  dpio2Ioctl (devno_out, DPIO2_CMD_FIFO_FLUSH, 0 );

  /* Set no swap */
  dpio2Ioctl (devno_in,  DPIO2_CMD_DATA_SWAP_MODE_SELECT, DPIO2_NO_SWAP);
  dpio2Ioctl (devno_out, DPIO2_CMD_DATA_SWAP_MODE_SELECT, DPIO2_NO_SWAP);

  /* Configure FPDP interface. */
  status = activateFpdpInterface (devno_in);
  if (status != OK) 
  {
    printf("Failed to activate FPDP interface\n");
    return (-1);
  }
  status = activateFpdpInterface (devno_out);
  if (status != OK) 
  {
    printf("Failed to activate FPDP interface\n");
    return (-1);
  }

  /* Allocate buffer. */
  status = allocateAndLockBuffer (size, &buffer_in, &pPageInfo_in);
  if (status != OK) {
    printf("Failed to allocate and lock buffer\n");
    return (-1);
  }
  status = allocateAndLockBuffer (size, &buffer_out, &pPageInfo_out);
  if (status != OK) {
    printf("Failed to allocate buffer\n");
    return (-1);
  }

  /* Configure DMA chain.  */
  status = configureDmaChain (devno_in, pPageInfo_in, 0);
  if (status != OK) {
    printf ("Failed to configure DMA chain\n");
    unlockAndFreeBuffer (pPageInfo_in, buffer_in);
    return (-1);
  }
  status = configureDmaChain (devno_out, pPageInfo_out, 0);
  if (status != OK) {
    printf ("Failed to configure DMA chain\n");
    unlockAndFreeBuffer (pPageInfo_out, buffer_out);
    return (-1);
  }



  /* 
   * Transfer data from DPIO2 device 1 ti DPIO2 device 2

   */

  printf ("Starting transfer ...");

  /* Start DMA transfer. */
  status = startDmaTransfer (devno_in, 0);
  if (status != OK) 
  {
    printf ("Failed to start DMA transfer\n");
    unlockAndFreeBuffer (pPageInfo_in, buffer_in);
    return (-1);
  }
  status = startDmaTransfer (devno_out, 0);
  if (status != OK) 
  {
    printf ("Failed to start DMA transfer\n");
    unlockAndFreeBuffer (pPageInfo_out, buffer_out);
    return (-1);
  }

  /* Wait for DMA transfer to complete */
  sem_wait (&dmaDoneSemaphore_in);
  sem_wait (&dmaDoneSemaphore_out);

  printf ("OK\n");




  /* Free buffer. */
  unlockAndFreeBuffer (pPageInfo_in,  buffer_in);
  unlockAndFreeBuffer (pPageInfo_out, buffer_out);

  /* Disable strove on both edges */
  dpio2Ioctl (devno_in,  DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT, FALSE);
  dpio2Ioctl (devno_out, DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT, FALSE);
  
  /* Close device   */
  closeDevice (devno_in);
  closeDevice (devno_out);

  return (0);
}








