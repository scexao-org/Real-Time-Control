/****************************************************************************
Module      : CDpio2DriverState.h

Description : Header file declaring the following classes:

                 CDpio2DataDirectionState
                 CDpio2DriverInputState
                 CDpio2DriverOutputState

                 CDpio2DataPackingState
                 CDpio2Packing8Bit4BitState
                 CDpio2Packing16Bit10BitState

                 CDpio2PllControlState
                 CDpio2PcbAInputState
                 CDpio2PcbAOutputState
                 CDpio2PcbBFpdpInputState
                 CDpio2PcbBFpdpOutputState
                 CDpio2PcbBLvdsInputState
                 CDpio2PcbBLvdsOutputState
                 CDpio2PcbBPeclInputState
                 CDpio2PcbBPeclOutputState
                 CDpio2PcbBRs422InputState
                 CDpio2PcbBRs422OutputState

                 CDpio2PersonalityState
                 CDpio2FpdpPersonalityState
                 CDpio2LvdsPersonalityState
                 CDpio2PeclPersonalityState
                 CDpio2Rs422PersonalityState

              CDpio2DataDirectionState is an abstract class which defines the
              direction dependent functionality available to an application.

              A CDpio2DriverInputState object implements the direction 
              dependent functionality when a DPIO2 is configured as input.

              A CDpio2DriverOutputState object implements the direction 
              dependent functionality when a DPIO2 is configured as output.

              CDpio2DataPackingState is an abstract class which defines
              functionality that depends upon the packing capability.

              A CDpio2Packing8Bit4BitState object implements the functionality 
              that depends upon the packing capability, when a DPIO2 is able
              to pack/unpack 8 bits and 4 bits.

              A CDpio2Packing16Bit10BitState object implements the functionality 
              that depends upon the packing capability, when a DPIO2 is able
              to pack/unpack 16 bits and 10 bits.

              CDpio2PllControlState is an abstract class which defines
              functionality for controlling the PLL.  This functionality is
              dependent upon the HW revision, the personality, and the direction
              the module is operating in.

              A CDpio2PcbAInputState object implements the PLL controlling
              functionality for PCB A versions operating as input.

              A CDpio2PcbAOutputState object implements the PLL controlling
              functionality for PCB A versions operating as output.

              A CDpio2PcbBFpdpInputState object implements the PLL controlling
              functionality for PCB B versions with FPDP personality operating as input.

              A CDpio2PcbBFpdpOutputState object implements the PLL controlling
              functionality for PCB B versions with FPDP personality operating as output.
                 
              A CDpio2PcbBLvdsInputState object implements the PLL controlling
              functionality for PCB B versions with LVDS personality operating as input.

              A CDpio2PcbBLvdsOutputState object implements the PLL controlling
              functionality for PCB B versions with LVDS personality operating as output.
                 
              A CDpio2PcbBPeclInputState object implements the PLL controlling
              functionality for PCB B versions with PECL input personality.

              A CDpio2PcbBPeclOutputState object implements the PLL controlling
              functionality for PCB B versions with PECL output personality.
                 
              A CDpio2PcbBRs422InputState object implements the PLL controlling
              functionality for PCB B versions with RS422 input personality.

              A CDpio2PcbBRs422OutputState object implements the PLL controlling
              functionality for PCB B versions with RS422 output personality.

                 
              CDpio2PersonalityState is a superclass which defines 
              functionality that depends on the personality module of the DPIO2.
              Its subclasses are:

                 CDpio2FpdpPersonalityState, 
                 CDpio2LvdsPersonalityState,
                 CDpio2PeclPersonalityState,
                 CDpio2Rs422PersonalityState

              which are used for DPIO2 modules with FPDP, LVDS, PECL, and
              RS422 personalities correspondingly.


Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01p,06jul2004.meh  Made changes necessary to compile file under VMOSA.
  01o,13feb2004,meh  Support new Strobe Frequency Range.
  01n,22jan2004,meh  Added support for _66_pcb_B
  01m,02oct2002,nib  Renamed TTL and PECL strobe to primary and secondary strobe.
  01l,13sep2002,nib  Added the enableEotSyncCount() and disableEotSyncCount() methods 
                     in the direction state classes.
  01k,22may2002,nib  Added state classes for PECL and RS422 modules.
  01j,18mar2002,nib  Updated file for use in a MCOS environment.
  01i,22feb2002,nib  Added methods for controlling forced assertion of SUSPEND.
  01h,12feb2002,nib  Added the setSyncGenerationCounter() method to the direction 
                     state classes.
  01g,23jan2002,nib  Changed HW version state classes to PLL control state classes.
                     Removed all constructor methods.
  01f,04jan2002,nib  Added classes for HW version states
                     - CDpio2HwVersionState, CDpio2PcbAState, and CDpio2PcbBState.
  01e,31oct2001,nib  Added methods for counter addressing in the direction state classes
                     - enableCounterAddressing() and disableCounterAddressing()
  01d,28oct2001,nib  Updated file to compile in a Windows environment.
  01c,03oct2001,nib  Renamed CDpio2DriverState to CDpio2DataDirectionState
                     Added CDpio2DataPackingState, CDpio2Packing8Bit4BitState,
                     and CDpio2Packing16Bit10BitState.
  01b,02oct2001,nib  Added methods; enableDvalidAlwaysAsserted()
                     and disableDvalidAlwaysAsserted().
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CDPIO2DRIVEROUTPUTSTATE_H_INCLUDED_C54165DD
#define CDPIO2DRIVEROUTPUTSTATE_H_INCLUDED_C54165DD


#include "CDpio2DriverController.h"


#ifdef VXWORKS
#include "CDpio2HwInterface.h"
#endif

#ifdef WIN32
#include "CWDpio2HwInterface.h"
#endif

#ifdef MCOS
#include "CMDpio2HwInterface.h"
#endif

#ifdef VMOSA
#include "vmosaCDpio2HwInterface.h"
#endif




//##ModelId=3ABEF5A00154
class CDpio2DataDirectionState
{

public:
  virtual STATUS setInputDmaDescriptor(CDpio2DriverController* pController,
				       UINT32 descriptorId,
				       const CDpio2InputDmaDescriptor* pDescriptor) = 0;
  virtual STATUS setOutputDmaDescriptor(CDpio2DriverController* pController,
					UINT32 descriptorId,
					const CDpio2OutputDmaDescriptor* pDescriptor) = 0;

  virtual STATUS selectEndOfTransferMode(CDpio2DriverController* pController, 
                                         DPIO2_END_OF_TRANSFER_MODE mode) = 0;
  virtual STATUS enableEndOfTransfer(CDpio2DriverController* pController) = 0;
  virtual STATUS disableEndOfTransfer(CDpio2DriverController* pController) = 0;

  virtual STATUS enableEotSyncCount(CDpio2DriverController* pController, UINT16 countValue) = 0;
  virtual STATUS disableEotSyncCount(CDpio2DriverController* pController) = 0;

  virtual STATUS enableContinueOnEndOfTransfer(CDpio2DriverController* pController) = 0;
  virtual STATUS disableContinueOnEndOfTransfer(CDpio2DriverController* pController) = 0;

  virtual STATUS setSyncGenerationCounter(CDpio2DriverController* pController, 
                                          UINT32 value) = 0;

  virtual STATUS selectSyncGenerationMode(CDpio2DriverController* pController,
                                          DPIO2_SYNC_GENERATION_MODE mode) = 0;
  virtual STATUS selectSyncReceptionMode(CDpio2DriverController* pController,
                                         DPIO2_SYNC_RECEPTION_MODE mode) = 0;

  virtual STATUS enableVideoMode(CDpio2DriverController* pController) = 0;
  virtual STATUS disableVideoMode(CDpio2DriverController* pController) = 0;

  virtual STATUS enableDvalidAlwaysAsserted(CDpio2DriverController* pController) = 0;
  virtual STATUS disableDvalidAlwaysAsserted(CDpio2DriverController* pController) = 0;

  virtual STATUS enableCounterAddressing(CDpio2DriverController* pController,
                                         UINT32 initialSkipCount, 
                                         UINT32 skipCount, 
                                         UINT32 receiveCount) = 0;
  virtual STATUS disableCounterAddressing(CDpio2DriverController* pController) = 0;

  virtual STATUS checkIsPipelineNotEmpty(CDpio2DriverController* pController, BOOL& flag) = 0;
  virtual STATUS flushPackingPipeline(CDpio2DriverController* pController) = 0;

  virtual STATUS enableForcedSuspendAssertion(CDpio2DriverController* pController) = 0;
  virtual STATUS disableForcedSuspendAssertion(CDpio2DriverController* pController) = 0;

};


//##ModelId=3ABEF68F0334
class CDpio2DriverInputState : public CDpio2DataDirectionState
{

public:
  virtual STATUS setInputDmaDescriptor(CDpio2DriverController* pController,
				       UINT32 descriptorId,
				       const CDpio2InputDmaDescriptor* pDescriptor);
  virtual STATUS setOutputDmaDescriptor(CDpio2DriverController* pController,
					UINT32 descriptorId,
					const CDpio2OutputDmaDescriptor* pDescriptor);

  virtual STATUS selectEndOfTransferMode(CDpio2DriverController* pController, 
                                         DPIO2_END_OF_TRANSFER_MODE mode);
  virtual STATUS enableEndOfTransfer(CDpio2DriverController* pController);
  virtual STATUS disableEndOfTransfer(CDpio2DriverController* pController);

  virtual STATUS enableEotSyncCount(CDpio2DriverController* pController, UINT16 countValue);
  virtual STATUS disableEotSyncCount(CDpio2DriverController* pController);

  virtual STATUS enableContinueOnEndOfTransfer(CDpio2DriverController* pController);
  virtual STATUS disableContinueOnEndOfTransfer(CDpio2DriverController* pController);

  virtual STATUS setSyncGenerationCounter(CDpio2DriverController* pController, UINT32 value);

  virtual STATUS selectSyncGenerationMode(CDpio2DriverController* pController,
                                          DPIO2_SYNC_GENERATION_MODE mode);
  virtual STATUS selectSyncReceptionMode(CDpio2DriverController* pController,
                                         DPIO2_SYNC_RECEPTION_MODE mode);

  virtual STATUS enableVideoMode(CDpio2DriverController* pController);
  virtual STATUS disableVideoMode(CDpio2DriverController* pController);

  virtual STATUS enableDvalidAlwaysAsserted(CDpio2DriverController* pController);
  virtual STATUS disableDvalidAlwaysAsserted(CDpio2DriverController* pController);

  virtual STATUS enableCounterAddressing(CDpio2DriverController* pController,
                                         UINT32 initialSkipCount, 
                                         UINT32 skipCount, 
                                         UINT32 receiveCount);
  virtual STATUS disableCounterAddressing(CDpio2DriverController* pController);

  virtual STATUS checkIsPipelineNotEmpty(CDpio2DriverController* pController, BOOL& flag);
  virtual STATUS flushPackingPipeline(CDpio2DriverController* pController);

  virtual STATUS enableForcedSuspendAssertion(CDpio2DriverController* pController);
  virtual STATUS disableForcedSuspendAssertion(CDpio2DriverController* pController);

};


//##ModelId=3ABEF6CE03D4
class CDpio2DriverOutputState : public CDpio2DataDirectionState
{

public:
  virtual STATUS setInputDmaDescriptor(CDpio2DriverController* pController,
				       UINT32 descriptorId,
				       const CDpio2InputDmaDescriptor* pDescriptor);
  virtual STATUS setOutputDmaDescriptor(CDpio2DriverController* pController,
					UINT32 descriptorId,
					const CDpio2OutputDmaDescriptor* pDescriptor);

  virtual STATUS selectEndOfTransferMode(CDpio2DriverController* pController, 
                                         DPIO2_END_OF_TRANSFER_MODE mode);
  virtual STATUS enableEndOfTransfer(CDpio2DriverController* pController);
  virtual STATUS disableEndOfTransfer(CDpio2DriverController* pController);

  virtual STATUS enableEotSyncCount(CDpio2DriverController* pController, UINT16 countValue);
  virtual STATUS disableEotSyncCount(CDpio2DriverController* pController);

  virtual STATUS enableContinueOnEndOfTransfer(CDpio2DriverController* pController);
  virtual STATUS disableContinueOnEndOfTransfer(CDpio2DriverController* pController);

  virtual STATUS setSyncGenerationCounter(CDpio2DriverController* pController, UINT32 value);

  virtual STATUS selectSyncGenerationMode(CDpio2DriverController* pController,
                                          DPIO2_SYNC_GENERATION_MODE mode);
  virtual STATUS selectSyncReceptionMode(CDpio2DriverController* pController,
                                         DPIO2_SYNC_RECEPTION_MODE mode);

  virtual STATUS enableVideoMode(CDpio2DriverController* pController);
  virtual STATUS disableVideoMode(CDpio2DriverController* pController);

  virtual STATUS enableDvalidAlwaysAsserted(CDpio2DriverController* pController);
  virtual STATUS disableDvalidAlwaysAsserted(CDpio2DriverController* pController);

  virtual STATUS enableCounterAddressing(CDpio2DriverController* pController,
                                         UINT32 initialSkipCount, 
                                         UINT32 skipCount, 
                                         UINT32 receiveCount);
  virtual STATUS disableCounterAddressing(CDpio2DriverController* pController);

  virtual STATUS checkIsPipelineNotEmpty(CDpio2DriverController* pController, BOOL& flag);
  virtual STATUS flushPackingPipeline(CDpio2DriverController* pController);

  virtual STATUS enableForcedSuspendAssertion(CDpio2DriverController* pController);
  virtual STATUS disableForcedSuspendAssertion(CDpio2DriverController* pController);

};


class CDpio2DataPackingState
{

public:

  virtual STATUS setPackingMode(CDpio2DriverController* pController, 
                                DPIO2_PACKING_MODE mode) = 0;

};


class CDpio2Packing8Bit4BitState : public CDpio2DataPackingState
{

public:

  virtual STATUS setPackingMode(CDpio2DriverController* pController, 
                                DPIO2_PACKING_MODE mode);

};


class CDpio2Packing16Bit10BitState : public CDpio2DataPackingState
{

public:

  virtual STATUS setPackingMode(CDpio2DriverController* pController, 
                                DPIO2_PACKING_MODE mode);

};


class CDpio2PllControlState
{

public:

  virtual void configurePll(CDpio2DriverController* pController) = 0;
  
  virtual STATUS setStrobeSkew(CDpio2DriverController* pController,
                               int skewInTimingUnits) = 0;
  virtual STATUS useDefaultStrobeSkew(CDpio2DriverController* pController) = 0;

  virtual BOOL checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range) = 0;

  virtual DPIO2_STROBE_FREQUENCY_RANGE getStrobeFrequencyRange(UINT32 frequency) = 0;


};


class CDpio2PcbAState : public CDpio2PllControlState
{

public:

  virtual void configurePll(CDpio2DriverController* pController);

  virtual STATUS setStrobeSkew(CDpio2DriverController* pController,
                               int skewInTimingUnits);
  virtual STATUS useDefaultStrobeSkew(CDpio2DriverController* pController);

  virtual BOOL checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range);

  virtual DPIO2_STROBE_FREQUENCY_RANGE getStrobeFrequencyRange(UINT32 frequency);

protected:


};


class CDpio2PcbBState : public CDpio2PllControlState
{

public:
  CDpio2PcbBState();

  virtual void configurePll(CDpio2DriverController* pController);
  
  virtual STATUS setStrobeSkew(CDpio2DriverController* pController, int skewInTimingUnits);

  virtual STATUS useDefaultStrobeSkew(CDpio2DriverController* pController);

  virtual BOOL checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range);

  virtual DPIO2_STROBE_FREQUENCY_RANGE getStrobeFrequencyRange(UINT32 frequency);

protected:

  BOOL   m_clockingOnBothStrobeEdgesIsEnabled;
  BOOL   m_moduleIsGeneratingStrobe;

  BOOL   m_useDefaultStrobeSkew;
  DPIO2_PLL_OPTION   m_pllOption0;
  DPIO2_PLL_OPTION   m_pllOption1;

  virtual void configurePllFrequencySelect(CDpio2DriverController* pController);
  virtual void configurePllOptions(CDpio2DriverController* pController) = 0;

};


class CDpio2PcbBFpdpInputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2PcbBFpdpOutputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2PcbBLvdsInputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2PcbBLvdsOutputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};


class CDpio2PcbBPeclInputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2PcbBPeclOutputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2PcbBRs422InputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2PcbBRs422OutputState : public CDpio2PcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};




/*meh, new 66 pcb B */

class CDpio2_66MHzPcbBState : public CDpio2PcbBState
{

public:
  /*  CDpio2_66MHzPcbBState();*/
  virtual void configurePll(CDpio2DriverController* pController);
  virtual STATUS setStrobeSkew(CDpio2DriverController* pController, int skewInTimingUnits);

  virtual BOOL checkIsStrobeFrequencyRangeValid(DPIO2_STROBE_FREQUENCY_RANGE& range);
  virtual DPIO2_STROBE_FREQUENCY_RANGE getStrobeFrequencyRange(UINT32 frequency);

protected:

  virtual void configurePllFrequencySelect(CDpio2DriverController* pController);


};


class CDpio2_66MHzPcbBFpdpInputState : public CDpio2_66MHzPcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};



class CDpio2_66MHzPcbBFpdpOutputState : public CDpio2_66MHzPcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2_66MHzPcbBLvdsInputState : public CDpio2_66MHzPcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};

class CDpio2_66MHzPcbBLvdsOutputState : public CDpio2_66MHzPcbBState
{

protected:

  virtual void configurePllOptions(CDpio2DriverController* pController);

};







class CDpio2PersonalityState
{

public:

  virtual STATUS enableStrobeGeneration(CDpio2DriverController* pController);
  virtual STATUS disableStrobeGeneration(CDpio2DriverController* pController);

  virtual STATUS selectSecondaryStrobe(CDpio2DriverController* pController);


};

class CDpio2FpdpPersonalityState : public CDpio2PersonalityState
{

public:


};

class CDpio2LvdsPersonalityState : public CDpio2PersonalityState
{

public:

  virtual STATUS selectSecondaryStrobe(CDpio2DriverController* pController);

};

class CDpio2PeclPersonalityState : public CDpio2PersonalityState
{

public:

  virtual STATUS enableStrobeGeneration(CDpio2DriverController* pController);
  virtual STATUS disableStrobeGeneration(CDpio2DriverController* pController);

  virtual STATUS selectSecondaryStrobe(CDpio2DriverController* pController);

};

class CDpio2Rs422PersonalityState : public CDpio2PersonalityState
{

public:

  virtual STATUS selectSecondaryStrobe(CDpio2DriverController* pController);

};


#endif /* CDPIO2DRIVEROUTPUTSTATE_H_INCLUDED_C54165DD */

