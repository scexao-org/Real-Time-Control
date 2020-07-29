/****************************************************************************
Module      : CInterruptSource.h

Description : Header file declaring the CInterruptSource class.
              A CInterruptSource object represents an interrupt source,
              and offers methods for enabling the source, disabling the 
              source, and checking whether the source has generated an
              interrupt.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01e,05sep2005,meh  Added extra read after clearing interrupt register.
  01d,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01c,18mar2002,nib  Updated file for use in a MCOS environment.
  01b,27oct2001,nib  Updated file for use in a Windows environment.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CINTERRUPTSOURCE_H_INCLUDED_C54F2C77
#define CINTERRUPTSOURCE_H_INCLUDED_C54F2C77


#if defined(WIN32)

#include "CWControlBit.h"
#include "CWStatusBit.h"

#elif defined(MCOS)

#include "CMControlBit.h"
#include "CStatusBit.h"

#elif defined(VMOSA)

#include "vmosaCControlBit.h"
#include "vmosaCStatusBit.h"

#else

#include "CControlBit.h"
#include "CStatusBit.h"

#endif


//##ModelId=3AADCFE2014A
class CInterruptSource
{
  public:

    //##ModelId=3AB0B5C800A0
    CInterruptSource(const CControlBit& bitEnable,
                     const CStatusBit& bitActiveFlag);

    //##ModelId=3AADD46C033E
    inline void enable();

    //##ModelId=3AADD46D0032
    inline void disable();

    inline BOOL checkIsEnabled();


    //##ModelId=3AADD489000A
    //##Documentation
    //## Clears interrupt from this interrupt source.
    //## 
    //## For interrupt sources where interrupts are cleared by disabling the
    //## source, this method will only clear a interrupt if the source is still
    //## enabled (actions executed for the source may have disabled further
    //## interrupts).
    inline void clear();

    inline BOOL checkIsActive();


  private:

    //##ModelId=3AB0B5C703DE
    CControlBit   m_bitEnable;

    CStatusBit   m_bitActiveFlag;

};


//##ModelId=3AADD46C033E
void CInterruptSource::enable()
{
  
  m_bitEnable.set();

}

//##ModelId=3AADD46D0032
void CInterruptSource::disable()
{
  
  m_bitEnable.clear();

}


BOOL CInterruptSource::checkIsEnabled()
{

  return (m_bitEnable.checkIsSet());

}


//##ModelId=3AADD489000A
void CInterruptSource::clear()
{

  /* Only toggle this bit if it is set,
   * because an interrupt will already be cleared if the enable bit is cleared.
   */
  if ( m_bitEnable.checkIsSet() ) {

    m_bitEnable.clear();
    m_bitEnable.set();
    m_bitEnable.checkIsSet();

  }

}


BOOL CInterruptSource::checkIsActive()
{

  return (m_bitActiveFlag.checkIsSet());

}


#endif /* CINTERRUPTSOURCE_H_INCLUDED_C54F2C77 */
