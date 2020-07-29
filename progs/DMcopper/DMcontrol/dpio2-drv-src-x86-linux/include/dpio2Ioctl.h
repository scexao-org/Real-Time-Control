/****************************************************************************
Module      : dpio2Ioctl.h

Description : Header file defining the DPIO2 device structures
              and the ioctl functions for DPIO2.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01b,08jul2004,meh  Made changes necessary to compile file under VMOSA.
  01a,29oct2001,nib  Created from dpio2.cpp to extract the DPIO2 device
                     structures which are used in both Windows and VxWorks. 

*****************************************************************************/

/* includes */




#if defined(VXWORKS)

#include "vxWorks.h"
#include "CDpio2HwInterface.h"

#elif defined(WIN32)

#include "CWDpio2HwInterface.h"

#elif defined(MCOS)

#include <pthread.h>

#include "McosTypes.h"
#include "CMDpio2HwInterface.h"

#elif defined(VMOSA)

#include "dpio2.h"
#include "vmosa.h"
#include "vmosaCDpio2HwInterface.h"

#endif


#include "CDpio2DriverController.h"

#include "CDpio2InterruptAction.h"


#ifndef VMOSA
#include "dpio.h"
#include "dpio2.h"
#endif






/* defines */
#if defined(MCOS)
#define  MAX_NUM_CHARS_MODEL_NAME      8
#define  MAX_NUM_CHARS_DEVICE_NAME    40
#define  NUM_PCI_TO_RACE_CORRIDORS   128
#endif


/* typedefs */
#if defined(MCOS)

#define MYPID() proc_getid()

typedef struct MIDAS_SHARED /* Globaly shared structure */
{
  int               initialized;

  int               iFirstAvailableCorridor;
  int               numAvailableCorridors;
  struct {
    SMB_handle        hSmb;
    PID               owner;
  } smb[NUM_PCI_TO_RACE_CORRIDORS];

} MIDAS_SHARED;

typedef struct MIDAS_BOARD /* Local application structure */
{
  char              deviceName[MAX_NUM_CHARS_DEVICE_NAME];
  MIDAS_ID          id;
  SMB_handle        hPmcRegionSmb;

  int               numReferences;

  MIDAS_SHARED      *shared; /* shared memory */

} MIDAS_BOARD;
#else

#define MYPID() 0

#endif

typedef struct DPIO2_DEVICE {

#if defined(VXWORKS)
  DEV_HDR   deviceHeader;     /* VxWorks I/O device header, - always goes first */
#elif defined(WIN32)
  int   deviceNumber;
#elif defined(MCOS)
  int                  validityMark;

  MIDAS_BOARD*         pMidasBoard;

  SMB_mapping_handle   hDpio2SmbMapping;
  SMB_mapping_info     dpio2SmbMappingInfo;

  int                  pmcSlotNumber;

  pthread_t            interruptThreadId;
  bool                 stopIsSignaled;

  unsigned long        interruptPollPeriodInMs;

  int                  racewayPriority;

#elif defined(VMOSA)
  int   deviceNumber;

#endif

  CDpio2HwInterface*   pHwInterface;

  CDpio2DriverController*   pController;

  CDpio2InputDmaDescriptor    inputDescriptor;
  CDpio2OutputDmaDescriptor   outputDescriptor;

  UINT32   initialOscillatorFrequency;

  BOOL   enableHalt;                   /* Param. to set to halt proc. after DMA start */


  CDpio2DisableInterrupt   dpio2DisableAction[DPIO2_INT_CONDITIONS];

  CAction*           pDpio2CurrentDmaAction[DPIO2_INT_CONDITIONS];
  CDpio2StartDma     dpio2StartDmaAction[DPIO2_INT_CONDITIONS];
  CDpio2StopDma      dpio2StopDmaAction[DPIO2_INT_CONDITIONS];
  CDpio2SuspendDma   dpio2SuspendDmaAction[DPIO2_INT_CONDITIONS];
  CDpio2ResumeDma    dpio2ResumeDmaAction[DPIO2_INT_CONDITIONS];

  CDpio2Signal   dpio2CallbackAction[DPIO2_INT_CONDITIONS];

  DPIO2_DEVICE*   pNextDevice;
  
} DPIO2_DEVICE;




#ifndef VMOSA

typedef struct DPIO2_DPIO_API_DEVICE {

#if defined(VXWORKS)
  DEV_HDR   deviceHeader;     /* VxWorks I/O device header, - always goes first */
#elif defined(WIN32)
  int   deviceNumber;
#endif

  CDpio2HwInterface*   pHwInterface;

  CDpio2DriverController*   pController;

  CDpio2InputDmaDescriptor   inputDescriptor;

  UINT32                      previousDescriptorId;
  BOOL                        previousAssertSyncFlag;
  CDpio2OutputDmaDescriptor   outputDescriptor;

  UINT32   initialOscillatorFrequency;

  UINT8   dpioModeRegisterValue;
  UINT8   dpioPioControlRegisterValue;
  UINT8   dpioDmaControlRegisterValue;

  BOOL   enableHalt;                   /* Param. to set to halt proc. after DMA start */

  BOOL   byteSwapIsEnabled;

  CDpio2DisableInterrupt   dpioDisableAction[DPIO_INT_CONDITIONS];

  CAction*           pDpioCurrentDmaAction[DPIO_INT_CONDITIONS];
  CDpio2StartDma     dpioStartDmaAction[DPIO_INT_CONDITIONS];
  CDpio2StopDma      dpioStopDmaAction[DPIO_INT_CONDITIONS];
  CDpio2SuspendDma   dpioSuspendDmaAction[DPIO_INT_CONDITIONS];
  CDpio2ResumeDma    dpioResumeDmaAction[DPIO_INT_CONDITIONS];

  CDpio2Signal   dpioCallbackAction[DPIO_INT_CONDITIONS];

  DPIO2_DPIO_API_DEVICE*   pNextDevice;
  
} DPIO2_DPIO_API_DEVICE;


#endif /* VMOSA */


STATUS dpio2Ioctl(DPIO2_DEVICE* pDevice, int command, int argument);
STATUS dpio2Ioctl(DPIO2_DEVICE* pDevice, int command, void* argument);             //rsr
#ifndef VMOSA
STATUS dpio2DpioApiIoctl(DPIO2_DPIO_API_DEVICE* pDevice, int command, int argument);
void dpio2SetHaltFunction(FUNCPTR pFunc);

void updateDpioModeRegister(CDpio2DriverController* pController, UINT8 currentValue, UINT8 newValue, UINT32 initialOscillatorFrequency);
void updateDpioPioControlRegister(CDpio2DriverController* pController, UINT8 currentValue, UINT8 newValue);
void updateDpioInterruptControlRegister(CDpio2DriverController* pController, UINT8 currentValue, UINT8 newValue);
void updateDpioDmaControlRegister(CDpio2DriverController* pController, UINT8 currentValue, UINT8 newValue);
#endif /* VMOSA */

#ifndef VMOSA
DPIO2_INTERRUPT_SOURCE_ID mapDpioConditionToSource(int condition);
#endif /* VMOSA */
DPIO2_INTERRUPT_SOURCE_ID mapDpio2ConditionToSource(int condition);

#ifndef VMOSA
STATUS dpio2DpioApiInitInterruptActions(DPIO2_DPIO_API_DEVICE* pDevice);
#endif /* VMOSA */

STATUS dpio2InitInterruptActions(DPIO2_DEVICE* pDevice);









