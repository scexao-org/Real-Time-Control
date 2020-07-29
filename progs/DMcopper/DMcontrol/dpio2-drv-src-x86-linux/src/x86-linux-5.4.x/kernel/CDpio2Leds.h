/****************************************************************************
Module      : CDpio2Leds.h

Description : Header file declaring the CDpio2Leds class.
              A CDpio2Leds object represents the functionality
              for controlling the LEDs on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01e,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01d,18mar2002,nib  Updated file for use in a MCOS environment.
  01c,26feb2002,nib  Added functionality for forcing enabled LEDs on.
  01b,26oct2001,nib  Updated file for use in a Windows environment.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CDPIO2LEDS_H_INCLUDED_C569E15A
#define CDPIO2LEDS_H_INCLUDED_C569E15A

#if defined(WIN32)

#include "CWControlBit.h"

#elif defined(MCOS)

#include "CMControlBit.h"

#elif defined(VMOSA)

#include "vmosaCControlBit.h"

#else

#include "CControlBit.h"

#endif

//##ModelId=3A94C6C502D0
class CDpio2Leds
{

  public:
    //##ModelId=3ADFF1C00298
    CDpio2Leds(const CControlBit& bitEnableNrdyLed,
	       const CControlBit& bitEnableClkLed,
	       const CControlBit& bitEnableSuspendLed,
	       const CControlBit& bitEnableSyncLed,
	       const CControlBit& bitEnableDvalidLed,
               const CControlBit& bitEnableLedTest);

    //##ModelId=3ADFF1C00370
    inline void enableNrdyLed();
    //##ModelId=3ADFF1C00371
    inline void disableNrdyLed();

    //##ModelId=3ADFF1C00372
    inline void enableClkLed();
    //##ModelId=3ADFF1C00373
    inline void disableClkLed();

    //##ModelId=3ADFF1C00374
    inline void enableSuspendLed();
    //##ModelId=3ADFF1C00375
    inline void disableSuspendLed();

    //##ModelId=3ADFF1C00376
    inline void enableSyncLed();
    //##ModelId=3ADFF1C003AC
    inline void disableSyncLed();

    //##ModelId=3ADFF1C003AD
    inline void enableDvalidLed();
    //##ModelId=3ADFF1C003AE
    inline void disableDvalidLed();

    inline void enableLedTest();
    inline void disableLedTest();


  protected:
    //##ModelId=3A94C82700A0
    CControlBit m_bitEnableNrdyLed;

    //##ModelId=3A94C86800FA
    CControlBit m_bitEnableClkLed;

    //##ModelId=3A94C86B01EA
    CControlBit m_bitEnableSuspendLed;

    //##ModelId=3A94C86E0348
    CControlBit m_bitEnableSyncLed;

    //##ModelId=3A94C87201D6
    CControlBit m_bitEnableDvalidLed;

    CControlBit m_bitEnableLedTest;

};


//##ModelId=3ADFF1C00370
void CDpio2Leds::enableNrdyLed()
{

  m_bitEnableNrdyLed.set();

}

//##ModelId=3ADFF1C00371
void CDpio2Leds::disableNrdyLed()
{

  m_bitEnableNrdyLed.clear();

}

//##ModelId=3ADFF1C00372
void CDpio2Leds::enableClkLed()
{

  m_bitEnableClkLed.set();

}

//##ModelId=3ADFF1C00373
void CDpio2Leds::disableClkLed()
{

  m_bitEnableClkLed.clear();

}


//##ModelId=3ADFF1C00374
void CDpio2Leds::enableSuspendLed()
{

  m_bitEnableSuspendLed.set();

}

//##ModelId=3ADFF1C00375
void CDpio2Leds::disableSuspendLed()
{

  m_bitEnableSuspendLed.clear();

}


//##ModelId=3ADFF1C00376
void CDpio2Leds::enableSyncLed()
{

  m_bitEnableSyncLed.set();

}

//##ModelId=3ADFF1C003AC
void CDpio2Leds::disableSyncLed()
{

  m_bitEnableSyncLed.clear();

}


//##ModelId=3ADFF1C003AD
void CDpio2Leds::enableDvalidLed()
{

  m_bitEnableDvalidLed.set();

}

//##ModelId=3ADFF1C003AE
void CDpio2Leds::disableDvalidLed()
{

  m_bitEnableDvalidLed.clear();

}


void CDpio2Leds::enableLedTest()
{

  m_bitEnableLedTest.set();
  
}


void CDpio2Leds::disableLedTest()
{

  m_bitEnableLedTest.clear();
  
}


#endif /* CDPIO2LEDS_H_INCLUDED_C569E15A */


