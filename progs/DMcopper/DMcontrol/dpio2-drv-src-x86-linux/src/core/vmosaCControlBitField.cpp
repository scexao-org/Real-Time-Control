/****************************************************************************
Module      : vmosaCControlBitField.cpp

Description : Source file defining the CControlBitField class.
              A CControlBitField object represents two or more consequtive
              bits in a control register, with methods for reading and writing.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlBitField.cpp for Windows

*****************************************************************************/

#include "vmosaCControlBitField.h"



CControlBitField::CControlBitField(HANDLE hDpio2, 
                                   UINT32 offset,
                                   int mostSignificantBitNumber, 
                                   int leastSignificantBitNumber)

{

  int   cBit;

  UINT32   bitMask;


  if ( (mostSignificantBitNumber < 0) || (mostSignificantBitNumber > 31) ) {

    DPIO2_ERR_MSG (("bit field out of range!\n"));
    throw;

  } else if ( (leastSignificantBitNumber < 0) || (leastSignificantBitNumber > 31) ) {

    DPIO2_ERR_MSG (("bit field out of range!\n"));
    throw;

  } else if ( mostSignificantBitNumber < leastSignificantBitNumber ) {

    DPIO2_ERR_MSG (("bit field out of range!\n"));
    throw;

  } else {

    m_hDpio2 = hDpio2;

    m_dwOffset = offset;

    m_leastSignificantBitNumber = leastSignificantBitNumber;

    bitMask = 0x00000001;
    for (cBit = 0; cBit < (mostSignificantBitNumber - leastSignificantBitNumber); cBit++) {

      bitMask = (bitMask << 1) | 0x00000001;

    }
    bitMask <<= leastSignificantBitNumber;
    
    m_dwBitFieldMask = bitMask;

  }

}


CControlBitField::CControlBitField(const CControlBitField& right)
{

  m_hDpio2 = right.m_hDpio2;
  m_dwOffset = right.m_dwOffset;
  m_dwBitFieldMask = right.m_dwBitFieldMask;
  m_dwBitFieldValue = right.m_dwBitFieldValue;
  m_leastSignificantBitNumber = right.m_leastSignificantBitNumber;

}

