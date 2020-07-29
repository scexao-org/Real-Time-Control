/****************************************************************************
Module      : CDpio2Fifo.h

Description : Header file declaring the CDpio2Fifo class.
              A CDpio2Fifo object represents functionality related to
              the FIFO of a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01d,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01c,18mar2002,nib  Updated file for use in a MCOS environment.
  01b,26oct2001,nib  Updated file for use in a Windows environment.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CDPIO2FIFO_H_INCLUDED_C569EC14
#define CDPIO2FIFO_H_INCLUDED_C569EC14

#if defined(WIN32)

#include "CWControlBit.h"
#include "CWStatusBit.h"
#include "CWStatusBitField.h"

#elif defined (MCOS)

#include "CMControlBit.h"
#include "CStatusBit.h"
#include "CStatusBitField.h"

#elif defined(VMOSA)

#include "vmosaCControlBit.h"
#include "vmosaCStatusBit.h"
#include "vmosaCStatusBitField.h"
#include "Dpio2Defs.h"

#else

#include "CControlBit.h"
#include "CStatusBit.h"
#include "CStatusBitField.h"

#endif

#include "DpioCommon.h"


//##ModelId=3A93B390023A
class CDpio2Fifo
{
  public:

    //##ModelId=3A9A85B20370
    CDpio2Fifo(const CControlBit& bitDisableSuspendFlowControl,
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
               const CStatusBitField& bitFieldFifoSize);

    //##ModelId=3ADFF1C0018C
    inline UINT32 getFifoSize();

    //##ModelId=3A93B42202D0
    inline void flushFifo();

    //##ModelId=3A9A6F120226
    inline DPIO_FIFO_STATE getFifoState();

    inline void resetHistoryFlags();

    inline BOOL checkHasBeenFull();
    //##ModelId=3A9A7CD702D0
    inline BOOL checkHasBeenAlmostFull();
    inline BOOL checkHasBeenMoreThanHalfFull();
    //##ModelId=3A9A7CD7029E
    inline BOOL checkHasBeenAlmostEmpty();
    inline BOOL checkHasBeenEmpty();
    //##ModelId=3A9A7CD7033E
    inline BOOL checkHasOverflowed();

    inline void enableSuspendFlowControl();
    inline void disableSuspendFlowControl();

    inline void enableNrdyFlowControl();
    inline void disableNrdyFlowControl();

    inline void enable100MhzFifoClock();
    inline void disable100MhzFifoClock();

  protected:

    CControlBit m_bitDisableSuspendFlowControl;
    CControlBit m_bitDisableNrdy;
    CControlBit m_bitResetFlags;
    //##ModelId=3A93B3B602A8
    CControlBit m_bitReset;
    CControlBit m_bitEnable100MhzFifoClock;


    CStatusBit m_bitFifoHasBeenFull;
    CStatusBit m_bitFifoHasBeenHalfFull;
    CStatusBit m_bitFifoHasBeenEmpty;
    //##ModelId=3A93A5B301C2
    CStatusBit m_bitFifoHasBeenAlmostEmpty;
    //##ModelId=3A93A5AF0258
    CStatusBit m_bitFifoHasBeenAlmostFull;
    //##ModelId=3A938E1A02A8
    CStatusBit m_bitFifoOverflowOccurred;


    CStatusBitField m_bitFieldFifoState;
    //##ModelId=3ADFF1C00187
    CStatusBitField m_bitFieldFifoSize;

};


//##ModelId=3ADFF1C0018C
UINT32 CDpio2Fifo::getFifoSize()
{

  UINT32   codedSize;
  UINT32   size;


  codedSize = m_bitFieldFifoSize.getValue();

  if ( codedSize == DPIO2_FIFO_SIZE_32K ) {

    size = 32 * 1024;

  } else {

    size = 128 * 1024;

  }


  return (size);

}


//##ModelId=3A93B42202D0
void CDpio2Fifo::flushFifo()
{

  m_bitReset.set();
  m_bitReset.clear();

}


//##ModelId=3A9A6F120226
DPIO_FIFO_STATE CDpio2Fifo::getFifoState()
{

  UINT32   value;

  DPIO_FIFO_STATE   fifoState;


  value = m_bitFieldFifoState.getValue();

  if ( (value & DPIO2_FIFO_STATE_EMPTY_BITMASK) != 0 ) {

    fifoState = DPIO_FIFO_EMPTY;

  } else if ( (value & DPIO2_FIFO_STATE_ALMOST_EMPTY_BITMASK) != 0 ) {

    fifoState = DPIO_FIFO_ALMOST_EMPTY;

  } else if ( (value & DPIO2_FIFO_STATE_FULL_BITMASK) != 0 ) {

    fifoState = DPIO_FIFO_FULL;

  } else if ( (value & DPIO2_FIFO_STATE_ALMOST_FULL_BITMASK) != 0 ) {

    fifoState = DPIO_FIFO_ALMOST_FULL;

  } else if ( (value & DPIO2_FIFO_STATE_HALF_FULL_BITMASK) != 0 ) {

    fifoState = DPIO_FIFO_GREATER_THAN_HALF_FULL;

  } else {

    fifoState = DPIO_FIFO_LESS_THAN_HALF_FULL;

  }


  return (fifoState);

}


void CDpio2Fifo::resetHistoryFlags()
{

  m_bitResetFlags.clear();
  m_bitResetFlags.set();

}


BOOL CDpio2Fifo::checkHasBeenFull()
{

  return (m_bitFifoHasBeenFull.checkIsSet());

}


//##ModelId=3A9A7CD702D0
BOOL CDpio2Fifo::checkHasBeenAlmostFull()
{

  return (m_bitFifoHasBeenAlmostFull.checkIsSet());

}


BOOL CDpio2Fifo::checkHasBeenMoreThanHalfFull()
{

  return (m_bitFifoHasBeenHalfFull.checkIsSet());

}


//##ModelId=3A9A7CD7029E
BOOL CDpio2Fifo::checkHasBeenAlmostEmpty()
{

  return (m_bitFifoHasBeenAlmostEmpty.checkIsSet());

}


BOOL CDpio2Fifo::checkHasBeenEmpty()
{

  return (m_bitFifoHasBeenEmpty.checkIsSet());

}


//##ModelId=3A9A7CD7033E
BOOL CDpio2Fifo::checkHasOverflowed()
{

  return (m_bitFifoOverflowOccurred.checkIsSet());

}


void CDpio2Fifo::enableSuspendFlowControl()
{
  
  m_bitDisableSuspendFlowControl.clear();

}


void CDpio2Fifo::disableSuspendFlowControl()
{
  
  m_bitDisableSuspendFlowControl.set();

}


void CDpio2Fifo::enableNrdyFlowControl()
{
  
  m_bitDisableNrdy.clear();

}


void CDpio2Fifo::disableNrdyFlowControl()
{
  
  m_bitDisableNrdy.set();

}


void CDpio2Fifo::enable100MhzFifoClock()
{

  m_bitEnable100MhzFifoClock.set();

}


void CDpio2Fifo::disable100MhzFifoClock()
{

  m_bitEnable100MhzFifoClock.clear();

}

#endif /* CDPIO2FIFO_H_INCLUDED_C569EC14 */
