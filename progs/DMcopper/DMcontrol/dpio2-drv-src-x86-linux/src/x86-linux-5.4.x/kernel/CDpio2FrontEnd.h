/****************************************************************************
Module      : CDpio2FrontEnd.h

Description : Header file declaring the CDpio2FrontEnd class.
              A CDpio2FrontEnd object represents the functionality related to
              the front end of a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01q,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01p,02oct2002,nib  Renamed TTL and PECL strobe to primary and secondary strobe.
  01o,19sep2002,nib  Renamed CONSTANT_PATTERN to COUNTER_PATTERN_WITH_PROGRAMMABLE_START.
  01n,22may2002,nib  Added support for PLL on PECL and RS422 personality modules.
  01m,08apr2002,nib  Added support for flushing internal FIFOs in the Front End.
  01l,18mar2002,nib  Updated file for use in a MCOS environment.
  01k,22feb2002,nib  Added methods for asserting/deasserting SUSPEND.
  01j,19feb2002,nib  Added support for test pattern generation.
  01i,17dec2001,nib  Added support for PLL options.
  01h,21nov2001,nib  Added get methods for the four quadbyte counters
                     - getInitialSkipCounter(), getSkipCounter(), getReceivCounter(),
                     and getSyncCounter().
  01g,31oct2001,nib  Added methods for controlling the enable counter addressing bit
                     - enableCounterAddressing() and disableCounterAddressing().
  01f,26oct2001,nib  Updated file for use in a Windows environment.
  01e,04oct2001,nib  Added methods for setting the four quad-byte counters;
                     setInitialSkipCounter(), setSkipCounter(), setReceiveCounter(), 
                     and setSyncCounter().
  01d,02oct2001,nib  Added m_bitAssumeDvalidAlwaysAsserted and
                     enableDvalidAlwaysAsserted() and enableDvalidAlwaysAsserted()
  01c,01oct2001,nib  Changed m_bitEnable16BitPacking and m_bitUseMostSignificantBitsInPacking 
                     into a control bit field; m_bitFieldPackingMode.
                     Added status bit; m_bit8Bit4BitPackingAvailable.
                     Added new method; checkIs8Bit4BitPackingAvailable().
                     Renamed bitFlush16BitPackingPipeline to bitFlushPackingPipeline
                     Changed enable16BitPacking() to setPackingMode().
                     Changed checkIs16BitPackingEnabled() to getPackingMode().
                     Changed flush16BitPackingPipeline() to flushPackingPipeline().
                     Removed disable16BitPacking().
  01b,19jul2001,nib  Implemented support for 8 and 16 bit swapping, and replaced 
                     enable32BitSwap(), disable32BitSwap(), enable16BitSwap(),
                     disable16BitSwap(), enable8BitSwap(), and disable8BitSwap()
                     with selectDataSwapMode().
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CDPIO2FRONTEND_H_INCLUDED_C5699E4E
#define CDPIO2FRONTEND_H_INCLUDED_C5699E4E


#if 1 /*meh*/
#define MARKM printf ("Was Here %s,%d\n", __FILE__, __LINE__);
#endif



#if defined(WIN32)

#include "CWControlBit.h"
#include "CWControlBitField.h"
#include "CWControlQuadByte.h"
#include "CWStatusBit.h"
#include "CWStatusBitField.h"

#elif defined(MCOS)

#include "CMControlBit.h"
#include "CMControlBitField.h"
#include "CControlQuadByte.h"
#include "CStatusBit.h"
#include "CStatusBitField.h"

#elif defined(VMOSA)

#include "vmosaCControlBit.h"
#include "vmosaCControlBitField.h"
#include "vmosaCControlQuadByte.h"
#include "vmosaCStatusBit.h"
#include "vmosaCStatusBitField.h"

#else

#include "CControlBit.h"
#include "CControlBitField.h"
#include "CControlQuadByte.h"
#include "CStatusBit.h"
#include "CStatusBitField.h"

#endif

#include "DpioCommon.h"
#include "Dpio2Defs.h"


typedef enum
{

  RAW_NO_PACKING = 0,
  RAW_PACK_16LSB_OR_8LSB = 1,
  RAW_PACK_10LSB_OR_4LSB = 2,
  RAW_PACK_16MSB = 3

} DPIO2_RAW_PACKING_MODE;


typedef enum
{

  DPIO2_PLL_OPTION_LOW,
  DPIO2_PLL_OPTION_MID,
  DPIO2_PLL_OPTION_HIGH

} DPIO2_PLL_OPTION;


//##ModelId=3A9271D20014
class CDpio2FrontEnd
{

public:

  //##ModelId=3AB8BB0803CA
  CDpio2FrontEnd(const CControlBit& bitEnableStrobeGeneration,
                 const CControlBit& bitActivateFpdpTransfer,
                 const CControlBit& bitEnableSecondaryStrobe,
                 const CControlBit& bitRes1OutputEnable,
                 const CControlBit& bitRes1OutputValue,
                 const CControlBit& bitRes2OutputEnable,
                 const CControlBit& bitRes2OutputValue,
                 const CControlBit& bitPio1OutputEnable,
                 const CControlBit& bitPio1OutputValue,
                 const CControlBit& bitPio2OutputEnable,
                 const CControlBit& bitPio2OutputValue,
                 const CControlBit& bitEnableInterrupts,
                 const CControlBit& bitEnableWordSwap,
                 const CControlBit& bitEnableMasterInterface,
                 const CControlBit& bitEnablePll,
                 const CControlBit& bitEnableClockingOnBothStrobeEdges,
                 const CControlBit& bitAssertSuspend,
                 const CControlBit& bitEnablePioDecoding,
                 const CControlBit& bitEnableCounterDecoding,
                 const CControlBit& bitRes3OutputEnable,
                 const CControlBit& bitRes3OutputValue,
                 const CControlBit& bitDisableJumberForStrobeGeneration,
                 const CControlBit& bitDisableJumperForMasterInterface,
                 const CControlBit& bitEnableProgrammableOscillator,
                 const CControlBit& bitFlushPackingPipeline,
                 const CControlBit& bitEnableVideoMode,
                 const CControlBit& bitEnable8BitSwap,
                 const CControlBit& bitEnable16BitSwap,
                 const CControlBit& bitEnableD0ForSync,
                 const CControlBit& bitAssumeDvalidAlwaysAsserted,
                 const CControlBit& bitPllOpt1Mid,
                 const CControlBit& bitPllOpt1High,
                 const CControlBit& bitPllOpt0Mid,
                 const CControlBit& bitPllOpt0High,
                 const CControlBit& bitPllFsMid,
                 const CControlBit& bitPllFsHigh,
                 const CControlBit& bitEnableTestPatternGeneration,
                 const CControlBit& bitFlushInternalFifos,
                 const CControlBit& bitPersonalityPllOutputEnable,
                 const CControlBit& bitPersonalityPllOutputValue,

                 const CControlBitField& bitFieldPioAddress,
                 const CControlBitField& bitFieldPioAddressMask,
                 const CControlBitField& bitFieldSyncMode,
                 const CControlBitField& bitFieldPackingMode,
                 const CControlBitField& bitFieldTestPattern,

                 const CControlQuadByte& initialSkipCounter,
                 const CControlQuadByte& receiveCounter,
                 const CControlQuadByte& skipCounter,
                 const CControlQuadByte& syncCounter,

                 const CStatusBit& bitDataInPipeLine,
                 const CStatusBit& bitRes1InputValue,
                 const CStatusBit& bitRes2InputValue,
                 const CStatusBit& bitRes3InputValue,
                 const CStatusBit& bitPio1InputValue,
                 const CStatusBit& bitPio2InputValue,
                 const CStatusBit& bitDirInputValue,
                 const CStatusBit& bitNrdyInputValue,
                 const CStatusBit& bitSuspendInputValue,
                 const CStatusBit& bitStrobeGenerationIsEnabled,
                 const CStatusBit& bitMasterInterfaceIsEnabled,
                 const CStatusBit& bit8Bit4BitPackingAvailable,
                 const CStatusBit& bitFpdp2IsSupported,

                 const CStatusBitField& bitFieldPersonalityId,
                 const CStatusBitField& bitFieldMainBoardId);

  //##ModelId=3AE691E4034E
  inline void enableStrobeGenerationJumper();
  //##ModelId=3AE7DDA503DE
  inline void disableStrobeGenerationJumper();

  inline void enableMasterInterfaceJumper();
  inline void disableMasterInterfaceJumper();


  inline void enableStrobeGeneration();
  inline void disableStrobeGeneration();
  inline BOOL checkIsStrobeGenerationEnabled();


  //##ModelId=3A8BBD6B0230
  inline void enableMasterInterface();
  //##ModelId=3A8BBD6D00C8
  inline void disableMasterInterface();
  inline BOOL checkIsMasterInterfaceEnabled();


  inline void enableFrontEndInterrupts();
  inline void disableFrontEndInterrupts();
  inline BOOL checkAreFrontEndInterruptsEnabled();


  inline void startTransfer();
  inline void stopTransfer();


  inline void selectSyncReceptionMode(DPIO2_SYNC_RECEPTION_MODE mode);
  inline void selectSyncGenerationMode(DPIO2_SYNC_GENERATION_MODE mode);

  inline void enableD0ForSync();
  inline void disableD0ForSync();

  inline void enableVideoMode();
  inline void disableVideoMode();


  //##ModelId=3ADFF1C00294
  inline void activateProgrammableOscillator();
  //##ModelId=3ADFF1C00295
  inline void activateFixedOscillator();
  inline BOOL checkIsProgrammableOscillatorActivated();


  inline void selectPrimaryStrobe();
  inline void selectSecondaryStrobe();


  //##ModelId=3A8CD6310104
  inline void enableClockingOnBothStrobeEdges();
  //##ModelId=3A8CD6310320
  inline void disableClockingOnBothStrobeEdges();
  inline BOOL checkIsClockingOnBothStrobeEdgesEnabled();


  inline void setPackingMode(DPIO2_RAW_PACKING_MODE mode);
  inline DPIO2_RAW_PACKING_MODE getPackingMode();
  inline BOOL checkIs8Bit4BitPackingAvailable();

  inline BOOL checkIsPipelineNotEmpty();
  inline void flushPackingPipeline();


  //##ModelId=3A8B9C6D003C
  inline STATUS selectDataSwapMode(DPIO2_DATA_SWAP_MODE mode);


  inline void enableRes1AsOutput();
  inline void disableRes1AsOutput();
  inline void setRes1OutputHigh();
  inline void setRes1OutputLow();
  inline BOOL checkIsRes1InputHigh();

  inline void enableRes2AsOutput();
  inline void disableRes2AsOutput();
  inline void setRes2OutputHigh();
  inline void setRes2OutputLow();
  inline BOOL checkIsRes2InputHigh();

  inline void enableRes3AsOutput();
  inline void disableRes3AsOutput();
  inline void setRes3OutputHigh();
  inline void setRes3OutputLow();
  inline BOOL checkIsRes3InputHigh();

  inline void enablePio1AsOutput();
  inline void disablePio1AsOutput();
  inline void setPio1OutputHigh();
  inline void setPio1OutputLow();
  inline BOOL checkIsPio1InputHigh();

  inline void enablePio2AsOutput();
  inline void disablePio2AsOutput();
  inline void setPio2OutputHigh();
  inline void setPio2OutputLow();
  inline BOOL checkIsPio2InputHigh();


  inline BOOL checkIsDirSignalHigh();


  inline BOOL checkIsSuspendSignalHigh();


  //##ModelId=3A8BC6E0037A
  inline void enableStrobePll();
  //##ModelId=3A8BC7F300F0
  inline void disableStrobePll();


  inline UINT8 getPersonalityId();
  inline UINT8 getMainBoardId();


  //##ModelId=3ADFF1C00296
  inline void setDirectionAsIn();
  //##ModelId=3ADFF1C00297
  inline void setDirectionAsOut();


  inline void enableDvalidAlwaysAsserted();
  inline void disableDvalidAlwaysAsserted();


  inline void setInitialSkipCounter(UINT32 value);
  inline void setSkipCounter(UINT32 value);
  inline void setReceiveCounter(UINT32 value);
  inline void setSyncCounter(UINT32 value);

  inline UINT32 getInitialSkipCounter();
  inline UINT32 getSkipCounter();
  inline UINT32 getReceiveCounter();
  inline UINT32 getSyncCounter();

  inline void enableCounterAddressing();
  inline void disableCounterAddressing();

  inline void setPllOption0(DPIO2_PLL_OPTION option);
  inline void setPllOption1(DPIO2_PLL_OPTION option);
  inline void setPllFrequencySelect(DPIO2_PLL_OPTION option);

  inline DPIO2_PLL_OPTION getPllOption0();
  inline DPIO2_PLL_OPTION getPllOption1();
  inline DPIO2_PLL_OPTION getPllFrequencySelect();

  inline void setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION option);
  inline DPIO2_PLL_OPTION getPersonalityPllFrequencySelect();

  inline BOOL checkIsFpdp2Supported();

  inline STATUS enableTestPatternGeneration(DPIO2_TEST_PATTERN pattern);
  inline void disableTestPatternGeneration();

  inline void enableForcedSuspendAssertion();
  inline void disableForcedSuspendAssertion();

  inline void activateInternalFifoReset();
  inline void deactivateInternalFifoReset();


protected:

  CControlBit m_bitEnableStrobeGeneration;
  CControlBit m_bitActivateFpdpTransfer;
  CControlBit m_bitEnableSecondaryStrobe;
  CControlBit m_bitRes1OutputEnable;
  CControlBit m_bitRes1OutputValue;
  CControlBit m_bitRes2OutputEnable;
  CControlBit m_bitRes2OutputValue;
  //##ModelId=3A9383CA0212
  CControlBit m_bitRes3OutputEnable;
  //##ModelId=3A9383CE028A
  CControlBit m_bitRes3OutputValue;
  CControlBit m_bitPio1OutputEnable;
  CControlBit m_bitPio1OutputValue;
  CControlBit m_bitPio2OutputEnable;
  CControlBit m_bitPio2OutputValue;
  CControlBit m_bitEnableInterrupts;
  //##ModelId=3A92625C00FA
  CControlBit m_bitEnableWordSwap;
  //##ModelId=3A926484008C
  CControlBit m_bitEnableMasterInterface;
  //##ModelId=3A92664B01A4
  CControlBit m_bitEnablePll;
  //##ModelId=3A9267990096
  CControlBit m_bitEnableClockingOnBothStrobeEdges;
  //##ModelId=3A926B120186
  CControlBit m_bitAssertSuspend;
  //##ModelId=3A93701D005A
  CControlBit m_bitEnablePioDecoding;
  //##ModelId=3A9370A20154
  CControlBit m_bitEnableCounterDecoding;
  //##ModelId=3AB0DA500064
  CControlBit m_bitDisableJumperForStrobeGeneration;
  //##ModelId=3AB0DA5000D3
  CControlBit m_bitDisableJumperForMasterInterface;
  //##ModelId=3ADFF1C001F5
  CControlBit m_bitEnableProgrammableOscillator;
  CControlBit m_bitFlushPackingPipeline;
  CControlBit m_bitEnableVideoMode;
  CControlBit m_bitEnable8BitSwap;
  CControlBit m_bitEnable16BitSwap;
  CControlBit m_bitEnableD0ForSync;
  CControlBit m_bitAssumeDvalidAlwaysAsserted;
  CControlBit m_bitPllOpt1Mid;
  CControlBit m_bitPllOpt1High;
  CControlBit m_bitPllOpt0Mid;
  CControlBit m_bitPllOpt0High;
  CControlBit m_bitPllFsMid;
  CControlBit m_bitPllFsHigh;
  CControlBit m_bitEnableTestPatternGeneration;
  CControlBit m_bitFlushInternalFifos;
  CControlBit m_bitPersonalityPllOutputEnable;
  CControlBit m_bitPersonalityPllOutputValue;

  //##ModelId=3A936E2C0122
  CControlBitField m_bitFieldPioAddress;
  //##ModelId=3A936EAF000A
  CControlBitField m_bitFieldPioAddressMask;
  CControlBitField m_bitFieldSyncMode;
  CControlBitField m_bitFieldPackingMode;
  CControlBitField m_bitFieldTestPattern;

  //##ModelId=3A93C74D0334
  CControlQuadByte m_initialSkipCounter;
  //##ModelId=3A93C75201B8
  CControlQuadByte m_receiveCounter;
  //##ModelId=3A93C759012C
  CControlQuadByte m_skipCounter;
  //##ModelId=3A93C75E012C
  CControlQuadByte m_syncCounter;

  CStatusBit m_bitDataInPipeLine;
  CStatusBit m_bitRes1InputValue;
  CStatusBit m_bitRes2InputValue;
  CStatusBit m_bitRes3InputValue;
  CStatusBit m_bitPio1InputValue;
  CStatusBit m_bitPio2InputValue;
  CStatusBit m_bitDirInputValue;
  //##ModelId=3A9389A400B4
  CStatusBit m_bitNrdyInputValue;
  //##ModelId=3A9389D20104
  CStatusBit m_bitSuspendInputValue;
  CStatusBit m_bitStrobeGenerationIsEnabled;
  CStatusBit m_bitMasterInterfaceIsEnabled;
  CStatusBit m_bit8Bit4BitPackingAvailable;
  CStatusBit m_bitFpdp2IsSupported;

  CStatusBitField m_bitFieldPersonalityId;
  CStatusBitField m_bitFieldMainBoardId;


};


//##ModelId=3AE691E4034E
void CDpio2FrontEnd::enableStrobeGenerationJumper()
{

  m_bitDisableJumperForStrobeGeneration.clear();

}


//##ModelId=3AE7DDA503DE
void CDpio2FrontEnd::disableStrobeGenerationJumper()
{

  m_bitDisableJumperForStrobeGeneration.set();

}


void CDpio2FrontEnd::enableMasterInterfaceJumper()
{

  m_bitDisableJumperForMasterInterface.clear();

}


void CDpio2FrontEnd::disableMasterInterfaceJumper()
{

  m_bitDisableJumperForMasterInterface.set();

}


void CDpio2FrontEnd::enableStrobeGeneration()
{

  m_bitEnableStrobeGeneration.set();

}


void CDpio2FrontEnd::disableStrobeGeneration()
{

  m_bitEnableStrobeGeneration.clear();

}


BOOL CDpio2FrontEnd::checkIsStrobeGenerationEnabled()
{

  return (m_bitEnableStrobeGeneration.checkIsSet());

}


//##ModelId=3A8BBD6B0230
void CDpio2FrontEnd::enableMasterInterface()
{

  m_bitEnableMasterInterface.set();

}


//##ModelId=3A8BBD6D00C8
void CDpio2FrontEnd::disableMasterInterface()
{

  m_bitEnableMasterInterface.clear();

}


BOOL CDpio2FrontEnd::checkIsMasterInterfaceEnabled()
{

  return (m_bitMasterInterfaceIsEnabled.checkIsSet());

}


void CDpio2FrontEnd::enableFrontEndInterrupts()
{

  m_bitEnableInterrupts.set();

}


void CDpio2FrontEnd::disableFrontEndInterrupts()
{

  m_bitEnableInterrupts.clear();

}


BOOL CDpio2FrontEnd::checkAreFrontEndInterruptsEnabled()
{

  return (m_bitEnableInterrupts.checkIsSet());

}


void CDpio2FrontEnd::startTransfer()
{

  m_bitActivateFpdpTransfer.set();

}


void CDpio2FrontEnd::stopTransfer()
{

  m_bitActivateFpdpTransfer.clear();

}


void CDpio2FrontEnd::selectSyncGenerationMode(DPIO2_SYNC_GENERATION_MODE mode)
{

  m_bitFieldSyncMode.setValue(mode);

}


void CDpio2FrontEnd::selectSyncReceptionMode(DPIO2_SYNC_RECEPTION_MODE mode)
{

  UINT32   currentMode;


  /* When a SYNC reception mode is selected,
   * any previously selected EOT mode will be disabled.
   * Warn about this.
   */
  currentMode = m_bitFieldSyncMode.getValue();

  switch (currentMode) {

  case DPIO2_SYNC_MARKS_EOT:
  case DPIO2_PIO1_MARKS_EOT:
  case DPIO2_PIO2_MARKS_EOT:
  case DPIO2_RES1_MARKS_EOT:
    DEBUG_MSG(("Selecting SYNC mode %d disables EOT mode %d\n", mode, (int) currentMode), TRUE);
    break;
    
  }


  m_bitFieldSyncMode.setValue(mode);

}


void CDpio2FrontEnd::enableD0ForSync()
{

  m_bitEnableD0ForSync.set();

}


void CDpio2FrontEnd::disableD0ForSync()
{

  m_bitEnableD0ForSync.clear();

}


void CDpio2FrontEnd::enableVideoMode()
{

  m_bitEnableVideoMode.set();

}


void CDpio2FrontEnd::disableVideoMode()
{

  m_bitEnableVideoMode.clear();

}


//##ModelId=3ADFF1C00294
void CDpio2FrontEnd::activateProgrammableOscillator()
{

  m_bitEnableProgrammableOscillator.set();

}


//##ModelId=3ADFF1C00295
void CDpio2FrontEnd::activateFixedOscillator()
{

  m_bitEnableProgrammableOscillator.clear();

}


BOOL CDpio2FrontEnd::checkIsProgrammableOscillatorActivated()
{

  return (m_bitEnableProgrammableOscillator.checkIsSet());

}


void CDpio2FrontEnd::selectPrimaryStrobe()
{

  m_bitEnableSecondaryStrobe.clear();

}


void CDpio2FrontEnd::selectSecondaryStrobe()
{

  m_bitEnableSecondaryStrobe.set();

}


//##ModelId=3A8CD6310104
void CDpio2FrontEnd::enableClockingOnBothStrobeEdges()
{

  m_bitEnableClockingOnBothStrobeEdges.set();

}


//##ModelId=3A8CD6310320
void CDpio2FrontEnd::disableClockingOnBothStrobeEdges()
{

  m_bitEnableClockingOnBothStrobeEdges.clear();

}


BOOL CDpio2FrontEnd::checkIsClockingOnBothStrobeEdgesEnabled()
{

  return (m_bitEnableClockingOnBothStrobeEdges.checkIsSet());

}


void CDpio2FrontEnd::setPackingMode(DPIO2_RAW_PACKING_MODE mode)
{

  m_bitFieldPackingMode.setValue(mode);

}


DPIO2_RAW_PACKING_MODE CDpio2FrontEnd::getPackingMode()
{

  return ((DPIO2_RAW_PACKING_MODE) m_bitFieldPackingMode.getValue());

}


BOOL CDpio2FrontEnd::checkIs8Bit4BitPackingAvailable()
{

  return (m_bit8Bit4BitPackingAvailable.checkIsSet());

}


BOOL CDpio2FrontEnd::checkIsPipelineNotEmpty()
{

  return (m_bitDataInPipeLine.checkIsSet());

}


void CDpio2FrontEnd::flushPackingPipeline()
{

  m_bitFlushPackingPipeline.set();
  m_bitFlushPackingPipeline.clear();

}


//##ModelId=3A8B9C6D003C
STATUS CDpio2FrontEnd::selectDataSwapMode(DPIO2_DATA_SWAP_MODE mode)
{

  STATUS   status;


  /* Set whether the two 32 bits words in a 64 bits words shall be swapped or not.
   * Also determine whether the identifier for swap mode is valid.
   */
  if ( (mode == NO_SWAP) ||
       (mode == SWAP_8BIT) ||
       (mode == SWAP_16BIT) ||
       (mode == SWAP_8BIT_16BIT) ) {

    m_bitEnableWordSwap.clear();
    status = OK;

  } else if ( (mode == SWAP_32BIT) ||
              (mode == SWAP_8BIT_32BIT) ||
              (mode == SWAP_16BIT_32BIT) ||
              (mode == SWAP_8BIT_16BIT_32BIT) ) {

    m_bitEnableWordSwap.set();
    status = OK;

  } else {

    ERROR_MSG(("Unknown Data Swap mode (%d).\n", mode));
    status = ERROR;

  }


  /* Skip the rest if the mode identifier is invalid.
   */
  if ( status == OK ) {

    /* Set whether the two 16 bits parts in a 32 bits words shall be swapped or not.
     */
    if ( (mode == NO_SWAP) ||
         (mode == SWAP_8BIT) ||
         (mode == SWAP_32BIT) ||
         (mode == SWAP_8BIT_32BIT) ) {

      m_bitEnable16BitSwap.clear();

    } else {

      m_bitEnable16BitSwap.set();

    } 


    /* Set whether the two 8 bits parts in a 16 bits words shall be swapped or not.
     */
    if ( (mode == NO_SWAP) ||
         (mode == SWAP_16BIT) ||
         (mode == SWAP_32BIT) ||
         (mode == SWAP_16BIT_32BIT) ) {

      m_bitEnable8BitSwap.clear();

    } else {

      m_bitEnable8BitSwap.set();

    } 

  }

  return (status);

}


void CDpio2FrontEnd::enableRes1AsOutput()
{

  m_bitRes1OutputEnable.set();

}


void CDpio2FrontEnd::disableRes1AsOutput()
{

  m_bitRes1OutputEnable.clear();

}


void CDpio2FrontEnd::setRes1OutputHigh()
{

  m_bitRes1OutputValue.set();

}


void CDpio2FrontEnd::setRes1OutputLow()
{

  m_bitRes1OutputValue.clear();

}


BOOL CDpio2FrontEnd::checkIsRes1InputHigh()
{

  return (m_bitRes1InputValue.checkIsSet());

}


void CDpio2FrontEnd::enableRes2AsOutput()
{

  m_bitRes2OutputEnable.set();

}


void CDpio2FrontEnd::disableRes2AsOutput()
{

  m_bitRes2OutputEnable.clear();

}


void CDpio2FrontEnd::setRes2OutputHigh()
{

  m_bitRes2OutputValue.set();

}


void CDpio2FrontEnd::setRes2OutputLow()
{

  m_bitRes2OutputValue.clear();

}


BOOL CDpio2FrontEnd::checkIsRes2InputHigh()
{

  return (m_bitRes2InputValue.checkIsSet());

}


void CDpio2FrontEnd::enableRes3AsOutput()
{

  m_bitRes3OutputEnable.set();

}


void CDpio2FrontEnd::disableRes3AsOutput()
{

  m_bitRes3OutputEnable.clear();

}


void CDpio2FrontEnd::setRes3OutputHigh()
{

  m_bitRes3OutputValue.set();

}


void CDpio2FrontEnd::setRes3OutputLow()
{

  m_bitRes3OutputValue.clear();

}


BOOL CDpio2FrontEnd::checkIsRes3InputHigh()
{

  return (m_bitRes3InputValue.checkIsSet());

}


void CDpio2FrontEnd::enablePio1AsOutput()
{

  m_bitPio1OutputEnable.set();

}


void CDpio2FrontEnd::disablePio1AsOutput()
{

  m_bitPio1OutputEnable.clear();

}


void CDpio2FrontEnd::setPio1OutputHigh()
{

  m_bitPio1OutputValue.set();

}


void CDpio2FrontEnd::setPio1OutputLow()
{

  m_bitPio1OutputValue.clear();

}


BOOL CDpio2FrontEnd::checkIsPio1InputHigh()
{

  return (m_bitPio1InputValue.checkIsSet());

}


void CDpio2FrontEnd::enablePio2AsOutput()
{

  m_bitPio2OutputEnable.set();

}


void CDpio2FrontEnd::disablePio2AsOutput()
{

  m_bitPio2OutputEnable.clear();

}


void CDpio2FrontEnd::setPio2OutputHigh()
{

  m_bitPio2OutputValue.set();

}


void CDpio2FrontEnd::setPio2OutputLow()
{

  m_bitPio2OutputValue.clear();

}


BOOL CDpio2FrontEnd::checkIsPio2InputHigh()
{

  return (m_bitPio2InputValue.checkIsSet());

}


BOOL CDpio2FrontEnd::checkIsDirSignalHigh()
{

  return (m_bitDirInputValue.checkIsSet());

}


BOOL CDpio2FrontEnd::checkIsSuspendSignalHigh()
{

  return (m_bitSuspendInputValue.checkIsSet());

}


//##ModelId=3A8BC6E0037A
void CDpio2FrontEnd::enableStrobePll()
{

#if 0
  DEBUG_MSG(("Skipped setting PLL enable bit\n"), TRUE);
#else
  m_bitEnablePll.set();
#endif

}


//##ModelId=3A8BC7F300F0
void CDpio2FrontEnd::disableStrobePll()
{

#if 0
  DEBUG_MSG(("Skipped clearing PLL enable bit\n"), TRUE);
#else
  m_bitEnablePll.clear();
#endif

}


UINT8 CDpio2FrontEnd::getPersonalityId()
{

  return (m_bitFieldPersonalityId.getValue());

}


UINT8 CDpio2FrontEnd::getMainBoardId()
{

  return (m_bitFieldMainBoardId.getValue());

}


//##ModelId=3ADFF1C00296
void CDpio2FrontEnd::setDirectionAsIn()
{

  /* When the DPIO2 is configured as input
   * the frontend use the primary strobe by default.
   */
  selectPrimaryStrobe();

}


//##ModelId=3ADFF1C00297
void CDpio2FrontEnd::setDirectionAsOut()
{

  /* When the DPIO2 is configured as output, the frontend activates
   * the programmable oscillator as the default clock source.
   */
  activateProgrammableOscillator();

}


void CDpio2FrontEnd::enableDvalidAlwaysAsserted()
{

  m_bitAssumeDvalidAlwaysAsserted.set();

}


void CDpio2FrontEnd::disableDvalidAlwaysAsserted()
{

  m_bitAssumeDvalidAlwaysAsserted.clear();

}


void CDpio2FrontEnd::setInitialSkipCounter(UINT32 value)
{

  m_initialSkipCounter.setValue(value);

}


void CDpio2FrontEnd::setSkipCounter(UINT32 value)
{

  m_skipCounter.setValue(value);

}


void CDpio2FrontEnd::setReceiveCounter(UINT32 value)
{

  m_receiveCounter.setValue(value);
  
}


void CDpio2FrontEnd::setSyncCounter(UINT32 value)
{

  m_syncCounter.setValue(value);

}


UINT32 CDpio2FrontEnd::getInitialSkipCounter()
{

  return (m_initialSkipCounter.getValue());

}


UINT32 CDpio2FrontEnd::getSkipCounter()
{

  return (m_skipCounter.getValue());

}


UINT32 CDpio2FrontEnd::getReceiveCounter()
{

  return (m_receiveCounter.getValue());
  
}


UINT32 CDpio2FrontEnd::getSyncCounter()
{

  return (m_syncCounter.getValue());

}


void CDpio2FrontEnd::enableCounterAddressing()
{

  m_bitEnableCounterDecoding.set();

}


void CDpio2FrontEnd::disableCounterAddressing()
{

  m_bitEnableCounterDecoding.clear();

}


void CDpio2FrontEnd::setPllOption0(DPIO2_PLL_OPTION option)
{

  switch (option) {

  case DPIO2_PLL_OPTION_LOW:
    m_bitPllOpt0Mid.clear();
    m_bitPllOpt0High.clear();
    break;

  case DPIO2_PLL_OPTION_MID:
    m_bitPllOpt0Mid.set();
    break;

  case DPIO2_PLL_OPTION_HIGH:
    m_bitPllOpt0Mid.clear();
    m_bitPllOpt0High.set();
    break;

  }

}


void CDpio2FrontEnd::setPllOption1(DPIO2_PLL_OPTION option)
{

  switch (option) {

  case DPIO2_PLL_OPTION_LOW:
    m_bitPllOpt1Mid.clear();
    m_bitPllOpt1High.clear();
    break;

  case DPIO2_PLL_OPTION_MID:
    m_bitPllOpt1Mid.set();
    break;

  case DPIO2_PLL_OPTION_HIGH:
    m_bitPllOpt1Mid.clear();
    m_bitPllOpt1High.set();
    break;

  }

}


void CDpio2FrontEnd::setPllFrequencySelect(DPIO2_PLL_OPTION option)
{

  switch (option) {

  case DPIO2_PLL_OPTION_LOW:
    m_bitPllFsMid.clear();
    m_bitPllFsHigh.clear();
    break;

  case DPIO2_PLL_OPTION_MID:
    m_bitPllFsMid.set();
    break;

  case DPIO2_PLL_OPTION_HIGH:
    m_bitPllFsMid.clear();
    m_bitPllFsHigh.set();
    break;

  }

}


DPIO2_PLL_OPTION CDpio2FrontEnd::getPllOption0()
{

  DPIO2_PLL_OPTION   currentOption;

  if ( m_bitPllOpt0Mid.checkIsSet() ) {

    currentOption = DPIO2_PLL_OPTION_MID;

  } else {

    if ( m_bitPllOpt0High.checkIsSet() ) {

      currentOption = DPIO2_PLL_OPTION_HIGH;

    } else {

      currentOption = DPIO2_PLL_OPTION_LOW;

    }

  }

  return (currentOption);

}


DPIO2_PLL_OPTION CDpio2FrontEnd::getPllOption1()
{

  DPIO2_PLL_OPTION   currentOption;

  if ( m_bitPllOpt1Mid.checkIsSet() ) {

    currentOption = DPIO2_PLL_OPTION_MID;

  } else {

    if ( m_bitPllOpt1High.checkIsSet() ) {

      currentOption = DPIO2_PLL_OPTION_HIGH;

    } else {

      currentOption = DPIO2_PLL_OPTION_LOW;

    }

  }

  return (currentOption);

}


DPIO2_PLL_OPTION CDpio2FrontEnd::getPllFrequencySelect()
{

  DPIO2_PLL_OPTION   currentOption;

  if ( m_bitPllFsMid.checkIsSet() ) {

    currentOption = DPIO2_PLL_OPTION_MID;

  } else {

    if ( m_bitPllFsHigh.checkIsSet() ) {

      currentOption = DPIO2_PLL_OPTION_HIGH;

    } else {

      currentOption = DPIO2_PLL_OPTION_LOW;

    }

  }

  return (currentOption);

}


void CDpio2FrontEnd::setPersonalityPllFrequencySelect(DPIO2_PLL_OPTION option)
{

  switch (option) {

  case DPIO2_PLL_OPTION_LOW:
    m_bitPersonalityPllOutputEnable.set();
    m_bitPersonalityPllOutputValue.clear();
    break;

  case DPIO2_PLL_OPTION_MID:
    m_bitPersonalityPllOutputEnable.clear();
    break;

  case DPIO2_PLL_OPTION_HIGH:
    m_bitPersonalityPllOutputEnable.set();
    m_bitPersonalityPllOutputValue.set();
    break;

  }

}


DPIO2_PLL_OPTION CDpio2FrontEnd::getPersonalityPllFrequencySelect()
{

  DPIO2_PLL_OPTION   currentOption;

  if ( m_bitPersonalityPllOutputEnable.checkIsSet() ) {

    if ( m_bitPersonalityPllOutputValue.checkIsSet() ) {

      currentOption = DPIO2_PLL_OPTION_HIGH;

    } else {

      currentOption = DPIO2_PLL_OPTION_LOW;

    }

  } else {

    currentOption = DPIO2_PLL_OPTION_MID;

  }

  return (currentOption);

}


BOOL CDpio2FrontEnd::checkIsFpdp2Supported()
{

  return (m_bitFpdp2IsSupported.checkIsSet());

}


STATUS CDpio2FrontEnd::enableTestPatternGeneration(DPIO2_TEST_PATTERN pattern)
{

  STATUS   status;


  /* When the COUNTER_PATTERN_WITH_PROGRAMMABLE_START is specified,
   * the 16 LSB bits of the initialSkipCounter is used as start value
   * for the count pattern.
   */

  switch ( pattern ) {

  case WALKING_ONE_PATTERN:
  case WALKING_ZERO_PATTERN:
  case COUNTER_PATTERN:
  case COUNTER_PATTERN_WITH_PROGRAMMABLE_START:
    m_bitFieldTestPattern.setValue((UINT32) pattern);
    m_bitEnableTestPatternGeneration.set();
    status = OK;
    break;

  default:
    ERROR_MSG(("%d is not a valid Test Pattern ID\n", (int) pattern));
    status = ERROR;
    break;

  }

  return (status);

}


void CDpio2FrontEnd::disableTestPatternGeneration()
{

  m_bitEnableTestPatternGeneration.clear();

}


void CDpio2FrontEnd::enableForcedSuspendAssertion()
{

  m_bitAssertSuspend.set();

}


void CDpio2FrontEnd::disableForcedSuspendAssertion()
{

  m_bitAssertSuspend.clear();

}


void CDpio2FrontEnd::activateInternalFifoReset()
{

  m_bitFlushInternalFifos.set();

}


void CDpio2FrontEnd::deactivateInternalFifoReset()
{

  m_bitFlushInternalFifos.clear();

}


#endif /* CDPIO2FRONTEND_H_INCLUDED_C5699E4E */



