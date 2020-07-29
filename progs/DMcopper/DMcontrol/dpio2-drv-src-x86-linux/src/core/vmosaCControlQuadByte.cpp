/****************************************************************************
Module      : vmosaCControlQuadByte.cpp

Description : Source file defining the CControlQuadByte class.
              A CControlQuadByte object represents the least significant 
              bytes of four consequtive 32 bits control registers.  
              These four bytes are read and written as one 32 bit value.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlQuadByte.cpp for Windows

*****************************************************************************/

#include "vmosaCControlQuadByte.h"


CControlQuadByte::CControlQuadByte(HANDLE hDpio2, UINT32 offsetByte0)
{

  m_hDpio2 = hDpio2;

  m_dwOffset = offsetByte0;

}


CControlQuadByte::CControlQuadByte(const CControlQuadByte& right)
{

  m_hDpio2 = right.m_hDpio2;

  m_dwOffset = right.m_dwOffset;

}


