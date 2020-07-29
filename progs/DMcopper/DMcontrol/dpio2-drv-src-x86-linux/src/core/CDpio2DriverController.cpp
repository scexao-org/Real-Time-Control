/****************************************************************************
Module      : CDpio2DriverController.cpp

Description : Source file defining the CDpio2DriverController class.
              A CDpio2DriverController object represents all available
              functionality of one DPIO2 module, and controls that 
              direction dependent functionlity is used correctly.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  02b,06jul2004.meh  Made changes necessary to compile file under VMOSA.
  02a,13feb2004,meh  Support new Strobe Frequency Range.
  01z,22jan2004,meh  Added support for _66_pcb_B
  01y,16dec2003,meh  Change getProgrammedFrequency to getFrequency.
  01x,01oct2003,meh  Added the global variables dpio2ErrorMessageEnable and
                     dpio2WarningMessageEnable.
                     Changed the DEBUG_MSG with TRUE flag to
                     DPIO2_WARNING_MSG.
                     Changed the ERROR_MSG whitch not is rigth
                     infront of a throw to DPIO2_ERROR_MSG.
  01w,02oct2002,nib  Renamed TTL and PECL strobe to primary and secondary strobe.
  01v,19sep2002,nib  Added the setTestPatternStartValue() method.
  01u,13sep2002,nib  Added the getTransferredByteCount(), enableEotSyncCount(),
                     and disableEotSyncCount() methods.
  01t,22may2002,nib  Added support for PECL and RS422 modules.
  01s,08apr2002,nib  Added flushing of internal FIFOs in the Front End in flushFifo().
  01r,18mar2002,nib  Updated file for use in a MCOS environment.
  01q,22feb2002,nib  Added methods for controlling forced assertion of SUSPEND.
  01p,19feb2002,nib  Added support for test pattern generation.
  01o,12feb2002,nib  Added the setSyncGenerationCounter() method.
  01n,23jan2002,nib  Changed the constructor to create PLL control objects.
  01m,04jan2002,nib  Changed configurePll() to its corresponding method
                     in the HW Version State object.
  01l,13dec2001,nib  Added methods for controlling DMA FIFO flushing on stop
                     - enableDmaFifoFlushOnStop() and disableDmaFifoFlushOnStop()
  01k,20nov2001,nib  Added method for getting the base address of the DPIO2 FLASH
                     - getBaseAddressOfFlash(). 
  01j,31oct2001,nib  Added method for reading the number of bytes in the descriptor
                     that were not transferred - getRemainingByteCount(). 
  01i,31oct2001,nib  Added methods for counter addressing
                     - enableCounterAddressing() and disableCounterAddressing()
  01h,28oct2001,nib  Updated file to compile in a Windows environment.
  01g,04oct2001,nib  Added disabling of interrupt sources and removal of interrupt
                     actions in the desctructor.
  01f,03oct2001,nib  Renamed m_pDataDirectionState to m_pDataDirectionState.
                     Added m_pDataPackingState, initialization of this pointer 
                     in the constructor and deletion in the destructor.
  01e,02oct2001,nib  Added methods; enableDvalidAlwaysAsserted()
                     and disableDvalidAlwaysAsserted().
  01d,01oct2001,nib  Replaced enablePacking() and disablePacking() with setPackingMode().
                     Changed configureFifoFrequency() to use getPackingMode() to
                     determine whether packing is enabled or not.
  01c,24sep2001,nib  Added enableD0ToBeUsedForSync() and disableD0ToBeUsedForSync()
  01b,19jul2001,nib  Updated setDataSwapMode() to use CFrontEnd::selectDataSwapMode().
  01a,22jun2001,nib  Created.

*****************************************************************************/

#include "CDpio2DriverController.h"
#include "CDpio2DriverState.h"
#include "CDpio2Leds.h"
#include "CDpio2Oscillator.h"

#if defined(WIN32)

#include "CWDpio2HwInterface.h"
#include "CWInterruptManager.h"

#elif defined(MCOS)

#include "CMDpio2HwInterface.h"
#include "CMInterruptManager.h"

#elif defined(VMOSA)

#include "vmosaCDpio2HwInterface.h"
#include "vmosaCInterruptManager.h"

#else /* VXWORKS */

#include "CDpio2HwInterface.h"
#include "CInterruptManager.h"

#endif



#if 0 /*meh*/
#define MARK printf ("Was Here %s,%d\n", __FILE__, __LINE__);\
             vmosa_mdelay (100);
#endif




/* Global */
#ifdef __cplusplus
extern "C" {
#endif

  BOOL dpio2ErrorMessageEnable = TRUE;
  BOOL dpio2WarningMessageEnable = TRUE;

#ifdef __cplusplus
}
#endif




//##ModelId=3A8247E50294
CDpio2DriverController::CDpio2DriverController(CDpio2HwInterface* pHwInterface)
{

  UINT8   revisionId;

  BOOL   moduleIsConfiguredAsOutput;

  UINT8   personalityModuleId;
  UINT8   mainBoardId;

  int   moduleType;

  UINT16   deviceId;


  m_pHwInterface = pHwInterface;

  /* Get pointer to all the aggregate objects of the HW interface object.
   */
  m_pPrimaryDmaController = m_pHwInterface->getPrimaryDmaController();
  m_pFifo                 = m_pHwInterface->getFifo();
  m_pFrontEnd             = m_pHwInterface->getFrontEnd();
  m_pOscillator           = m_pHwInterface->getOscillator();
  m_pLeds                 = m_pHwInterface->getLeds();
  m_pInterruptManager     = m_pHwInterface->getInterruptManager();


  moduleIsConfiguredAsOutput = m_pHwInterface->checkIsOutput();


  /* Determine the DPIO2 device type.
   */ 
  personalityModuleId = m_pFrontEnd->getPersonalityId();
  mainBoardId = m_pFrontEnd->getMainBoardId();

  moduleType = (mainBoardId << 4) | personalityModuleId;


  /* Create the state objects.
   */
  if ( moduleIsConfiguredAsOutput ) {

    m_pDataDirectionState = new CDpio2DriverOutputState;

  } else {

    m_pDataDirectionState = new CDpio2DriverInputState;
    
  }

  if ( m_pFrontEnd->checkIs8Bit4BitPackingAvailable() ) {

    m_pDataPackingState = new CDpio2Packing8Bit4BitState;

  } else {
    
    m_pDataPackingState = new CDpio2Packing16Bit10BitState;

  }


  if ( m_pHwInterface->getDeviceId(deviceId) != OK ) {

    ERROR_MSG (("getDeviceId\n"));
    throw;

  } else {

    if ( m_pHwInterface->getRevisionId(revisionId) != OK ) {

      ERROR_MSG (("getRevisionId\n"));
      throw;

    } else {


      if ( (deviceId == PCI_DEVICE_ID_DPIO2)
           && (revisionId == DPIO2_REVISION_ID_PCB_A) ) {


        m_pPllControlState = new CDpio2PcbAState;
    

      } else if ( ( (deviceId == PCI_DEVICE_ID_DPIO2) &&
                    (revisionId == DPIO2_REVISION_ID_PCB_B) ) ||
                  ( (deviceId == PCI_DEVICE_ID_DPIO2_66MHZ) &&
                    (revisionId == DPIO2_REVISION_ID_PCB_A) ) ) {


        switch (moduleType) {
          
        case DPIO2_LVDS_MODULE:
          if ( moduleIsConfiguredAsOutput ) {
      
            m_pPllControlState = new CDpio2PcbBLvdsOutputState;

          } else {

            m_pPllControlState = new CDpio2PcbBLvdsInputState;

          }
          break;


        case DPIO2_PECL_INPUT_MODULE:
          if ( moduleIsConfiguredAsOutput ) {
      
            ERROR_MSG(("PECL input module is configured for output\n"));
            throw;

          } 

          m_pPllControlState = new CDpio2PcbBPeclInputState;
          break;


        case DPIO2_PECL_OUTPUT_MODULE:
          if ( !moduleIsConfiguredAsOutput ) {
      
            ERROR_MSG(("PECL output module is configured for input\n"));
            throw;

          } 

          m_pPllControlState = new CDpio2PcbBPeclOutputState;
          break;


        case DPIO2_RS422_INPUT_MODULE:
          if ( moduleIsConfiguredAsOutput ) {
      
            ERROR_MSG(("RS422 input module is configured for output\n"));
            throw;

          } 

          m_pPllControlState = new CDpio2PcbBRs422InputState;
          break;


        case DPIO2_RS422_OUTPUT_MODULE:
          if ( !moduleIsConfiguredAsOutput ) {
            
            ERROR_MSG(("RS422 output module is configured for input\n"));
            throw;

          } 

          m_pPllControlState = new CDpio2PcbBRs422OutputState;
          break;


        default:
          DPIO2_WARNING_MSG(("Warning: Unknown Module Type (0x%02X) - assuming FPDP\n", moduleType), TRUE);

        case DPIO2_FPDP_MODULE:
          if ( moduleIsConfiguredAsOutput ) {
      
            m_pPllControlState = new CDpio2PcbBFpdpOutputState;

          } else {

            m_pPllControlState = new CDpio2PcbBFpdpInputState;

          }
          break;

        }

        /*( ( (deviceId == PCI_DEVICE_ID_DPIO2) &&
          (revisionId == DPIO2_REVISION_ID_PCB_B) ) ||
          ( (deviceId == PCI_DEVICE_ID_DPIO2_66MHZ) &&
          (revisionId == DPIO2_REVISION_ID_PCB_A) ) ) */

      
      } else if ( (deviceId == PCI_DEVICE_ID_DPIO2_66MHZ) &&       /*meh*/
                  (revisionId == DPIO2_REVISION_ID_PCB_B) ) {


        switch (moduleType) {
        
        case DPIO2_LVDS_MODULE:
          if ( moduleIsConfiguredAsOutput ) {
            
            m_pPllControlState = new CDpio2_66MHzPcbBLvdsOutputState;
            
          } else {

            m_pPllControlState = new CDpio2_66MHzPcbBLvdsInputState;
            
          }
          break;
          

        case DPIO2_FPDP_MODULE:
          if ( moduleIsConfiguredAsOutput ) {
            
            m_pPllControlState = new CDpio2_66MHzPcbBFpdpOutputState;

          } else {

            m_pPllControlState = new CDpio2_66MHzPcbBFpdpInputState;

          }
          break;

          
        case DPIO2_PECL_INPUT_MODULE:
          
          ERROR_MSG(("PECL is not supported with 66MHz\n"));
          throw;

          break;


        case DPIO2_PECL_OUTPUT_MODULE:
       
          ERROR_MSG(("PECL is not supported with 66MHz\n"));
          throw;

          break;


        case DPIO2_RS422_INPUT_MODULE:

          ERROR_MSG(("RS422 is not supported with 66MHz\n"));
          throw;

          break;


        case DPIO2_RS422_OUTPUT_MODULE:

          ERROR_MSG(("RS422 is not supported with 66MHz\n"));
          throw;
          
          break;


        default:
          DPIO2_WARNING_MSG(("Warning: Unknown Module Type (0x%02X) - assuming FPDP\n", moduleType), TRUE);

             
        } /* switch */
        /*meh*/
        
      } /* ( (deviceId == PCI_DEVICE_ID_DPIO2_66MHZ) &&
           (revisionId == DPIO2_REVISION_ID_PCB_B) ) */

    }

  }


  /* Create personality state object.
   */
  switch (moduleType) {

  case DPIO2_LVDS_MODULE:
    m_pPersonalityState = new CDpio2LvdsPersonalityState;
    break;

  case DPIO2_PECL_INPUT_MODULE:
  case DPIO2_PECL_OUTPUT_MODULE:
    m_pPersonalityState = new CDpio2PeclPersonalityState;
    break;
   
  case DPIO2_RS422_INPUT_MODULE:
  case DPIO2_RS422_OUTPUT_MODULE:
    m_pPersonalityState = new CDpio2Rs422PersonalityState;
    break;

  default:
    DPIO2_WARNING_MSG(("Warning: Unknown Module Type (0x%02X) - assuming FPDP\n", moduleType), TRUE);

  case DPIO2_FPDP_MODULE:
    m_pPersonalityState = new CDpio2FpdpPersonalityState;
    break;

  }


  /* Initialize the DPIO2.
   */
  initialize();

}


//##ModelId=3A8247E502D0
CDpio2DriverController::~CDpio2DriverController()
{

  m_pInterruptManager->disableAllInterruptSources();
  m_pInterruptManager->removeAllInterruptActions();

  delete m_pDataDirectionState;
  delete m_pDataPackingState;
  delete m_pPllControlState;
  delete m_pPersonalityState;

}


//##ModelId=3AE6892201C2
void CDpio2DriverController::initialize()
{

  CDpio2Oscillator*   pOscillator;

  UINT32   defaultFrequency;


  /* Initialize the frequency range attribute using the default
   * frequency of the programmable oscillator.
   */
  pOscillator = m_pHwInterface->getOscillator();
  defaultFrequency = pOscillator->getFrequency();
  updateStrobeFrequencyRange(defaultFrequency);


  /* By default the Strobe is assumed to be free running.
   */
  m_inputStrobeIsAsynchronous = FALSE;


  /* Initialize the PLL and FIFO frequency.
   */ 
  configureFifoFrequency();
  configurePll();

}


//##ModelId=3AED589403DE
STATUS CDpio2DriverController::addInterruptAction(DPIO2_INTERRUPT_SOURCE_ID sourceId,
                                                  CAction* pAction)
{

  return (m_pInterruptManager->addInterruptAction((int) sourceId, pAction));

}


//##ModelId=3AED58950172
STATUS CDpio2DriverController::removeInterruptAction(DPIO2_INTERRUPT_SOURCE_ID sourceId,
                                                     CAction* pAction)
{

  return (m_pInterruptManager->removeInterruptAction((int) sourceId, pAction));

}


//##ModelId=3AED589502F8
STATUS CDpio2DriverController::enableInterruptSource(DPIO2_INTERRUPT_SOURCE_ID sourceId)
{

  return (m_pInterruptManager->enableInterruptSource((int) sourceId));

}


//##ModelId=3AED589503D4
STATUS CDpio2DriverController::disableInterruptSource(DPIO2_INTERRUPT_SOURCE_ID sourceId)
{

  return (m_pInterruptManager->disableInterruptSource((int) sourceId));

}


void CDpio2DriverController::disableAllInterruptSources()
{

  m_pInterruptManager->disableAllInterruptSources();

}


STATUS CDpio2DriverController::checkIsInterruptSourceEnabled(DPIO2_INTERRUPT_SOURCE_ID sourceId, 
                                                             BOOL& enabledFlag)
{

  return (m_pInterruptManager->checkIsInterruptSourceEnabled((int) sourceId, enabledFlag));

}


STATUS CDpio2DriverController::checkIsInterruptSourceActive(DPIO2_INTERRUPT_SOURCE_ID sourceId, 
                                                            BOOL& activeFlag)
{
  
  return (m_pInterruptManager->checkIsInterruptSourceActive((int) sourceId, activeFlag));
 
}


UINT32 CDpio2DriverController::getFifoSize()
{

  return (m_pFifo->getFifoSize());

}


DPIO_FIFO_STATE CDpio2DriverController::getFifoState()
{

  return (m_pFifo->getFifoState());

}


void CDpio2DriverController::flushFifo()
{

  /* Hold internal FIFOs in Front End in reset,
   * while flushing the external FIFO and the FIFOs in the PCI interface.
   * to ensure that all data are flushed.
   */
  m_pFrontEnd->activateInternalFifoReset();

  m_pFifo->flushFifo();

  m_pFrontEnd->deactivateInternalFifoReset();

}


void CDpio2DriverController::resetFifoHistoryFlags()
{

  m_pFifo->resetHistoryFlags();

}


//##ModelId=3A9A7DF80208
BOOL CDpio2DriverController::checkHasFifoOverflowed() 
{

  return ((m_pHwInterface->getFifo())->checkHasOverflowed());

}


BOOL CDpio2DriverController::checkHasFifoBeenFull()
{

  return (m_pFifo->checkHasBeenFull());

}


//##ModelId=3A9A7DF8019A
BOOL CDpio2DriverController::checkHasFifoBeenAlmostFull()
{
  
  return (m_pFifo->checkHasBeenAlmostFull());

}


BOOL CDpio2DriverController::checkHasFifoBeenMoreThanHalfFull()
{

  return (m_pFifo->checkHasBeenMoreThanHalfFull());

}


//##ModelId=3A9A7DF8015E
BOOL CDpio2DriverController::checkHasFifoBeenAlmostEmpty() 
{

  return (m_pFifo->checkHasBeenAlmostEmpty());

}


BOOL CDpio2DriverController::checkHasFifoBeenEmpty() 
{

  return (m_pFifo->checkHasBeenEmpty());

}


void CDpio2DriverController::enableSuspendFlowControl()
{

  m_pFifo->enableSuspendFlowControl();

}


void CDpio2DriverController::disableSuspendFlowControl()
{

  m_pFifo->disableSuspendFlowControl();

}


void CDpio2DriverController::enableNrdyFlowControl()
{
  
  m_pFifo->enableNrdyFlowControl();

}


void CDpio2DriverController::disableNrdyFlowControl()
{
  
  m_pFifo->disableNrdyFlowControl();

}


STATUS CDpio2DriverController::setInputDmaDescriptor(UINT32 descriptorId,
						     const CDpio2InputDmaDescriptor* pDescriptor)
{

  return (m_pDataDirectionState->setInputDmaDescriptor(this, descriptorId, pDescriptor));

}


STATUS CDpio2DriverController::setOutputDmaDescriptor(UINT32 descriptorId,
						      const CDpio2OutputDmaDescriptor* pDescriptor)
{

  return (m_pDataDirectionState->setOutputDmaDescriptor(this, descriptorId, pDescriptor));

}


STATUS CDpio2DriverController::setNextDescriptorInDmaChain(UINT32 descriptorId)
{

  return (m_pPrimaryDmaController->setNextDescriptorInDmaChain(descriptorId));

}


UINT32 CDpio2DriverController::getNextDescriptorInDmaChain()
{

  return (m_pPrimaryDmaController->getNextDescriptorInDmaChain());

}


void CDpio2DriverController::startDmaTransfer()
{

  m_pPrimaryDmaController->startDmaTransfer();

}


void CDpio2DriverController::stopDmaTransfer()
{

  m_pPrimaryDmaController->stopDmaTransfer();

}


void CDpio2DriverController::enableDmaFifoFlushOnStop()
{

  m_pPrimaryDmaController->enableClearingOnStop();

}


void CDpio2DriverController::disableDmaFifoFlushOnStop()
{

  m_pPrimaryDmaController->disableClearingOnStop();

}


void CDpio2DriverController::suspendDmaTransfer()
{

  m_pPrimaryDmaController->suspendDmaTransfer();

}


void CDpio2DriverController::resumeDmaTransfer()
{

  m_pPrimaryDmaController->resumeDmaTransfer();

}


BOOL CDpio2DriverController::checkIsDmaTransferCompleted()
{

  return (m_pPrimaryDmaController->checkIsChainCompleted());

}


BOOL CDpio2DriverController::checkIsDmaTransferSuspended()
{

  return (m_pPrimaryDmaController->checkIsDmaTransferSuspended());

}


void CDpio2DriverController::enableDmaDemandMode()
{
  
  m_pPrimaryDmaController->enableDmaDemandMode();

}
 

void CDpio2DriverController::disableDmaDemandMode()
{
  
  m_pPrimaryDmaController->disableDmaDemandMode();

}
 

BOOL CDpio2DriverController::checkIsDmaDemandModeEnabled()
{
  
  return (m_pPrimaryDmaController->checkIsDmaDemandModeEnabled());

}


STATUS CDpio2DriverController::selectEndOfTransferMode(DPIO2_END_OF_TRANSFER_MODE mode)
{

  return (m_pDataDirectionState->selectEndOfTransferMode(this, mode));

}


STATUS CDpio2DriverController::enableEndOfTransfer()
{

  return (m_pDataDirectionState->enableEndOfTransfer(this));

}


STATUS CDpio2DriverController::disableEndOfTransfer()
{

  return (m_pDataDirectionState->disableEndOfTransfer(this));

}


STATUS CDpio2DriverController::enableEotSyncCount(UINT16 countValue)
{

  return (m_pDataDirectionState->enableEotSyncCount(this, countValue));

}


STATUS CDpio2DriverController::disableEotSyncCount()
{

  return (m_pDataDirectionState->disableEotSyncCount(this));

}


STATUS CDpio2DriverController::enableContinueOnEndOfTransfer()
{

  return (m_pDataDirectionState->enableContinueOnEndOfTransfer(this));

}


STATUS CDpio2DriverController::disableContinueOnEndOfTransfer()
{

  return (m_pDataDirectionState->disableContinueOnEndOfTransfer(this));

}


void CDpio2DriverController::getRemainingByteCount(UINT32& byteCount, 
                                                   BOOL& byteCountHasOverflowed)
{

  m_pPrimaryDmaController->getRemainingByteCount(byteCount, byteCountHasOverflowed);

}


void CDpio2DriverController::getTransferredByteCount(UINT32& byteCount)
{

  m_pPrimaryDmaController->getTransferredByteCount(byteCount);

}


void CDpio2DriverController::setDmaPciReadCommand(UINT32 command)
{

  m_pPrimaryDmaController->setPciReadCommand(command);

}


void CDpio2DriverController::setDmaPciWriteCommand(UINT32 command)
{

  m_pPrimaryDmaController->setPciWriteCommand(command);

}


void CDpio2DriverController::enableFrontEndInterrupts()
{

  m_pFrontEnd->enableFrontEndInterrupts();

}


void CDpio2DriverController::disableFrontEndInterrupts()
{

  m_pFrontEnd->disableFrontEndInterrupts();

}


BOOL CDpio2DriverController::checkAreFrontEndInterruptsEnabled()
{

  return (m_pFrontEnd->checkAreFrontEndInterruptsEnabled());

}


STATUS CDpio2DriverController::enableStrobeGeneration()
{

  return (m_pPersonalityState->enableStrobeGeneration(this));

}


STATUS CDpio2DriverController::disableStrobeGeneration() 
{
      
  return (m_pPersonalityState->disableStrobeGeneration(this));

}


BOOL CDpio2DriverController::checkIsStrobeGenerationEnabled() 
{

  return (m_pFrontEnd->checkIsStrobeGenerationEnabled());

}


//##ModelId=3AED589601A4
void CDpio2DriverController::enableMasterInterface()
{

  m_pFrontEnd->enableMasterInterface();

}


//##ModelId=3AED589601D6
void CDpio2DriverController::disableMasterInterface()
{

  m_pFrontEnd->disableMasterInterface();

}


void CDpio2DriverController::startTransfer()
{

  m_pFrontEnd->startTransfer();

}


void CDpio2DriverController::stopTransfer()
{

  m_pFrontEnd->stopTransfer();

}


STATUS CDpio2DriverController::selectFreeRunningClock()
{

  STATUS   status;

  /* Refuse to perform this request if the module itself is generating the strobe.
   */
  if ( m_pFrontEnd->checkIsStrobeGenerationEnabled() ) {

    DPIO2_ERROR_MSG(("Cannot select Clock Mode, because module is generating strobe.\n"));

    status = ERROR;

  } else {

    m_inputStrobeIsAsynchronous = FALSE;

    DPIO2_WARNING_MSG(("Free Running Clock is selected.\n"), TRUE);

    /* Update the configuration of FIFO frequency and PLL.
     */
    configureFifoFrequency();
    configurePll();
    
    status = OK;

  }

  return (status);

}


STATUS CDpio2DriverController::selectAsynchronousClock()
{

  STATUS   status;


  /* Refuse to perform this request if the module itself is generating the strobe.
   */
  if ( m_pFrontEnd->checkIsStrobeGenerationEnabled() ) {

    DPIO2_ERROR_MSG(("Cannot select Clock Mode, because module is generating strobe.\n"));

    status = ERROR;

  } else {

    m_inputStrobeIsAsynchronous = TRUE;

    DPIO2_WARNING_MSG(("Asynchronous Clock is selected.\n"), TRUE);

    /* Update the configuration of FIFO frequency and PLL.
     */
    configureFifoFrequency();
    configurePll();

    status = OK;

  }

  return (status);

}


STATUS CDpio2DriverController::selectPrimaryStrobe()
{

  STATUS   status;


  /* Refuse to perform this request if the module itself is generating the strobe.
   */
  if ( m_pFrontEnd->checkIsStrobeGenerationEnabled() ) {

    DPIO2_ERROR_MSG(("Cannot select input strobe, because module is generating strobe.\n"));

    status = ERROR;

  } else {

    m_pFrontEnd->selectPrimaryStrobe();

    status = OK;

  }

  return (status);

}


STATUS CDpio2DriverController::selectSecondaryStrobe()
{

  STATUS   status;


  /* Refuse to perform this request if the module itself is generating the strobe.
   */
  if ( m_pFrontEnd->checkIsStrobeGenerationEnabled() ) {

    DPIO2_ERROR_MSG(("Cannot select input strobe, because module is generating strobe.\n"));

    status = ERROR;

  } else {

    status = m_pPersonalityState->selectSecondaryStrobe(this);

  }

  return (status);

}


void CDpio2DriverController::enableRes1AsOutput()
{

  m_pFrontEnd->enableRes1AsOutput();

}


void CDpio2DriverController::disableRes1AsOutput()
{

  m_pFrontEnd->disableRes1AsOutput();

}


void CDpio2DriverController::setRes1OutputHigh()
{

  m_pFrontEnd->setRes1OutputHigh();

}


void CDpio2DriverController::setRes1OutputLow()
{

  m_pFrontEnd->setRes1OutputLow();

}


BOOL CDpio2DriverController::checkIsRes1InputHigh()
{

  return (m_pFrontEnd->checkIsRes1InputHigh());

}


void CDpio2DriverController::enableRes2AsOutput()
{

  m_pFrontEnd->enableRes2AsOutput();

}


void CDpio2DriverController::disableRes2AsOutput()
{

  m_pFrontEnd->disableRes2AsOutput();

}


void CDpio2DriverController::setRes2OutputHigh()
{

  m_pFrontEnd->setRes2OutputHigh();

}


void CDpio2DriverController::setRes2OutputLow()
{

  m_pFrontEnd->setRes2OutputLow();

}


BOOL CDpio2DriverController::checkIsRes2InputHigh()
{

  return (m_pFrontEnd->checkIsRes2InputHigh());

}


void CDpio2DriverController::enableRes3AsOutput()
{

  m_pFrontEnd->enableRes3AsOutput();

}


void CDpio2DriverController::disableRes3AsOutput()
{

  m_pFrontEnd->disableRes3AsOutput();

}


void CDpio2DriverController::setRes3OutputHigh()
{

  m_pFrontEnd->setRes3OutputHigh();

}


void CDpio2DriverController::setRes3OutputLow()
{

  m_pFrontEnd->setRes3OutputLow();

}


BOOL CDpio2DriverController::checkIsRes3InputHigh()
{

  return (m_pFrontEnd->checkIsRes3InputHigh());

}


void CDpio2DriverController::enablePio1AsOutput()
{

  m_pFrontEnd->enablePio1AsOutput();

}


void CDpio2DriverController::disablePio1AsOutput()
{

  m_pFrontEnd->disablePio1AsOutput();

}


void CDpio2DriverController::setPio1OutputHigh()
{

  m_pFrontEnd->setPio1OutputHigh();

}


void CDpio2DriverController::setPio1OutputLow()
{

  m_pFrontEnd->setPio1OutputLow();

}


BOOL CDpio2DriverController::checkIsPio1InputHigh()
{

  return (m_pFrontEnd->checkIsPio1InputHigh());

}


void CDpio2DriverController::enablePio2AsOutput()
{

  m_pFrontEnd->enablePio2AsOutput();

}


void CDpio2DriverController::disablePio2AsOutput()
{

  m_pFrontEnd->disablePio2AsOutput();

}


void CDpio2DriverController::setPio2OutputHigh()
{

  m_pFrontEnd->setPio2OutputHigh();

}


void CDpio2DriverController::setPio2OutputLow()
{

  m_pFrontEnd->setPio2OutputLow();

}


BOOL CDpio2DriverController::checkIsPio2InputHigh()
{

  return (m_pFrontEnd->checkIsPio2InputHigh());

}


BOOL CDpio2DriverController::checkIsDirSignalHigh()
{

  return (m_pFrontEnd->checkIsDirSignalHigh());

}


BOOL CDpio2DriverController::checkIsSuspendAsserted()
{

  return (!m_pFrontEnd->checkIsSuspendSignalHigh());

}


STATUS CDpio2DriverController::setSyncGenerationCounter(UINT32 value)
{

  return (m_pDataDirectionState->setSyncGenerationCounter(this, value));

}


STATUS CDpio2DriverController::selectSyncGenerationMode(DPIO2_SYNC_GENERATION_MODE mode)
{

  return (m_pDataDirectionState->selectSyncGenerationMode(this, mode));

}


STATUS CDpio2DriverController::selectSyncReceptionMode(DPIO2_SYNC_RECEPTION_MODE mode)
{

  return (m_pDataDirectionState->selectSyncReceptionMode(this, mode));

}


void CDpio2DriverController::enableD0ToBeUsedForSync()
{

  m_pFrontEnd->enableD0ForSync();

}


void CDpio2DriverController::disableD0ToBeUsedForSync()
{

  m_pFrontEnd->disableD0ForSync();

}


STATUS CDpio2DriverController::enableVideoMode()
{

  return (m_pDataDirectionState->enableVideoMode(this));

}


STATUS CDpio2DriverController::disableVideoMode()
{

  return (m_pDataDirectionState->disableVideoMode(this));

}


STATUS CDpio2DriverController::enableDvalidAlwaysAsserted()
{

  return (m_pDataDirectionState->enableDvalidAlwaysAsserted(this));

}


STATUS CDpio2DriverController::disableDvalidAlwaysAsserted()
{

  return (m_pDataDirectionState->disableDvalidAlwaysAsserted(this));

}


STATUS CDpio2DriverController::enableCounterAddressing(UINT32 initialSkipCount, 
                                                       UINT32 skipCount, 
                                                       UINT32 receiveCount)
{

  return (m_pDataDirectionState->enableCounterAddressing(this,
                                                         initialSkipCount,
                                                         skipCount,
                                                         receiveCount));

}


STATUS CDpio2DriverController::disableCounterAddressing()
{

  return (m_pDataDirectionState->disableCounterAddressing(this));

}


STATUS CDpio2DriverController::enableTestPatternGeneration(DPIO2_TEST_PATTERN pattern)
{

  return (m_pFrontEnd->enableTestPatternGeneration(pattern));

}


void CDpio2DriverController::disableTestPatternGeneration()
{

  m_pFrontEnd->disableTestPatternGeneration();

}


void CDpio2DriverController::setTestPatternStartValue(UINT16 value)
{

  m_pFrontEnd->setInitialSkipCounter((UINT32) value);

}


STATUS CDpio2DriverController::enableForcedSuspendAssertion()
{

  return (m_pDataDirectionState->enableForcedSuspendAssertion(this));

}


STATUS CDpio2DriverController::disableForcedSuspendAssertion()
{

  return (m_pDataDirectionState->disableForcedSuspendAssertion(this));

}


STATUS CDpio2DriverController::checkIsPipelineNotEmpty(BOOL& flag)
{

  return (m_pDataDirectionState->checkIsPipelineNotEmpty(this, flag));

}


STATUS CDpio2DriverController::flushPackingPipeline()
{

  return (m_pDataDirectionState->flushPackingPipeline(this));

}


//##ModelId=3AE68B82005A
STATUS CDpio2DriverController::setOscillatorFrequency(UINT32 requestedFrequency, 
                                                      UINT32& resultingFrequency)
{

  STATUS   status;


  status = m_pOscillator->setFrequency(requestedFrequency, resultingFrequency);

  if ( status == OK ) {

    m_strobeFrequency = resultingFrequency;

    /* Update the attribute for strobe frequency range, and update the configuration
     * of FIFO frequency and PLL if strobe generation is enabled.
     */
    updateStrobeFrequencyRange(resultingFrequency);

    if ( m_pFrontEnd->checkIsStrobeGenerationEnabled() ) {

      configureFifoFrequency();
      configurePll();

    }

  }


  return (status);

}


UINT32 CDpio2DriverController::getOscillatorFrequency()
{

  return (m_pOscillator->getFrequency());

}


void CDpio2DriverController::activateProgrammableOscillator()
{

  m_pFrontEnd->activateProgrammableOscillator();

}


void CDpio2DriverController::activateFixedOscillator()
{

  m_pFrontEnd->activateFixedOscillator();

}


BOOL CDpio2DriverController::checkIsProgrammableOscillatorActivated()
{

  return (m_pFrontEnd->checkIsProgrammableOscillatorActivated());

}


STATUS CDpio2DriverController::setStrobeFrequencyRange(DPIO2_STROBE_FREQUENCY_RANGE range)
{

  STATUS   status;

  BOOL   programmableOscillatorIsActivated;
  BOOL   moduleIsGeneratingStrobe;



  /* Refuse to update the strobe frequency range ,
   * if the module is generating the strobe using the programmable oscillator.
   */
  programmableOscillatorIsActivated = checkIsProgrammableOscillatorActivated();
  moduleIsGeneratingStrobe = checkIsStrobeGenerationEnabled();

  if ( programmableOscillatorIsActivated && moduleIsGeneratingStrobe ) {

    DPIO2_ERROR_MSG(("Cannot set Strobe Frequency Range"));

    status = ERROR;

  } else {

    if (checkIsStrobeFrequencyRangeValid(range)) {

      m_strobeFrequencyRange = range;


      /* Update the configuration of FIFO frequency and PLL
       */
      configureFifoFrequency();
      configurePll();
      
      status = OK;

    } else { 

      DPIO2_ERROR_MSG(("The given Strobe Frequency Range is not valid"));

      status = ERROR;

    }

  }

  return (status);

}


void CDpio2DriverController::enableClockingOnBothStrobeEdges()
{

  m_pFrontEnd->enableClockingOnBothStrobeEdges();


  /* Update the configuration for of the FIFO frequency and PLL.
   */
  configureFifoFrequency();
  configurePll();

}


void CDpio2DriverController::disableClockingOnBothStrobeEdges()
{

  m_pFrontEnd->disableClockingOnBothStrobeEdges();


  /* Update the configuration for of the FIFO frequency and PLL.
   */
  configureFifoFrequency();
  configurePll();

}


STATUS CDpio2DriverController::setStrobeSkew(int skewInTimingUnits)
{

  return (m_pPllControlState->setStrobeSkew(this, skewInTimingUnits));

}


STATUS CDpio2DriverController::useDefaultStrobeSkew()
{

  return (m_pPllControlState->useDefaultStrobeSkew(this));

}



//##ModelId=3AED589600C8
STATUS CDpio2DriverController::setDataSwapMode(DPIO2_DATA_SWAP_MODE mode)
{

  return (m_pFrontEnd->selectDataSwapMode(mode));

}


STATUS CDpio2DriverController::setPackingMode(DPIO2_PACKING_MODE mode)
{

  return (m_pDataPackingState->setPackingMode(this, mode));

}


BOOL CDpio2DriverController::checkIs8Bit4BitPackingAvailable()
{

  return (m_pFrontEnd->checkIs8Bit4BitPackingAvailable());

}


void CDpio2DriverController::enableNrdyLed()
{

  m_pLeds->enableNrdyLed();

}


void CDpio2DriverController::disableNrdyLed()
{

  m_pLeds->disableNrdyLed();

}



void CDpio2DriverController::enableClkLed()
{

  m_pLeds->enableClkLed();

}


void CDpio2DriverController::disableClkLed()
{

  m_pLeds->disableClkLed();

}


void CDpio2DriverController::enableSuspendLed()
{

  m_pLeds->enableSuspendLed();

}


void CDpio2DriverController::disableSuspendLed()
{

  m_pLeds->disableSuspendLed();

}


void CDpio2DriverController::enableSyncLed()
{

  m_pLeds->enableSyncLed();

}


void CDpio2DriverController::disableSyncLed()
{

  m_pLeds->disableSyncLed();

}


void CDpio2DriverController::enableDvalidLed()
{

  m_pLeds->enableDvalidLed();

}


void CDpio2DriverController::disableDvalidLed()
{

  m_pLeds->disableDvalidLed();

}


BOOL CDpio2DriverController::checkIsOutput()
{

  return (m_pHwInterface->checkIsOutput());

}


UINT8 CDpio2DriverController::getPciDeviceNumber()
{
    
  return (m_pHwInterface->getPciDeviceNumber());
    
}


UINT8 CDpio2DriverController::getPciBusNumber() 
{
    
  return (m_pHwInterface->getPciBusNumber());
    
}


UINT8 CDpio2DriverController::getLatencyTimer()
{

  return (m_pHwInterface->getLatencyTimer());

}


void CDpio2DriverController::setLatencyTimer(UINT8 timerValue)
{

  m_pHwInterface->setLatencyTimer(timerValue);

}


BOOL CDpio2DriverController::checkHasMasterDataParityErrorOccurred()
{

  return (m_pHwInterface->checkHasMasterDataParityErrorOccurred());

}


BOOL CDpio2DriverController::checkIsTargetAbortSignaled()
{

  return (m_pHwInterface->checkIsTargetAbortSignaled());

}


BOOL CDpio2DriverController::checkIsTargetAbortReceived()
{

  return (m_pHwInterface->checkIsTargetAbortReceived());

}


BOOL CDpio2DriverController::checkIsMasterAbortReceived()
{

  return (m_pHwInterface->checkIsMasterAbortReceived());

}


BOOL CDpio2DriverController::checkIsSystemErrorSignaled()
{

  return (m_pHwInterface->checkIsSystemErrorSignaled());

}


UINT32 CDpio2DriverController::getBaseAddressOfFifo()
{

  return (m_pHwInterface->getBaseAddressOfFifo());

}


UINT32 CDpio2DriverController::getBaseAddressOfFlash()
{

  return (m_pHwInterface->getBaseAddressOfFlash());

}


UINT32 CDpio2DriverController::getBaseAddressOfSram()
{

  return (m_pHwInterface->getBaseAddressOfSram());

}


UINT8 CDpio2DriverController::getPersonalityId()
{
  
  return (m_pFrontEnd->getPersonalityId());

}


UINT8 CDpio2DriverController::getMainBoardId()
{

  return (m_pFrontEnd->getMainBoardId());

}




void CDpio2DriverController::updateStrobeFrequencyRange(UINT32 frequency)
{

  m_strobeFrequencyRange = getStrobeFrequencyRange (frequency);

}





void CDpio2DriverController::configureFifoFrequency()
{

  CDpio2FrontEnd*   pFrontEnd;
  CDpio2Fifo*       pFifo;

  DPIO2_RAW_PACKING_MODE   packingMode;

  BOOL   clockingOnBothStrobeEdgesIsEnabled;
  BOOL   bitPackingIsEnabled;
  BOOL   strobeFrequencyIsAbove25Mhz;
  BOOL   strobeFrequencyIsAbove50Mhz;
  BOOL   strobeFrequencyIsAbove100Mhz;


  pFrontEnd = m_pHwInterface->getFrontEnd();
  pFifo = m_pHwInterface->getFifo();

  clockingOnBothStrobeEdgesIsEnabled
    = pFrontEnd->checkIsClockingOnBothStrobeEdgesEnabled();


  /* To simplify the code, all packing modes are assumed to halve 
   * the required FIFO frequency, although most packing modes reduce
   * the required frequency even more.
   */
  packingMode = pFrontEnd->getPackingMode();
  bitPackingIsEnabled = (packingMode != RAW_NO_PACKING);

  strobeFrequencyIsAbove100Mhz = 
    m_strobeFrequencyRange == ABOVE_100MHZ;

  strobeFrequencyIsAbove50Mhz =
    strobeFrequencyIsAbove100Mhz ||
    (m_strobeFrequencyRange == ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ) ||
    (m_strobeFrequencyRange == ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ) ||
    (m_strobeFrequencyRange == ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ) ||
    (m_strobeFrequencyRange == ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ) ||
    (m_strobeFrequencyRange == ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ) ||
    (m_strobeFrequencyRange == ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ) ||
    (m_strobeFrequencyRange == ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ) ||
    (m_strobeFrequencyRange == ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ) ||
    (m_strobeFrequencyRange == ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ) ||
    (m_strobeFrequencyRange == ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ) ||
    (m_strobeFrequencyRange == ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ);
 
  strobeFrequencyIsAbove25Mhz =
    strobeFrequencyIsAbove50Mhz ||
    (m_strobeFrequencyRange == ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ) ||
    (m_strobeFrequencyRange == ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ) ||
    (m_strobeFrequencyRange == ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ) ||
    (m_strobeFrequencyRange == ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ) ||
    (m_strobeFrequencyRange == ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ) ||
    (m_strobeFrequencyRange == ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ); 

  if ( (!clockingOnBothStrobeEdgesIsEnabled
        && ((bitPackingIsEnabled && strobeFrequencyIsAbove100Mhz)
            || (!bitPackingIsEnabled && strobeFrequencyIsAbove50Mhz)))
       || (clockingOnBothStrobeEdgesIsEnabled
           && ((bitPackingIsEnabled && strobeFrequencyIsAbove50Mhz)
               || (!bitPackingIsEnabled && strobeFrequencyIsAbove25Mhz))) ) {

    pFifo->enable100MhzFifoClock();

  } else {

    pFifo->disable100MhzFifoClock();

  }

}


void CDpio2DriverController::configurePll()
{

  m_pPllControlState->configurePll(this);

}


/*meh*/
BOOL CDpio2DriverController::checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range)
{

  return (m_pPllControlState->checkIsStrobeFrequencyRangeValid(range));

}


DPIO2_STROBE_FREQUENCY_RANGE CDpio2DriverController::getStrobeFrequencyRange(UINT32 frequency)
{

  return (m_pPllControlState->getStrobeFrequencyRange(frequency));

}

