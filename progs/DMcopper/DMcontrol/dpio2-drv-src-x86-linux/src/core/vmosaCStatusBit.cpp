/****************************************************************************
Module      : vmosaCStatusBit.cpp

Description : Source file defining the CStatusBit class.
              A CStatusBit object represents one status bit in a 32 bits
              status register, and offers a method for checking whether
              the status bit is set or not.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWStatusBit.cpp for Windows

*****************************************************************************/

#include "vmosaCStatusBit.h"


CStatusBit::CStatusBit(HANDLE hDpio2,
		       UINT32 offset, 
		       int bitNumber)
{

  if ( (bitNumber < 0) || (bitNumber > 31) ) {

    throw ;

  }

  m_hDpio2 = hDpio2;

  m_dwOffset = offset;
  m_dwBitMask = (1 << bitNumber);

}


CStatusBit::CStatusBit(const CStatusBit& right)
{

  m_hDpio2 = right.m_hDpio2;
  m_dwOffset = right.m_dwOffset;
  m_dwBitMask = right.m_dwBitMask;

}

