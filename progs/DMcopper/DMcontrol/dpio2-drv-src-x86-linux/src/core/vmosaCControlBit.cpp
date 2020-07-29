/****************************************************************************
Module      : vmosaCControlBit.cpp

Description : Source file defining the CControlBit class.
              A CControlBit object represents one bit in a control register,
              with methods for setting, clearing, and reading.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
 01a,28jun2004,meh  Created based on CWControlBit.cpp for Windows

*****************************************************************************/

#include "vmosaCControlBit.h"



CControlBit::CControlBit(HANDLE hDpio2,
			 UINT32 offset, 
			 int bitNumber)
{

  if ( (bitNumber < 0) || (bitNumber > 31) ) {

    DPIO2_ERR_MSG (("bit number out of range!\n"));
    throw ;

  }

  m_hDpio2 = hDpio2;
  m_dwOffset = offset;
  m_dwBitMask = (UINT32) (0x00000001 << bitNumber);
  m_bitNumber = bitNumber;

}


CControlBit::CControlBit(const CControlBit& right)
{

  m_hDpio2 = right.m_hDpio2;
  m_dwOffset = right.m_dwOffset;
  m_dwBitMask = right.m_dwBitMask;
  m_bitNumber = right.m_bitNumber;

}

