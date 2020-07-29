/****************************************************************************
Module      : CInterruptSource.cpp

Description : Source file defining the CInterruptSource class.
              A CInterruptSource object represents an interrupt source,
              and offers methods for enabling the source, disabling the 
              source, and checking whether the source has generated an
              interrupt.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,22jun2001,nib  Created.

*****************************************************************************/

#include "CInterruptSource.h"

//##ModelId=3AB0B5C800A0
CInterruptSource::CInterruptSource(const CControlBit& bitEnable,
                                   const CStatusBit& bitActiveFlag)
  : m_bitEnable(bitEnable),
    m_bitActiveFlag(bitActiveFlag)
{

  ;

}



