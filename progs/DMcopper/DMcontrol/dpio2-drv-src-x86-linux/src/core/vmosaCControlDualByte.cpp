/****************************************************************************
Module      : vmosaCControlDualByte.cpp

Description : Source file defining the CControlDualByte class.
              A CControlDualByte object represents the least significant 
              bytes of two consequtive 32 bits control registers.  
              These two bytes are read and written as one 16 bit value.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlDualByte.cpp for Windows

*****************************************************************************/

#include "vmosaCControlDualByte.h"


CControlDualByte::CControlDualByte(HANDLE   hDpio2, 
				   UINT32   offsetByte0)
{

  m_hDpio2 = hDpio2;

  m_dwOffset = offsetByte0;

}


CControlDualByte::CControlDualByte(const CControlDualByte& right)
{

  m_hDpio2 = right.m_hDpio2;

  m_dwOffset = right.m_dwOffset;

}


