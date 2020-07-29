/****************************************************************************
Module      : CDpio2FrontEnd.cpp

Description : Source file defining the CDpio2FrontEnd class.
              A CDpio2FrontEnd object represents the functionality related to
              the front end of a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01h,02oct2002,nib  Renamed TTL and PECL strobe to primary and secondary strobe.
  01g,22may2002,nib  Added support for PLL on PECL and RS422 personality modules.
  01f,08apr2002,nib  Added support for flushing internal FIFOs in the Front End.
  01e,19feb2002,nib  Added support for test pattern generation.
  01d,17dec2001,nib  Added support for PLL options.
  01c,24sep2001,nib  Added bitEnableD0ForSync to constructor.
  01b,19jul2001,nib  Implemented support for 8 and 16 bit swapping.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#include "CDpio2FrontEnd.h"

//##ModelId=3AB8BB0803CA
CDpio2FrontEnd::CDpio2FrontEnd(const CControlBit& bitEnableStrobeGeneration,
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
                               const CControlBit& bitDisableJumperForStrobeGeneration,
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
                               const CStatusBitField& bitFieldMainBoardId)

  : m_bitEnableStrobeGeneration(bitEnableStrobeGeneration),
    m_bitActivateFpdpTransfer(bitActivateFpdpTransfer),
    m_bitEnableSecondaryStrobe(bitEnableSecondaryStrobe),
    m_bitRes1OutputEnable(bitRes1OutputEnable),
    m_bitRes1OutputValue(bitRes1OutputValue),
    m_bitRes2OutputEnable(bitRes2OutputEnable),
    m_bitRes2OutputValue(bitRes2OutputValue),
    m_bitRes3OutputEnable(bitRes3OutputEnable),
    m_bitRes3OutputValue(bitRes3OutputValue),
    m_bitPio1OutputEnable(bitPio1OutputEnable),
    m_bitPio1OutputValue(bitPio1OutputValue),
    m_bitPio2OutputEnable(bitPio2OutputEnable),
    m_bitPio2OutputValue(bitPio2OutputValue),
    m_bitEnableInterrupts(bitEnableInterrupts),
    m_bitEnableWordSwap(bitEnableWordSwap),
    m_bitEnableMasterInterface(bitEnableMasterInterface),
    m_bitEnablePll(bitEnablePll),
    m_bitEnableClockingOnBothStrobeEdges(bitEnableClockingOnBothStrobeEdges),
    m_bitAssertSuspend(bitAssertSuspend),
    m_bitEnablePioDecoding(bitEnablePioDecoding),
    m_bitEnableCounterDecoding(bitEnableCounterDecoding),
    m_bitDisableJumperForStrobeGeneration(bitDisableJumperForStrobeGeneration),
    m_bitDisableJumperForMasterInterface(bitDisableJumperForMasterInterface),
    m_bitEnableProgrammableOscillator(bitEnableProgrammableOscillator),
    m_bitFlushPackingPipeline(bitFlushPackingPipeline),
    m_bitEnableVideoMode(bitEnableVideoMode),
    m_bitEnable8BitSwap(bitEnable8BitSwap),
    m_bitEnable16BitSwap(bitEnable16BitSwap),
    m_bitEnableD0ForSync(bitEnableD0ForSync),
    m_bitAssumeDvalidAlwaysAsserted(bitAssumeDvalidAlwaysAsserted),
    m_bitPllOpt1Mid(bitPllOpt1Mid),
    m_bitPllOpt1High(bitPllOpt1High),
    m_bitPllOpt0Mid(bitPllOpt0Mid),
    m_bitPllOpt0High(bitPllOpt0High),
    m_bitPllFsMid(bitPllFsMid),
    m_bitPllFsHigh(bitPllFsHigh),
    m_bitEnableTestPatternGeneration(bitEnableTestPatternGeneration),
    m_bitFlushInternalFifos(bitFlushInternalFifos),
    m_bitPersonalityPllOutputEnable(bitPersonalityPllOutputEnable),
    m_bitPersonalityPllOutputValue(bitPersonalityPllOutputValue),

    m_bitFieldPioAddress(bitFieldPioAddress),
    m_bitFieldPioAddressMask(bitFieldPioAddressMask),
    m_bitFieldSyncMode(bitFieldSyncMode),
    m_bitFieldPackingMode(bitFieldPackingMode),
    m_bitFieldTestPattern(bitFieldTestPattern),

    m_initialSkipCounter(initialSkipCounter),
    m_receiveCounter(receiveCounter),
    m_skipCounter(skipCounter),
    m_syncCounter(syncCounter),

    m_bitDataInPipeLine(bitDataInPipeLine),
    m_bitRes1InputValue(bitRes1InputValue),
    m_bitRes2InputValue(bitRes2InputValue),
    m_bitRes3InputValue(bitRes3InputValue),
    m_bitPio1InputValue(bitPio1InputValue),
    m_bitPio2InputValue(bitPio2InputValue),
    m_bitDirInputValue(bitDirInputValue),
    m_bitNrdyInputValue(bitNrdyInputValue),
    m_bitSuspendInputValue(bitSuspendInputValue),
    m_bitStrobeGenerationIsEnabled(bitStrobeGenerationIsEnabled),
    m_bitMasterInterfaceIsEnabled(bitMasterInterfaceIsEnabled),
    m_bit8Bit4BitPackingAvailable(bit8Bit4BitPackingAvailable),
    m_bitFpdp2IsSupported(bitFpdp2IsSupported),

    m_bitFieldPersonalityId(bitFieldPersonalityId),
    m_bitFieldMainBoardId(bitFieldMainBoardId)
{

}
