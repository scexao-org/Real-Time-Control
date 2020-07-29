/****************************************************************************
Module      : CDpio2DriverState.cpp

Description : Source file defining the CDpio2DataDirectionState, 
              CDpio2DriverInputState, and CDpio2DriverOutputState classes.

              CDpio2DataDirectionState is an abstract class which defines the
              direction dependent functionality available to an application.

              A CDpio2DriverInputState object implements the direction 
              dependent functionality when a DPIO2 is configured as input.

              A CDpio2DriverOutputState object implements the direction 
              dependent functionality when a DPIO2 is configured as output.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01q,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01p,13feb2004,meh  Support new Strobe Frequency Range.
  01o,22jan2004,meh  Added support for _66_pcb_B
  01n,01oct2003,meh  Changed ERROR_MSG to DPIO2_ERROR_MSG.
                     Changed the DEBUG_MSG with TRUE flag to
                     DPIO2_WARNING_MSG.
  01m,02oct2002,nib  Renamed TTL and PECL strobe to primary and secondary strobe.
  01l,19sep2002,nib  Updated the default skew settings when double clocking is enabled.
  01k,13sep2002,nib  Added the enableEotSyncCount() and disableEotSyncCount() methods 
                     in the direction state classes.
  01j,04jul2002,nib  Removed message about jumper for strobe generation for RS422 modules.
  01i,22may2002,nib  Added state classes for PECL and RS422 modules.
  01h,22feb2002,nib  Added methods for controlling forced assertion of SUSPEND.
  01g,12feb2002,nib  Added the setSyncGenerationCounter() method to the direction 
                     state classes.
  01f,04feb2002,nib  Added 1ms delay in the configurePll() methods.
  01e,23jan2002,nib  Added PLL control state classes.
                     Removed constructors from all state classes.
  01d,31oct2001,nib  Added methods for counter addressing in the direction state classes
                     - enableCounterAddressing() and disableCounterAddressing()
  01c,03oct2001,nib  Renamed CDpio2DriverState to CDpio2DataDirectionState.
  01b,02oct2001,nib  Added methods; enableDvalidAlwaysAsserted()
                     and disableDvalidAlwaysAsserted().
  01a,22jun2001,nib  Created.

*****************************************************************************/
#if (defined VXWORKS)

#include <vxWorks.h>
#include <sysLib.h>
#include <taskLib.h>

#elif (defined WIN32)


#elif (defined VMOSA)

#include "vmosa.h"

#endif


#include "CDpio2DriverState.h"
#include "CDpio2FrontEnd.h"
#include "CDpio2Oscillator.h"

#include "Dpio2Defs.h"




#if 0 /*meh*/
#define MARKM printf ("Was Here %s,%d\n", __FILE__, __LINE__);
#endif





STATUS CDpio2DriverInputState::setInputDmaDescriptor(CDpio2DriverController* pController,
						     UINT32 descriptorId,
						     const CDpio2InputDmaDescriptor* pDescriptor)
{

  return (pController->m_pPrimaryDmaController->setDescriptor(descriptorId, pDescriptor));

}


STATUS CDpio2DriverInputState::setOutputDmaDescriptor(CDpio2DriverController* pController,
						      UINT32 descriptorId,
						      const CDpio2OutputDmaDescriptor* pDescriptor)
{

  DPIO2_ERROR_MSG(("Cannot set Output DMA descriptor on DPIO2 configured as Input"));
  
  /* compiler quiet */
  pController  = pController;
  descriptorId = descriptorId;
  pDescriptor  = pDescriptor;

  return (ERROR);

}


STATUS CDpio2DriverInputState::selectEndOfTransferMode(CDpio2DriverController* pController, 
                                                       DPIO2_END_OF_TRANSFER_MODE mode)
{

  pController->m_pPrimaryDmaController->selectEndOfTransferMode(mode);

  return (OK);

}


STATUS CDpio2DriverInputState::enableEndOfTransfer(CDpio2DriverController* pController)
{

  pController->m_pPrimaryDmaController->enableEndOfTransfer();

  return (OK);
}


STATUS CDpio2DriverInputState::disableEndOfTransfer(CDpio2DriverController* pController)
{

  pController->m_pPrimaryDmaController->disableEndOfTransfer();

  return (OK);

}


STATUS CDpio2DriverInputState::enableEotSyncCount(CDpio2DriverController* pController,
                                                  UINT16 countValue)
{

  pController->m_pPrimaryDmaController->enableEotSyncCount(countValue);

  return (OK);

}


STATUS CDpio2DriverInputState::disableEotSyncCount(CDpio2DriverController* pController)
{

  pController->m_pPrimaryDmaController->disableEotSyncCount();

  return (OK);

}


STATUS CDpio2DriverInputState::enableContinueOnEndOfTransfer(CDpio2DriverController* pController)
{

  pController->m_pPrimaryDmaController->enableContinueOnEndOfTransfer();

  return (OK);

}


STATUS CDpio2DriverInputState::disableContinueOnEndOfTransfer(CDpio2DriverController* pController)
{

  pController->m_pPrimaryDmaController->disableContinueOnEndOfTransfer();

  return (OK);

}


STATUS CDpio2DriverInputState::setSyncGenerationCounter(CDpio2DriverController* pController, 
                                                        UINT32 value)
{

  DPIO2_ERROR_MSG(("Setting the SYNC Generation Counter is not applicable for an input module.\n"));

  /* compiler quiet */
  pController  = pController;
  value        = value; 

  return (ERROR);

}


STATUS CDpio2DriverInputState::selectSyncGenerationMode(CDpio2DriverController* pController,
                                                        DPIO2_SYNC_GENERATION_MODE mode)
{

  DPIO2_ERROR_MSG(("Selecting SYNC generation mode is not applicable for an input module.\n"));

  /* compiler quiet */
  pController  = pController;
  mode         = mode; 

  return (ERROR);

}


STATUS CDpio2DriverInputState::selectSyncReceptionMode(CDpio2DriverController* pController,
                                                       DPIO2_SYNC_RECEPTION_MODE mode)
{

  pController->m_pFrontEnd->selectSyncReceptionMode(mode);

  return (OK);

}


STATUS CDpio2DriverInputState::enableVideoMode(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->enableVideoMode();

  return (OK);

}


STATUS CDpio2DriverInputState::disableVideoMode(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->disableVideoMode();

  return (OK);

}


STATUS CDpio2DriverInputState::enableDvalidAlwaysAsserted(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->enableDvalidAlwaysAsserted();

  return (OK);

}


STATUS CDpio2DriverInputState::disableDvalidAlwaysAsserted(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->disableDvalidAlwaysAsserted();

  return (OK);

}


STATUS CDpio2DriverInputState::enableCounterAddressing(CDpio2DriverController* pController,
                                                       UINT32 initialSkipCount,
                                                       UINT32 skipCount, 
                                                       UINT32 receiveCount)
{

  const UINT32   MIN_COUNT_VALUE = 2;
  const UINT32   MAX_COUNT_VALUE = 0x00FFFFFF + 1;

  STATUS   status;


  /* Check that the count values are valid.
   */
  status = OK;

  if ( (initialSkipCount != 0) && (initialSkipCount < MIN_COUNT_VALUE) ) {

    DPIO2_ERROR_MSG(("Initial Skip Count must be zero or higher than %d\n", (int) (MIN_COUNT_VALUE - 1)));
    status = ERROR;

  }

  if ( initialSkipCount > MAX_COUNT_VALUE ) {

    DPIO2_ERROR_MSG(("Initial Skip Count must be lower than %d\n", (int) (MAX_COUNT_VALUE + 1)));
    status = ERROR;

  }

  if ( skipCount < MIN_COUNT_VALUE ) {

    DPIO2_ERROR_MSG(("Skip Count must be higher than %d\n", (int) (MIN_COUNT_VALUE - 1)));
    status = ERROR;

  }

  if ( skipCount > MAX_COUNT_VALUE ) {

    DPIO2_ERROR_MSG(("Skip Count must be lower than %d\n", (int) (MAX_COUNT_VALUE + 1)));
    status = ERROR;

  }

  if ( receiveCount < MIN_COUNT_VALUE ) {

    DPIO2_ERROR_MSG(("Receive Count must be higher than %d\n", (int) (MIN_COUNT_VALUE - 1)));
    status = ERROR;

  }

  if ( receiveCount > MAX_COUNT_VALUE ) {

    DPIO2_ERROR_MSG(("Receive Count must be lower than %d\n", (int) (MAX_COUNT_VALUE + 1)));
    status = ERROR;

  }

  
  if ( status == OK ) {

    if ( initialSkipCount == 0 ) {

      pController->m_pFrontEnd->setInitialSkipCounter(0);

    } else {

      pController->m_pFrontEnd->setInitialSkipCounter(initialSkipCount - MIN_COUNT_VALUE + 1);

    }

    pController->m_pFrontEnd->setSkipCounter(skipCount - MIN_COUNT_VALUE + 1);

    pController->m_pFrontEnd->setReceiveCounter(receiveCount - MIN_COUNT_VALUE + 1);

    pController->m_pFrontEnd->enableCounterAddressing();

  }

  return (status);

}


STATUS CDpio2DriverInputState::disableCounterAddressing(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->disableCounterAddressing();

  return (OK);

}


STATUS CDpio2DriverInputState::checkIsPipelineNotEmpty(CDpio2DriverController* pController, 
                                                       BOOL& flag)
{

  flag = pController->m_pFrontEnd->checkIsPipelineNotEmpty();

  return (OK);

}


STATUS CDpio2DriverInputState::flushPackingPipeline(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->flushPackingPipeline();

  return (OK);

}



STATUS CDpio2DriverInputState::enableForcedSuspendAssertion(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->enableForcedSuspendAssertion();

  return (OK);

}



STATUS CDpio2DriverInputState::disableForcedSuspendAssertion(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->disableForcedSuspendAssertion();

  return (OK);

}



STATUS CDpio2DriverOutputState::setInputDmaDescriptor(CDpio2DriverController* pController,
						      UINT32 descriptorId,
						      const CDpio2InputDmaDescriptor* pDescriptor)
{

  DPIO2_ERROR_MSG(("Cannot set Input DMA descriptor on DPIO2 configured as Output"));

  /* compiler quiet */
  pController  = pController;
  descriptorId = descriptorId;
  pDescriptor  = pDescriptor;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::setOutputDmaDescriptor(CDpio2DriverController* pController,
						       UINT32 descriptorId,
						       const CDpio2OutputDmaDescriptor* pDescriptor)
{

  return (pController->m_pPrimaryDmaController->setDescriptor(descriptorId, pDescriptor));

}


STATUS CDpio2DriverOutputState::selectEndOfTransferMode(CDpio2DriverController* pController, 
                                                        DPIO2_END_OF_TRANSFER_MODE mode)
{

  DPIO2_ERROR_MSG(("Cannot select End-Of-Transfer Mode - not applicable to an output module.\n"));

  /* compiler quiet */
  pController  = pController;
  mode         = mode;

  return (ERROR);

}


STATUS CDpio2DriverOutputState::enableEndOfTransfer(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Cannot enable End Of Transfer - not applicable to an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);

}


STATUS CDpio2DriverOutputState::disableEndOfTransfer(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Cannot disable End Of Transfer - not applicable to an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::enableEotSyncCount(CDpio2DriverController* pController,
                                                  UINT16 countValue)
{

  DPIO2_ERROR_MSG(("Cannot enable EOT SYNC count - not applicable to an output module.\n"));

  /* compiler quiet */
  pController = pController;
  countValue  = countValue;

  return (ERROR);

}


STATUS CDpio2DriverOutputState::disableEotSyncCount(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Cannot disable EOT SYNC count - not applicable to an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);

}


STATUS CDpio2DriverOutputState::enableContinueOnEndOfTransfer(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Cannot enable continue on End Of Transfer - not applicable to an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::disableContinueOnEndOfTransfer(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Cannot disable continue on End Of Transfer - not applicable to an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::setSyncGenerationCounter(CDpio2DriverController* pController, 
                                                         UINT32 value)
{

  const UINT32   MIN_COUNT_VALUE = 2;


  if ( (value != 0) && (value < MIN_COUNT_VALUE) ) {

    DPIO2_ERROR_MSG(("SYNC Generation Counter must be zero or higher than %d\n", 
               (int) (MIN_COUNT_VALUE - 1)));
    return (ERROR);

  }


  if ( value == 0 ) {

    pController->m_pFrontEnd->setSyncCounter(0);

  } else {

    pController->m_pFrontEnd->setSyncCounter(value - 1);

  }

  return (OK);

}


STATUS CDpio2DriverOutputState::selectSyncGenerationMode(CDpio2DriverController* pController,
                                                         DPIO2_SYNC_GENERATION_MODE mode)
{

  pController->m_pFrontEnd->selectSyncGenerationMode(mode);

  return (OK);

}


STATUS CDpio2DriverOutputState::selectSyncReceptionMode(CDpio2DriverController* pController,
                                                       DPIO2_SYNC_RECEPTION_MODE mode)
{

  DPIO2_ERROR_MSG(("Selecting SYNC reception mode is not applicable for a output module.\n"));

  /* compiler quiet */
  pController = pController;
  mode        = mode;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::enableVideoMode(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Enabling Video mode is not applicable for a output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::disableVideoMode(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Disabling Video mode is not applicable for a output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::enableDvalidAlwaysAsserted(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("An output module cannot assume that DVALID is always asserted.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::disableDvalidAlwaysAsserted(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("An output module cannot assume that DVALID is always asserted.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);
}


STATUS CDpio2DriverOutputState::enableCounterAddressing(CDpio2DriverController* pController,
                                                       UINT32 initialSkipCount,
                                                       UINT32 skipCount, 
                                                       UINT32 receiveCount)
{

  DPIO2_ERROR_MSG(("Counter Addressing is not implemented for output modules.\n"));

  /* compiler quiet */
  pController      = pController;
  initialSkipCount = initialSkipCount;
  skipCount        = skipCount;
  receiveCount     = receiveCount;

  return (ERROR);

}


STATUS CDpio2DriverOutputState::disableCounterAddressing(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Counter Addressing is not implemented for output modules.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);

}


STATUS CDpio2DriverOutputState::checkIsPipelineNotEmpty(CDpio2DriverController* pController, 
                                                        BOOL& flag)
{

  DPIO2_ERROR_MSG(("Can not check for data in pipeline on an output module.\n"));

  /* compiler quiet */
  pController = pController;
  flag        = flag;

  return (ERROR);

}


STATUS CDpio2DriverOutputState::flushPackingPipeline(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Can not flush data in pipeline on an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);

}



STATUS CDpio2DriverOutputState::enableForcedSuspendAssertion(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Forced SUSPEND assertion is not applicable on an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);

}



STATUS CDpio2DriverOutputState::disableForcedSuspendAssertion(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Forced SUSPEND assertion is not applicable on an output module.\n"));

  /* compiler quiet */
  pController = pController;

  return (ERROR);

}



STATUS CDpio2Packing8Bit4BitState::setPackingMode(CDpio2DriverController* pController, 
                                                  DPIO2_PACKING_MODE mode)
{

  STATUS   status;


  switch (mode) {

  case NO_PACKING:
    pController->m_pFrontEnd->setPackingMode(RAW_NO_PACKING);
    status = OK;
    break;

  case PACK_16_LEAST_SIGNIFICANT_FPDP_BITS:
    DPIO2_ERROR_MSG(("The FPGA image must be changed in order to pack 16 LSB on FPDP\n"));
    status = ERROR;
    break;

  case PACK_10_LEAST_SIGNIFICANT_FPDP_BITS:
    DPIO2_ERROR_MSG(("The FPGA image must be changed in order to pack 16 LSB on FPDP\n"));
    status = ERROR;
    break;

  case PACK_8_LEAST_SIGNIFICANT_FPDP_BITS:
    pController->m_pFrontEnd->setPackingMode(RAW_PACK_16LSB_OR_8LSB);
    status = OK;
    break;

  case PACK_4_LEAST_SIGNIFICANT_FPDP_BITS:
    pController->m_pFrontEnd->setPackingMode(RAW_PACK_10LSB_OR_4LSB);
    status = OK;
    break;

  case PACK_16_MOST_SIGNIFICANT_FPDP_BITS:
    DPIO2_ERROR_MSG(("The FPGA image must be changed in order to pack 16 MSB on FPDP\n"));
    status = ERROR;
    break;

  default:
    DPIO2_ERROR_MSG(("Unknown packing mode (%d)\n", mode));
    status = ERROR;
    break;    

  }


  return (status);

}


STATUS CDpio2Packing16Bit10BitState::setPackingMode(CDpio2DriverController* pController, 
                                                    DPIO2_PACKING_MODE mode)
{

  STATUS   status;


  switch (mode) {

  case NO_PACKING:
    pController->m_pFrontEnd->setPackingMode(RAW_NO_PACKING);
    status = OK;
    break;

  case PACK_16_LEAST_SIGNIFICANT_FPDP_BITS:
    pController->m_pFrontEnd->setPackingMode(RAW_PACK_16LSB_OR_8LSB);
    status = OK;
    break;

  case PACK_10_LEAST_SIGNIFICANT_FPDP_BITS:
    pController->m_pFrontEnd->setPackingMode(RAW_PACK_10LSB_OR_4LSB);
    status = OK;
    break;

  case PACK_8_LEAST_SIGNIFICANT_FPDP_BITS:
    DPIO2_ERROR_MSG(("The FPGA image must be changed in order to pack 8 LSB on FPDP\n"));
    status = ERROR;
    break;

  case PACK_4_LEAST_SIGNIFICANT_FPDP_BITS:
    DPIO2_ERROR_MSG(("The FPGA image must be changed in order to pack 4 LSB on FPDP\n"));
    status = ERROR;
    break;

  case PACK_16_MOST_SIGNIFICANT_FPDP_BITS:
    pController->m_pFrontEnd->setPackingMode(RAW_PACK_16MSB);
    status = OK;
    break;

  default:
    DPIO2_ERROR_MSG(("Unknown packing mode (%d)\n", mode));
    status = ERROR;
    break;    

  }


  return (status);

}


void CDpio2PcbAState::configurePll(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;

  BOOL   clockingOnBothStrobeEdgesIsEnabled;
  BOOL   moduleIsNotGeneratingStrobe;
  BOOL   strobeFrequencyIsBelow10Mhz;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  clockingOnBothStrobeEdgesIsEnabled
    = pFrontEnd->checkIsClockingOnBothStrobeEdgesEnabled();

  moduleIsNotGeneratingStrobe = !pFrontEnd->checkIsStrobeGenerationEnabled();

  strobeFrequencyIsBelow10Mhz
    = (pController->m_strobeFrequencyRange == BELOW_OR_EQUAL_TO_5MHZ)
      || (pController->m_strobeFrequencyRange == ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ);

  if ( !clockingOnBothStrobeEdgesIsEnabled
       && (strobeFrequencyIsBelow10Mhz
           || (moduleIsNotGeneratingStrobe && pController->m_inputStrobeIsAsynchronous))) {

    pFrontEnd->disableStrobePll();

  } else {

    pFrontEnd->enableStrobePll();

  }


  /* Delay 1ms to allow the PLL to lock properly.
   */
#if (defined VXWORKS)

  {

    int   systemClockRate;
    int   delayInTicks;


    systemClockRate = sysClkRateGet();

    /* Delay two ticks if the system clock resolution is too coarse 
     * to represent milliseconds.  Otherwise, compute the necessary number
     * of ticks and add one to ensure that the delay is 1 ms or longer.
     */
    if ( (systemClockRate / 1000) > 1 ) {

      delayInTicks = (systemClockRate / 1000) + 1;

    } else {

      delayInTicks = 2;

    }

    taskDelay(delayInTicks);

  }

#elif (defined WIN32)

  Sleep(1);

#elif defined (MCOS)

  {
    
    struct timespec   delay = {0, 1*1000*1000};
    
    nanosleep(&delay, NULL);

  }

#elif (defined VMOSA)

  vmosa_mdelay (1);

#else

#error "1 ms delay is not implemented"

#endif

}


STATUS CDpio2PcbAState::setStrobeSkew(CDpio2DriverController* pController,
                                      int skewInTimingUnits)
{

  DPIO2_ERROR_MSG(("Cannot control Strobe Skew on a DPIO2 revision A board\n"));

  /* compiler quiet */
  pController       = pController;
  skewInTimingUnits = skewInTimingUnits;

  return (ERROR);

}


STATUS CDpio2PcbAState::useDefaultStrobeSkew(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("Cannot control Strobe Skew on a DPIO2 revision A board\n"));

  /* compiler quiet */
  pController       = pController;

  return (ERROR);

}



CDpio2PcbBState::CDpio2PcbBState()
  : CDpio2PllControlState(),
    m_useDefaultStrobeSkew(TRUE)
{
}


void CDpio2PcbBState::configurePll(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;

  BOOL     strobeFrequencyIsBelow10Mhz;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();



  m_clockingOnBothStrobeEdgesIsEnabled
    = pFrontEnd->checkIsClockingOnBothStrobeEdgesEnabled();

  m_moduleIsGeneratingStrobe = pFrontEnd->checkIsStrobeGenerationEnabled();

  strobeFrequencyIsBelow10Mhz
    = (pController->m_strobeFrequencyRange == BELOW_OR_EQUAL_TO_5MHZ)
       || (pController->m_strobeFrequencyRange == ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ);


  if ( !m_clockingOnBothStrobeEdgesIsEnabled
       && (strobeFrequencyIsBelow10Mhz
           || ((!m_moduleIsGeneratingStrobe) && pController->m_inputStrobeIsAsynchronous))) {

    pFrontEnd->disableStrobePll();

  } else {

    pFrontEnd->enableStrobePll();

    configurePllFrequencySelect(pController);

    configurePllOptions(pController);

  }


  /* Delay 1ms to allow the PLL to lock properly.
   */
#if (defined VXWORKS)

  {

    int   systemClockRate;
    int   delayInTicks;


    systemClockRate = sysClkRateGet();

    /* Delay two ticks if the system clock resolution is too coarse 
     * to represent milliseconds.  Otherwise, compute the necessary number
     * of ticks and add one to ensure that the delay is 1 ms or longer.
     */
    if ( (systemClockRate / 1000) > 1 ) {

      delayInTicks = (systemClockRate / 1000) + 1;

    } else {

      delayInTicks = 2;

    }

    taskDelay(delayInTicks);

  }

#elif (defined WIN32)

  Sleep(1);

#elif defined (MCOS)

  {
    
    struct timespec   delay = {0, 1*1000*1000};
    
    nanosleep(&delay, NULL);

  }

#elif (defined VMOSA)

  vmosa_mdelay (1);

#else

#error "1 ms delay is not implemented"

#endif

}




/*meh*/
void CDpio2_66MHzPcbBState::configurePll(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;

  BOOL     strobeFrequencyIsBellowOrEqualTo20Mhz;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  m_clockingOnBothStrobeEdgesIsEnabled
    = pFrontEnd->checkIsClockingOnBothStrobeEdgesEnabled();

  m_moduleIsGeneratingStrobe = pFrontEnd->checkIsStrobeGenerationEnabled();

  strobeFrequencyIsBellowOrEqualTo20Mhz =
    (pController->m_strobeFrequencyRange == BELOW_OR_EQUAL_TO_5MHZ) ||
    (pController->m_strobeFrequencyRange == ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ) ||
    (pController->m_strobeFrequencyRange == ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ) ||
    (pController->m_strobeFrequencyRange == ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ); 
  
  
  if ( !m_clockingOnBothStrobeEdgesIsEnabled
       && ( strobeFrequencyIsBellowOrEqualTo20Mhz
           || ((!m_moduleIsGeneratingStrobe) && pController->m_inputStrobeIsAsynchronous))) {
    
    pFrontEnd->disableStrobePll();


  } else {


    pFrontEnd->enableStrobePll();

    configurePllFrequencySelect(pController);

    configurePllOptions(pController);

  }


  /* Delay 1ms to allow the PLL to lock properly.
   */
#if (defined VXWORKS)

  {

    int   systemClockRate;
    int   delayInTicks;


    systemClockRate = sysClkRateGet();

    /* Delay two ticks if the system clock resolution is too coarse 
     * to represent milliseconds.  Otherwise, compute the necessary number
     * of ticks and add one to ensure that the delay is 1 ms or longer.
     */
    if ( (systemClockRate / 1000) > 1 ) {

      delayInTicks = (systemClockRate / 1000) + 1;

    } else {

      delayInTicks = 2;

    }

    taskDelay(delayInTicks);

  }

#elif (defined WIN32)

  Sleep(1);

#elif defined (MCOS)

  {
    
    struct timespec   delay = {0, 1*1000*1000};
    
    nanosleep(&delay, NULL);

  }

#elif (defined VMOSA)

  vmosa_mdelay (1);

#else

#error "1 ms delay is not implemented"

#endif

}


/*meh*/
BOOL CDpio2PcbAState::checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range)
{


  /* check if old values */

  if ( (range == BELOW_OR_EQUAL_TO_5MHZ) ||
       (range == ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ) ||
       (range == ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ) ||
       (range == ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ) ||
       (range == ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ) ||
       (range == ABOVE_100MHZ) ) {

    return TRUE;

  }

  
  /* change new values to old */

  switch (range) {

  case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ:
  case ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ:
  case ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ:
    range = ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ;
    break;

  case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ:
  case ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ:
  case ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ:
  case ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ:
  case ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ:
    range = ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ;
    break;

  case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ:
  case ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ:
  case ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ:
  case ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ:
  case ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ:
  case ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ:
  case ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ:
  case ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ:
  case ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ:
  case ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ:
    range = ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ;
    break;

  default:
    DPIO2_ERROR_MSG(("The given Strobe Frequency Range is not valid"));
    return FALSE;

  }


  return TRUE;


}



/*meh*/
BOOL CDpio2PcbBState::checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range)
{



  /* check if old values */

  if ( (range == BELOW_OR_EQUAL_TO_5MHZ) ||
       (range == ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ) ||
       (range == ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ) ||
       (range == ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ) ||
       (range == ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ) ||
       (range == ABOVE_100MHZ) ) {

    return TRUE;

  }

  
  /* change new values to old */

  switch (range) {

  case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ:
  case ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ:
  case ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ:
    range = ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ;
    break;

  case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ:
  case ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ:
  case ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ:
  case ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ:
  case ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ:
    range = ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ;
    break;

  case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ:
  case ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ:
  case ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ:
  case ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ:
  case ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ:
  case ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ:
  case ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ:
  case ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ:
  case ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ:
  case ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ:
    range = ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ;
    break;

  default:
    DPIO2_ERROR_MSG(("The given Strobe Frequency Range is not valid"));
    return FALSE;

  }


  return TRUE;


}



/*meh*/
BOOL CDpio2_66MHzPcbBState::checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range)
{

  if ( (range == BELOW_OR_EQUAL_TO_5MHZ) ||
       (range == ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ) ||
       (range == ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ) ||
       (range == ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ) ||
       (range == ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ) ||
       (range == ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ) ||
       (range == ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ) ||
       (range == ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ) ||
       (range == ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ) ||
       (range == ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ) ||
       (range == ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ) ||
       (range == ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ) ||
       (range == ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ) ||
       (range == ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ) ||
       (range == ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ) ||
       (range == ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ) ||
       (range == ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ) ||
       (range == ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ) ||
       (range == ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ) ||
       (range == ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ) ||
       (range == ABOVE_100MHZ) ) {

    return TRUE;
    
  } 
       

  DPIO2_ERROR_MSG(("The given Strobe Frequency Range is not valid"));
  
  return FALSE;


}




/*meh*/
DPIO2_STROBE_FREQUENCY_RANGE CDpio2PcbAState::getStrobeFrequencyRange(UINT32 frequency)
{

  if ( frequency <= (5*1000*1000) ) {

    return BELOW_OR_EQUAL_TO_5MHZ;

  } else if ( frequency <= (10*1000*1000) ) {

    return ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ;

  } else  if ( frequency <= (25*1000*1000) ) {

    return ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ;

  } else  if ( frequency <= (50*1000*1000) ) {

    return ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ;

  } else  if ( frequency <= (100*1000*1000) ) {

    return ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ;

  } else {

    return ABOVE_100MHZ;

  }


  return ((DPIO2_STROBE_FREQUENCY_RANGE) ERROR);

}





/*meh*/
DPIO2_STROBE_FREQUENCY_RANGE CDpio2PcbBState::getStrobeFrequencyRange(UINT32 frequency)
{

  if ( frequency <= (5*1000*1000) ) {

    return BELOW_OR_EQUAL_TO_5MHZ;

  } else if ( frequency <= (10*1000*1000) ) {

    return ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ;

  } else  if ( frequency <= (25*1000*1000) ) {

    return ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ;

  } else  if ( frequency <= (50*1000*1000) ) {

    return ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ;

  } else  if ( frequency <= (100*1000*1000) ) {

    return ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ;

  } else {

    return ABOVE_100MHZ;

  }


  return ((DPIO2_STROBE_FREQUENCY_RANGE) ERROR);
  
}



/*meh*/
DPIO2_STROBE_FREQUENCY_RANGE CDpio2_66MHzPcbBState::getStrobeFrequencyRange(UINT32 frequency)
{


  if ( frequency <= (5*1000*1000) ) {

    return BELOW_OR_EQUAL_TO_5MHZ;

  } else  if ( frequency <= (10*1000*1000) ) {

    return ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ;

  } else  if ( frequency <= (15*1000*1000) ) {

    return ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ;

   } else  if ( frequency <= (20*1000*1000) ) {

    return ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ;

  } else  if ( frequency <= (25*1000*1000) ) {

    return ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ;

  } else  if ( frequency <= (30*1000*1000) ) {

    return ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ;

  } else  if ( frequency <= (35*1000*1000) ) {

    return ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ;

  } else  if ( frequency <= (40*1000*1000) ) {

    return ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ;

  } else  if ( frequency <= (45*1000*1000) ) {

    return ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ;

  } else  if ( frequency <= (50*1000*1000) ) {

    return ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ;

  } else  if ( frequency <= (55*1000*1000) ) {

    return ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ;

  } else  if ( frequency <= (60*1000*1000) ) {

    return ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ;

  } else  if ( frequency <= (65*1000*1000) ) {

    return ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ;

  } else  if ( frequency <= (70*1000*1000) ) {

    return ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ;

  } else  if ( frequency <= (75*1000*1000) ) {

    return ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ;

  } else  if ( frequency <= (80*1000*1000) ) {

    return ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ;

  } else  if ( frequency <= (85*1000*1000) ) {

    return ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ;

  } else  if ( frequency <= (90*1000*1000) ) {

    return ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ;

  } else  if ( frequency <= (95*1000*1000) ) {

    return ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ;

  } else  if ( frequency <= (100*1000*1000) ) {

    return ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ;

  } else {

    return ABOVE_100MHZ;

  }


  return ((DPIO2_STROBE_FREQUENCY_RANGE) ERROR);


}








void CDpio2PcbBState::configurePllFrequencySelect(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the Frequency Range of the PLL.
   */
  switch (pController->m_strobeFrequencyRange) {

  case BELOW_OR_EQUAL_TO_5MHZ:
  case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
  
  case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ:

    pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_LOW);
    break;

  case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ:

    pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_MID);
    break;

  case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ:

    pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_HIGH);
    break;

  case ABOVE_100MHZ:

    DPIO2_WARNING_MSG(("Warning: PLL is not designed for strobe frequency above 100MHz\n"), TRUE);
    pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_HIGH);
    break;

  default:
    DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
               pController->m_strobeFrequencyRange), TRUE);
    break;

  }

}


/*meh*/
void CDpio2_66MHzPcbBState::configurePllFrequencySelect(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the Frequency Range of the PLL.
   */

  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {


    switch (pController->m_strobeFrequencyRange) {

    case BELOW_OR_EQUAL_TO_5MHZ:
    case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
    case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ:
    case ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ:

      break;
      
    case ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ:
    case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ:
    case ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ:
    case ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ:
    case ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ:
    case ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ:
      
      pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_LOW);
      break;
      
    case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ:
    case ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ:
    case ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ:
    case ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ:
    case ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ:
    case ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ:
    case ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ:
    case ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ:
    case ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ:
    case ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ:
      
      pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_MID);
      break;
      
    case ABOVE_100MHZ:
      
      DPIO2_WARNING_MSG(("Warning: PLL is not designed for strobe frequency above 100MHz\n"), TRUE);
      pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_HIGH);
      break;
      
    default:
      DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
                         pController->m_strobeFrequencyRange), TRUE);
      break;
      
    }

    
  } else {
    

    switch (pController->m_strobeFrequencyRange) {
      
    case BELOW_OR_EQUAL_TO_5MHZ:
    case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
      
      break;

    case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ:
    case ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ:
    case ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ:  

      pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_LOW);
      break;

    case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ:
    case ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ:
    case ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ:
    case ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ:
    case ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ:
      
      pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_MID);
      break;
      
    case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ:
    case ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ:
    case ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ:
    case ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ:
    case ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ:
    case ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ:
    case ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ:
    case ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ:
    case ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ:
    case ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ:
      
      pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_HIGH);
      break;
      
    case ABOVE_100MHZ:
      
      DPIO2_WARNING_MSG(("Warning: PLL is not designed for strobe frequency above 100MHz\n"), TRUE);
      pFrontEnd->setPllFrequencySelect(DPIO2_PLL_OPTION_HIGH);
      break;
      
    default:
      DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
                         pController->m_strobeFrequencyRange), TRUE);
      break;
      
    }


  }


}



STATUS CDpio2PcbBState::setStrobeSkew(CDpio2DriverController* pController,
                                      int skewInTimingUnits)
{

  /* Determine the PLL options that correspond to the specified skew value.
   */
  switch (skewInTimingUnits) {

  case (-6):
    m_pllOption1 = DPIO2_PLL_OPTION_LOW;
    m_pllOption0 = DPIO2_PLL_OPTION_MID;
    break;

  case (-4):
    m_pllOption1 = DPIO2_PLL_OPTION_LOW;
    m_pllOption0 = DPIO2_PLL_OPTION_HIGH;
    break;

  case (-2):
    m_pllOption1 = DPIO2_PLL_OPTION_MID;
    m_pllOption0 = DPIO2_PLL_OPTION_LOW;
    break;

  case (0):
    m_pllOption1 = DPIO2_PLL_OPTION_MID;
    m_pllOption0 = DPIO2_PLL_OPTION_MID;
    break;

  case (2):
    m_pllOption1 = DPIO2_PLL_OPTION_MID;
    m_pllOption0 = DPIO2_PLL_OPTION_HIGH;
    break;

  case (4):
    m_pllOption1 = DPIO2_PLL_OPTION_HIGH;
    m_pllOption0 = DPIO2_PLL_OPTION_LOW;
    break;

  case (6):
    m_pllOption1 = DPIO2_PLL_OPTION_HIGH;
    m_pllOption0 = DPIO2_PLL_OPTION_MID;
    break;

  default:
    DPIO2_ERROR_MSG(("%d is not a valid value for Strobe Skew\n", skewInTimingUnits));
    return (ERROR);
    break;

  }


  m_useDefaultStrobeSkew = FALSE;
  

  configurePllOptions(pController);


  return (OK);

}




/*meh*/
STATUS CDpio2_66MHzPcbBState::setStrobeSkew(CDpio2DriverController* pController,
                                            int skewInTimingUnits)
{

  /* Determine the PLL options that correspond to the specified skew value.
   */
  switch (skewInTimingUnits) {

  case (-4):
    m_pllOption1 = DPIO2_PLL_OPTION_LOW;
    m_pllOption0 = DPIO2_PLL_OPTION_LOW;
    break;

  case (-3):
    m_pllOption1 = DPIO2_PLL_OPTION_LOW;
    m_pllOption0 = DPIO2_PLL_OPTION_MID;
    break;

  case (-2):
    m_pllOption1 = DPIO2_PLL_OPTION_LOW;
    m_pllOption0 = DPIO2_PLL_OPTION_HIGH;
    break;

  case (-1):
    m_pllOption1 = DPIO2_PLL_OPTION_MID;
    m_pllOption0 = DPIO2_PLL_OPTION_LOW;
    break;

  case (0):
    m_pllOption1 = DPIO2_PLL_OPTION_MID;
    m_pllOption0 = DPIO2_PLL_OPTION_MID;
    break;

  case (1):
    m_pllOption1 = DPIO2_PLL_OPTION_MID;
    m_pllOption0 = DPIO2_PLL_OPTION_HIGH;
    break;

  case (2):
    m_pllOption1 = DPIO2_PLL_OPTION_HIGH;
    m_pllOption0 = DPIO2_PLL_OPTION_LOW;
    break;

  case (3):
    m_pllOption1 = DPIO2_PLL_OPTION_HIGH;
    m_pllOption0 = DPIO2_PLL_OPTION_MID;
    break;

  case (4):
    m_pllOption1 = DPIO2_PLL_OPTION_HIGH;
    m_pllOption0 = DPIO2_PLL_OPTION_HIGH;
    break;

  default:
    DPIO2_ERROR_MSG(("%d is not a valid value for Strobe Skew\n", skewInTimingUnits));
    return (ERROR);
    break;

  }


  m_useDefaultStrobeSkew = FALSE;
  

  configurePllOptions(pController);


  return (OK);

}







STATUS CDpio2PcbBState::useDefaultStrobeSkew(CDpio2DriverController* pController)
{

  m_useDefaultStrobeSkew = TRUE;

  configurePllOptions(pController);

  return (OK);

}



void CDpio2PcbBFpdpInputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {

    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

      } else {

        /* The PLL options are frequency dependent 
         * when a LVDS input is receiving the strobe. 
         */
        switch (pController->m_strobeFrequencyRange) {

        case BELOW_OR_EQUAL_TO_5MHZ:
        case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
        case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ:
        case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ:
          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ:
        case ABOVE_100MHZ:
          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_HIGH);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        default:
          DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
                     pController->m_strobeFrequencyRange), TRUE);
          break;

        }
      
      }

    }  

  }

}  


void CDpio2PcbBFpdpOutputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( !m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);
        
      } else {

        /* The PLL options are frequency dependent 
         * when a FPDP output is generating the strobe. 
         */
        switch (pController->m_strobeFrequencyRange) {

        case BELOW_OR_EQUAL_TO_5MHZ:
        case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
        case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ:
        case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ:
          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ:
        case ABOVE_100MHZ:
          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_HIGH);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        default:
          DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
                     pController->m_strobeFrequencyRange), TRUE);
          break;

        }

      }

    }

  }

}


void CDpio2PcbBLvdsInputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

      } else {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_HIGH);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
      
      }

    }

  }

}


void CDpio2PcbBLvdsOutputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      /* The default skew for an LVDS output module using double clocking is zero,
       * regardless of whether it is generating the strobe signal or not.
       */
      pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
      pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

    }

  }

}


void CDpio2PcbBPeclInputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options of the main board.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

      } else {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_HIGH);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

      }    

    }  

  }


  /* Determine how to set the PLL options of the personality module.
   */
  switch ( pController->m_strobeFrequencyRange ) {

  case BELOW_OR_EQUAL_TO_5MHZ:
  case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
    DPIO2_WARNING_MSG(("PLL on the PECL personality module should be disabled\n"), TRUE);
    DPIO2_WARNING_MSG(("(by jumper) when strobe frequency is below 10MHz\n"), TRUE);

  case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ:
    
    pFrontEnd->setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION_LOW);
    break;

  case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ:

    pFrontEnd->setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION_MID);
    break;

  case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ:
  case ABOVE_100MHZ:

    pFrontEnd->setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION_HIGH);
    break;

  default:
    DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
               pController->m_strobeFrequencyRange), TRUE);
    break;

  }
  


}  


void CDpio2PcbBPeclOutputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      /* The default skew for a PECL output module using double clocking is zero,
       * regardless of whether it is generating the strobe signal or not.
       */
      pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
      pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);
        
    }

  }

}



void CDpio2PcbBRs422InputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options of the main board.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

      } else {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

      }

    }

  }


  /* Determine how to set the PLL options of the personality module.
   */
  switch ( pController->m_strobeFrequencyRange ) {

  case BELOW_OR_EQUAL_TO_5MHZ:
  case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
    DPIO2_WARNING_MSG(("PLL on the RS422 personality module should be disabled\n"), TRUE);
    DPIO2_WARNING_MSG(("(by jumper) when strobe frequency is below 10MHz\n"), TRUE);

  case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ:
    
    pFrontEnd->setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION_LOW);
    break;

  case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ:

    pFrontEnd->setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION_MID);
    break;

  case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ:
  case ABOVE_100MHZ:

    pFrontEnd->setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION_HIGH);
    break;

  default:
    DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
               pController->m_strobeFrequencyRange), TRUE);
    break;

  }

}  


void CDpio2PcbBRs422OutputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      /* The default skew for an RS422 output module using double clocking is zero,
       * regardless of whether it is generating the strobe signal or not.
       */
      pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
      pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);
        
    }

  }

}




/*meh*/
void CDpio2_66MHzPcbBFpdpInputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

  } else {

    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

      } else {

        /* The PLL options are frequency dependent 
         * when a LVDS input is receiving the strobe. 
         */
        switch (pController->m_strobeFrequencyRange) {

        case BELOW_OR_EQUAL_TO_5MHZ:
        case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
        case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ:
        case ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ:
        case ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ:  
        case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ:
        case ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ:
        case ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ:
        case ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ:
        case ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ:

          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ:
        case ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ:
        case ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ:
        case ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ:
        case ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ:
        case ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ:
        case ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ:
        case ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ:
        case ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ:
        case ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ:   
        case ABOVE_100MHZ:

          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        default:
          DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
                     pController->m_strobeFrequencyRange), TRUE);
          break;

        }
      
      }

    }  

  }

}  


void CDpio2_66MHzPcbBFpdpOutputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( !m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);
        
      } else {

        /* The PLL options are frequency dependent 
         * when a FPDP output is generating the strobe. 
         */
        switch (pController->m_strobeFrequencyRange) {

        case BELOW_OR_EQUAL_TO_5MHZ:
        case ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ:
        case ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ:
        case ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ:
        case ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ:  
        case ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ:
        case ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ:
        case ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ:
        case ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ:
        case ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ:

          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        case ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ:
        case ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ:
        case ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ:
        case ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ:
        case ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ:
        case ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ:
        case ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ:
        case ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ:
        case ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ:
        case ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ:   
        case ABOVE_100MHZ:

          pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
          pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
          break;

        default:
          DPIO2_WARNING_MSG(("Warning: Unknown frequency range (%d)\n", 
                     pController->m_strobeFrequencyRange), TRUE);
          break;

        }

      }

    }

  }

}


void CDpio2_66MHzPcbBLvdsInputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      if ( m_moduleIsGeneratingStrobe ) {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

      } else {

        pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_LOW);
        pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_LOW);
      
      }

    }

  }

}


void CDpio2_66MHzPcbBLvdsOutputState::configurePllOptions(CDpio2DriverController* pController)
{

  CDpio2FrontEnd*   pFrontEnd;


  pFrontEnd = pController->m_pHwInterface->getFrontEnd();


  /* Determine how to set the PLL options.
   */
  if ( !m_clockingOnBothStrobeEdgesIsEnabled ) {

    pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
    pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

  } else {
      
    if ( !m_useDefaultStrobeSkew ) {

      pFrontEnd->setPllOption0(m_pllOption0);
      pFrontEnd->setPllOption1(m_pllOption1);

    } else {

      /* The default skew for an LVDS output module using double clocking is zero,
       * regardless of whether it is generating the strobe signal or not.
       */
      pFrontEnd->setPllOption0(DPIO2_PLL_OPTION_MID);
      pFrontEnd->setPllOption1(DPIO2_PLL_OPTION_MID);

    }

  }

}



STATUS CDpio2PersonalityState::enableStrobeGeneration(CDpio2DriverController* pController)
{

  /* By default a DPIO2 shall use the programmable oscillator
   * to generate the strobe signals.
   */
  pController->m_pFrontEnd->activateProgrammableOscillator();

  
  pController->m_pFrontEnd->enableStrobeGeneration();

  
  /* Update the configuration of FIFO frequency and PLL.
   */
  pController->configureFifoFrequency();
  pController->configurePll();


  return (OK);

}


STATUS CDpio2PersonalityState::disableStrobeGeneration(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->disableStrobeGeneration();

  
  /* By default a DPIO2 shall use the primary strobe
   * when the module itself doesn't generate the strobe signals.
   */
  pController->m_pFrontEnd->selectPrimaryStrobe();


  /* Update the configuration of FIFO frequency and PLL.
   */
  pController->configureFifoFrequency();
  pController->configurePll();


  return (OK);

}


STATUS CDpio2PersonalityState::selectSecondaryStrobe(CDpio2DriverController* pController)
{

  pController->m_pFrontEnd->selectSecondaryStrobe();
  
  return (OK);

}


STATUS CDpio2PeclPersonalityState::enableStrobeGeneration(CDpio2DriverController* pController)
{

  DPIO2_WARNING_MSG(("NOTE: Jumper on PECL personality module must be set to enable Strobe Generation\n"),
                    TRUE);
            
  return (CDpio2PersonalityState::enableStrobeGeneration(pController));

}


STATUS CDpio2PeclPersonalityState::disableStrobeGeneration(CDpio2DriverController* pController)
{

  DPIO2_WARNING_MSG(("NOTE: Jumper on PECL personality module must be set to disable Strobe Generation\n"),
                    TRUE);
            
  return (CDpio2PersonalityState::disableStrobeGeneration(pController));

}


STATUS CDpio2LvdsPersonalityState::selectSecondaryStrobe(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("No Secondary Strobe signal is available on LVDS modules"));
  
  /* compiler quiet */
  pController       = pController;

  return (ERROR);

}


STATUS CDpio2PeclPersonalityState::selectSecondaryStrobe(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("No Secondary Strobe signal is available on PECL modules"));
  
  /* compiler quiet */
  pController       = pController;

  return (ERROR);

}


STATUS CDpio2Rs422PersonalityState::selectSecondaryStrobe(CDpio2DriverController* pController)
{

  DPIO2_ERROR_MSG(("No Secondary Strobe signal is available on RS422 modules"));
  
  /* compiler quiet */
  pController       = pController;

  return (ERROR);

}
