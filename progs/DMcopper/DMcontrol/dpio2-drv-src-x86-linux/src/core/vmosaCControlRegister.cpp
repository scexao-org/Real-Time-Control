/****************************************************************************
Module      : vmosaCControlRegister.cpp

Description : Source file defining the CControlRegister class.
              A CControlRegister object represents one 32 bits control 
              register, with methods for reading and writing.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlRegister.cpp for Windows

*****************************************************************************/

#include "vmosaCControlRegister.h"

CControlRegister::CControlRegister(HANDLE  hDpio2, 
				   UINT32  offset)
{

  m_hDpio2 = hDpio2;

  m_dwOffset = offset;

}


