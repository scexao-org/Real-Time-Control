/****************************************************************************
Module      : CDpio2Fifo.cpp

Description : Source file defining the CDpio2Fifo class.
              A CDpio2Fifo object represents the functionality related to
              the FIFO of a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,22jun2001,nib  Created.

*****************************************************************************/

#include "Dpio2Defs.h"

#include "CDpio2Fifo.h"

//##ModelId=3A9A85B20370
CDpio2Fifo::CDpio2Fifo(const CControlBit& bitDisableSuspendFlowControl,
		       const CControlBit& bitDisableNrdy,
		       const CControlBit& bitResetFlags,
		       const CControlBit& bitReset,
                       const CControlBit& bitEnable100MhzFifoClock,
		       const CStatusBit& bitFifoHasBeenFull,
		       const CStatusBit& bitFifoHasBeenHalfFull,
		       const CStatusBit& bitFifoHasBeenEmpty,
		       const CStatusBit& bitFifoHasBeenAlmostEmpty,
		       const CStatusBit& bitFifoHasBeenAlmostFull,
		       const CStatusBit& bitFifoOverflowOccurred,
		       const CStatusBitField& bitFieldFifoState,
		       const CStatusBitField& bitFieldFifoSize)
  : m_bitDisableSuspendFlowControl(bitDisableSuspendFlowControl),
    m_bitDisableNrdy(bitDisableNrdy),
    m_bitResetFlags(bitResetFlags),
    m_bitReset(bitReset),
    m_bitEnable100MhzFifoClock(bitEnable100MhzFifoClock),

    m_bitFifoHasBeenFull(bitFifoHasBeenFull),
    m_bitFifoHasBeenHalfFull(bitFifoHasBeenHalfFull),
    m_bitFifoHasBeenEmpty(bitFifoHasBeenEmpty),
    m_bitFifoHasBeenAlmostEmpty(bitFifoHasBeenAlmostEmpty),
    m_bitFifoHasBeenAlmostFull(bitFifoHasBeenAlmostFull),
    m_bitFifoOverflowOccurred(bitFifoOverflowOccurred),

    m_bitFieldFifoState(bitFieldFifoState),
    m_bitFieldFifoSize(bitFieldFifoSize)
{
}
