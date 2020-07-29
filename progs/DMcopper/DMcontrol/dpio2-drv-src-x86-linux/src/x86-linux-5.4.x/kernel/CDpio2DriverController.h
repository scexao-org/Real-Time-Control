/****************************************************************************
Module      : CDpio2DriverController.h

Description : Header file declaring the CDpio2DriverController class.
              A CDpio2DriverController object represents all available
              functionality of one DPIO2 module, and controls that 
              direction dependent functionlity is used correctly.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01v,06jul2004.meh  Made changes necessary to compile file under VMOSA.
  01u,13feb2004,meh  Support new Strobe Frequency Range.
  01t,22jan2004,meh  Added support for _66_pcb_B
  01s,02oct2002,nib  Renamed TTL and PECL strobe to primary and secondary strobe.
  01r,19sep2002,nib  Added the setTestPatternStartValue() method.
  01q,13sep2002,nib  Added the getTransferredByteCount(), enableEotSyncCount(),
                     and disableEotSyncCount() methods.
  01p,22may2002,nib  Added support for PECL and RS422 modules.
  01p,22feb2002,nib  Added methods for controlling forced assertion of SUSPEND.
  01o,19feb2002,nib  Added support for test pattern generation.
  01n,12feb2002,nib  Added the setSyncGenerationCounter() method.
  01m,23jan2002,nib  Converted m_pHwVersionState to m_pPllControlState.
  01l,04jan2002,nib  Added HW Version State - m_pHwVersionState.
  01k,13dec2001,nib  Added methods for controlling DMA FIFO flushing on stop
                     - enableDmaFifoFlushOnStop() and disableDmaFifoFlushOnStop()
  01j,20nov2001,nib  Added method for getting the base address of the DPIO2 FLASH
                     - getBaseAddressOfFlash(). 
  01i,31oct2001,nib  Added method for reading the number of bytes in the descriptor
                     that were not transferred - getRemainingByteCount(). 
  01h,31oct2001,nib  Added methods for counter addressing
                     - enableCounterAddressing() and disableCounterAddressing()
  01g,28oct2001,nib  Updated file to compile in a Windows environment.
  01f,03oct2001,nib  Renamed m_pState to m_pDataDirectionState.
                     Added m_pDataPackingState.
  01e,02oct2001,nib  Added methods; enableDvalidAlwaysAsserted()
                     and disableDvalidAlwaysAsserted().
  01d,01oct2001,nib  Replaced enablePacking() and disablePacking() 
                     with setPackingMode().
  01c,26sep2001,nib  Added flushPackingPipeline().
  01b,24sep2001,nib  Added enableD0ToBeUsedForSync() and disableD0ToBeUsedForSync()
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CDPIO2DRIVERCONTROLLER_H_INCLUDED_C569D287
#define CDPIO2DRIVERCONTROLLER_H_INCLUDED_C569D287


#ifdef VXWORKS
#include "vxWorks.h"
#endif

#ifdef WIN32
#include <windows.h>
#include "WindowsTypes.h"
#endif

#ifdef VMOSA
#include "vmosa.h"
#endif


#include "DpioCommon.h"
#include "Dpio2Defs.h"

class CAction;
class CDpio2DmaController;
class CDpio2InputDmaDescriptor;
class CDpio2OutputDmaDescriptor;
class CDpio2DataDirectionState;
class CDpio2DriverInputState;
class CDpio2DriverOutputState;
class CDpio2DataPackingState;
class CDpio2Packing8Bit4BitState;
class CDpio2Packing16Bit10BitState;
class CDpio2PllControlState;
class CDpio2PcbAState;
class CDpio2PcbBState;
class CDpio2_66MHzPcbBState;
class CDpio2PcbBFpdpInputState;
class CDpio2PcbBFpdpOutputState;
class CDpio2PcbBLvdsInputState;
class CDpio2PcbBLvdsOutputState;
class CDpio2PcbBPeclInputState;
class CDpio2PcbBPeclOutputState;
class CDpio2PcbBRs422InputState;
class CDpio2PcbBRs422OutputState;
class CDpio2_66MHzPcbBFpdpInputState;
class CDpio2_66MHzPcbBFpdpOutputState;
class CDpio2_66MHzPcbBLvdsInputState;
class CDpio2_66MHzPcbBLvdsOutputState;
class CDpio2PersonalityState;
class CDpio2FpdpPersonalityState;
class CDpio2LvdsPersonalityState;
class CDpio2PeclPersonalityState;
class CDpio2Rs422PersonalityState;
class CDpio2Fifo;
class CDpio2FrontEnd;
class CDpio2HwInterface;
class CDpio2Leds;
class CDpio2Oscillator;
class CInterruptManager;


//##ModelId=3A6BFA790258
class CDpio2DriverController
{

    friend class CDpio2DataDirectionState;
    friend class CDpio2DriverInputState;
    friend class CDpio2DriverOutputState;

    friend class CDpio2DataPackingState;
    friend class CDpio2Packing8Bit4BitState;
    friend class CDpio2Packing16Bit10BitState;

    friend class CDpio2PllControlState;
    friend class CDpio2PcbAState;
    friend class CDpio2PcbBState;
    friend class CDpio2_66MHzPcbBState;
    friend class CDpio2PcbBFpdpInputState;
    friend class CDpio2PcbBFpdpOutputState;
    friend class CDpio2PcbBLvdsInputState;
    friend class CDpio2PcbBLvdsOutputState;
    friend class CDpio2PcbBPeclInputState;
    friend class CDpio2PcbBPeclOutputState;
    friend class CDpio2PcbBRs422InputState;
    friend class CDpio2PcbBRs422OutputState;
    friend class CDpio2_66MHzPcbBFpdpInputState;
    friend class CDpio2_66MHzPcbBFpdpOutputState;
    friend class CDpio2_66MHzPcbBLvdsInputState;
    friend class CDpio2_66MHzPcbBLvdsOutputState;

    friend class CDpio2PersonalityState;
    friend class CDpio2FpdpPersonalityState;
    friend class CDpio2LvdsPersonalityState;
    friend class CDpio2PeclPersonalityState;
    friend class CDpio2Rs422PersonalityState;


  public:
    //##ModelId=3A8247E50294
    CDpio2DriverController(CDpio2HwInterface* pHwInterface);

    //##ModelId=3A8247E502D0
    virtual ~CDpio2DriverController();

    //##ModelId=3AE6892201C2
    void initialize();


    /*************************************************************
     **                Interrupt functionality
     ************************************************************/

    //##ModelId=3AED589403DE
    STATUS addInterruptAction(DPIO2_INTERRUPT_SOURCE_ID sourceId, CAction* pAction);
    //##ModelId=3AED58950172
    STATUS removeInterruptAction(DPIO2_INTERRUPT_SOURCE_ID sourceId, CAction* pAction);

    //##ModelId=3AED589502F8
    STATUS enableInterruptSource(DPIO2_INTERRUPT_SOURCE_ID sourceId);
    //##ModelId=3AED589503D4
    STATUS disableInterruptSource(DPIO2_INTERRUPT_SOURCE_ID sourceId);
    void disableAllInterruptSources();

    STATUS checkIsInterruptSourceEnabled(DPIO2_INTERRUPT_SOURCE_ID sourceId, BOOL& enableFlag);

    STATUS checkIsInterruptSourceActive(DPIO2_INTERRUPT_SOURCE_ID sourceId, BOOL& activeFlag);


    /*************************************************************
     **                  Fifo functionality
     ************************************************************/

    UINT32 getFifoSize();

    DPIO_FIFO_STATE getFifoState();

    void flushFifo();

    void resetFifoHistoryFlags();

    //##ModelId=3A9A7DF80208
    BOOL checkHasFifoOverflowed();
    BOOL checkHasFifoBeenFull();
    //##ModelId=3A9A7DF8019A
    BOOL checkHasFifoBeenAlmostFull();
    BOOL checkHasFifoBeenMoreThanHalfFull();
    //##ModelId=3A9A7DF8015E
    BOOL checkHasFifoBeenAlmostEmpty();
    BOOL checkHasFifoBeenEmpty();


    void enableSuspendFlowControl();
    void disableSuspendFlowControl();

    void enableNrdyFlowControl();
    void disableNrdyFlowControl();



    /*************************************************************
     **                  DMA functionality
     ************************************************************/
    STATUS setInputDmaDescriptor(UINT32 descriptorId,
				 const CDpio2InputDmaDescriptor* pDescriptor);
    STATUS setOutputDmaDescriptor(UINT32 descriptorId,
				  const CDpio2OutputDmaDescriptor* pDescriptor);

    STATUS setNextDescriptorInDmaChain(UINT32 descriptorId);
    UINT32 getNextDescriptorInDmaChain();

    void startDmaTransfer();
    void stopDmaTransfer();

    void enableDmaFifoFlushOnStop();
    void disableDmaFifoFlushOnStop();

    void suspendDmaTransfer();
    void resumeDmaTransfer();
    
    BOOL checkIsDmaTransferCompleted();
    BOOL checkIsDmaTransferSuspended();

    void enableDmaDemandMode();
    void disableDmaDemandMode();
    BOOL checkIsDmaDemandModeEnabled();

    STATUS selectEndOfTransferMode(DPIO2_END_OF_TRANSFER_MODE mode);
    STATUS enableEndOfTransfer();
    STATUS disableEndOfTransfer();

    STATUS enableEotSyncCount(UINT16 countValue);
    STATUS disableEotSyncCount();

    STATUS enableContinueOnEndOfTransfer();
    STATUS disableContinueOnEndOfTransfer();

    void getRemainingByteCount(UINT32& byteCount, 
                               BOOL& byteCountHasOverflowed);
    void getTransferredByteCount(UINT32& byteCount);

    void setDmaPciReadCommand(UINT32 command);
    void setDmaPciWriteCommand(UINT32 command);


    /*************************************************************
     **                Front-End functionality
     ************************************************************/

    void enableFrontEndInterrupts();
    void disableFrontEndInterrupts();
    BOOL checkAreFrontEndInterruptsEnabled();

    STATUS enableStrobeGeneration();
    STATUS disableStrobeGeneration();
    BOOL checkIsStrobeGenerationEnabled();

    //##ModelId=3AED589601A4
    void enableMasterInterface();
    //##ModelId=3AED589601D6
    void disableMasterInterface();

    void startTransfer();
    void stopTransfer();

    STATUS selectFreeRunningClock();
    STATUS selectAsynchronousClock();

    STATUS selectPrimaryStrobe();
    STATUS selectSecondaryStrobe();

    void enableRes1AsOutput();
    void disableRes1AsOutput();
    void setRes1OutputHigh();
    void setRes1OutputLow();
    BOOL checkIsRes1InputHigh();

    void enableRes2AsOutput();
    void disableRes2AsOutput();
    void setRes2OutputHigh();
    void setRes2OutputLow();
    BOOL checkIsRes2InputHigh();
   
    void enableRes3AsOutput();
    void disableRes3AsOutput();
    void setRes3OutputHigh();
    void setRes3OutputLow();
    BOOL checkIsRes3InputHigh();

    void enablePio1AsOutput();
    void disablePio1AsOutput();
    void setPio1OutputHigh();
    void setPio1OutputLow();
    BOOL checkIsPio1InputHigh();

    void enablePio2AsOutput();
    void disablePio2AsOutput();
    void setPio2OutputHigh();
    void setPio2OutputLow();
    BOOL checkIsPio2InputHigh();

    BOOL checkIsDirSignalHigh();

    BOOL checkIsSuspendAsserted();

    STATUS setSyncGenerationCounter(UINT32 value);

    STATUS selectSyncGenerationMode(DPIO2_SYNC_GENERATION_MODE mode);
    STATUS selectSyncReceptionMode(DPIO2_SYNC_RECEPTION_MODE mode);

    void enableD0ToBeUsedForSync();
    void disableD0ToBeUsedForSync();

    STATUS enableVideoMode();
    STATUS disableVideoMode();

    STATUS enableDvalidAlwaysAsserted();
    STATUS disableDvalidAlwaysAsserted();

    STATUS enableCounterAddressing(UINT32 initialSkipCount, 
                                   UINT32 skipCount, 
                                   UINT32 receiveCount);
    STATUS disableCounterAddressing();

    STATUS enableTestPatternGeneration(DPIO2_TEST_PATTERN pattern);
    void disableTestPatternGeneration();

    void setTestPatternStartValue(UINT16 value);

    STATUS enableForcedSuspendAssertion();
    STATUS disableForcedSuspendAssertion();
    


    //##ModelId=3AE68B82005A
    //##Documentation
    //## Program the programmable oscillator to generate a specific frequency.
    //## The frequency is specified in Hz and must be beween 3125Hz and 100MHz.
    //## 
    //## NOTE: Remember to disable the strobe PLL before programming the
    //## oscillator to a frequency below 10MHz.
    //## 
    //## The oscillator can not exactly generate all frequencies in the valid
    //## range.  For the frequencies which can not be generated exactly, this
    //## method computes the obtainable frequency which is closest to the
    //## requested, 
    //## and programs the oscillator to generate this.
    //## 
    //## Because of this possible deviation between requested and programmed
    //## frequency, the programmed frequency is returned by the second argument
    //## to this method.
    STATUS setOscillatorFrequency(UINT32 requestedFrequency,
                                  UINT32& resultingFrequency);
    UINT32 getOscillatorFrequency();

    void activateProgrammableOscillator();
    void activateFixedOscillator();
    BOOL checkIsProgrammableOscillatorActivated();

    STATUS setStrobeFrequencyRange(DPIO2_STROBE_FREQUENCY_RANGE range);

    void enableClockingOnBothStrobeEdges();
    void disableClockingOnBothStrobeEdges();

    STATUS setStrobeSkew(int skewInTimingUnits);
    STATUS useDefaultStrobeSkew();

    //##ModelId=3AED589600C8
    STATUS setDataSwapMode(DPIO2_DATA_SWAP_MODE mode);

    STATUS setPackingMode(DPIO2_PACKING_MODE mode);
    BOOL checkIs8Bit4BitPackingAvailable();
    STATUS checkIsPipelineNotEmpty(BOOL& flag);
    STATUS flushPackingPipeline();
    

    /*************************************************************
     **                  LED functionality
     ************************************************************/
    
    void enableNrdyLed();
    void disableNrdyLed();

    void enableClkLed();
    void disableClkLed();

    void enableSuspendLed();
    void disableSuspendLed();

    void enableSyncLed();
    void disableSyncLed();

    void enableDvalidLed();
    void disableDvalidLed();


    
    /*************************************************************
     **             DPIO2 HW related functionality
     ************************************************************/
    
    BOOL checkIsOutput();

    UINT8 getPciDeviceNumber();
    UINT8 getPciBusNumber();

    UINT8 getLatencyTimer();
    void setLatencyTimer(UINT8 timerValue);

    BOOL checkHasMasterDataParityErrorOccurred();
    BOOL checkIsTargetAbortSignaled();
    BOOL checkIsTargetAbortReceived();
    BOOL checkIsMasterAbortReceived();
    BOOL checkIsSystemErrorSignaled();

    UINT32 getBaseAddressOfFifo();
    UINT32 getBaseAddressOfSram();
    UINT32 getBaseAddressOfFlash();

    UINT8 getPersonalityId();
    UINT8 getMainBoardId();


  protected:
    //##ModelId=3ABEF8090032
    CDpio2DataDirectionState *m_pDataDirectionState;

    CDpio2DataPackingState* m_pDataPackingState;

    CDpio2PllControlState* m_pPllControlState;

    CDpio2PersonalityState* m_pPersonalityState;


  private:
    //##ModelId=3A89184301B8
    CDpio2HwInterface *m_pHwInterface;

    //##ModelId=3A950C86006E
    CDpio2DmaController* m_pPrimaryDmaController;

    //##ModelId=3A950C8603A2
    CDpio2Fifo* m_pFifo;

    //##ModelId=3A950C8E0014
    CDpio2FrontEnd* m_pFrontEnd;

    //##ModelId=3A950C90021C
    CDpio2Oscillator* m_pOscillator;

    //##ModelId=3A950C91005A
    CDpio2Leds* m_pLeds;

    CInterruptManager*   m_pInterruptManager;


    UINT32 m_strobeFrequency;
    DPIO2_STROBE_FREQUENCY_RANGE m_strobeFrequencyRange;

    BOOL m_inputStrobeIsAsynchronous;


    void updateStrobeFrequencyRange(UINT32 frequency);

    void configureFifoFrequency();
    void configurePll();

    BOOL checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range);
    DPIO2_STROBE_FREQUENCY_RANGE getStrobeFrequencyRange(UINT32 frequency);

};


#endif /* CDPIO2DRIVERCONTROLLER_H_INCLUDED_C569D287 */



