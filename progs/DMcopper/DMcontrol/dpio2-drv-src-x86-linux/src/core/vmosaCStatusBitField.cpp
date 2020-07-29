/****************************************************************************
Module      : vmosaCStatusBitField.cpp

Description : Source file defining the CStatusBitField class.
              A CStatusBitField object represents two or more consecutive
              bits in a 32 bits status register, with method reading their
              values.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWStatusBitField.cpp for Windows

*****************************************************************************/

#include "vmosaCStatusBitField.h"


CStatusBitField::CStatusBitField(HANDLE hDpio2, 
                                 UINT32 offset, 
                                 int numMostSignificantBit, 
                                 int numLeastSignificantBit)
{

  int   cBit;

  UINT32   bitMask;


  if ( (numMostSignificantBit < 0) || (numMostSignificantBit > 31) ) {

    throw;

  } else if ( (numLeastSignificantBit < 0) || (numLeastSignificantBit > 31) ) {

    throw;

  } else if ( numMostSignificantBit < numLeastSignificantBit ) {

    throw;

  } else {

    m_hDpio2 = hDpio2;

    m_dwOffset = offset;

    m_leastSignificantBitNumber = numLeastSignificantBit;

    bitMask = 0x00000001;
    for (cBit = 0; cBit < (numMostSignificantBit - numLeastSignificantBit); cBit++) {

      bitMask = (bitMask << 1) | 0x00000001;

    }
    bitMask <<= numLeastSignificantBit;
    
    m_dwBitFieldMask = bitMask;

  }

}


CStatusBitField::CStatusBitField(const CStatusBitField& right)
{

  m_hDpio2 = right.m_hDpio2;
  m_dwOffset = right.m_dwOffset;
  m_dwBitFieldMask = right.m_dwBitFieldMask;
  m_dwBitFieldValue = right.m_dwBitFieldValue;
  m_leastSignificantBitNumber = right.m_leastSignificantBitNumber;

}
