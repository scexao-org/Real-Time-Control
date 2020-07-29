/****************************************************************************
Module      : CDpio2Leds.cpp

Description : Source file defining the CDpio2Leds class.
              A CDpio2Leds object represents the functionality
              for controlling the LEDs on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01b,26feb2002,nib  Added functionality for forcing enabled LEDs on.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#include "CDpio2Leds.h"

//##ModelId=3ADFF1C00298
CDpio2Leds::CDpio2Leds(const CControlBit& bitEnableNrdyLed,
		       const CControlBit& bitEnableClkLed,
		       const CControlBit& bitEnableSuspendLed,
		       const CControlBit& bitEnableSyncLed,
		       const CControlBit& bitEnableDvalidLed,
                       const CControlBit& bitEnableLedTest)
  : m_bitEnableNrdyLed(bitEnableNrdyLed),
    m_bitEnableClkLed(bitEnableClkLed),
    m_bitEnableSuspendLed(bitEnableSuspendLed),
    m_bitEnableSyncLed(bitEnableSyncLed),
    m_bitEnableDvalidLed(bitEnableDvalidLed),
    m_bitEnableLedTest(bitEnableLedTest)
{
}
