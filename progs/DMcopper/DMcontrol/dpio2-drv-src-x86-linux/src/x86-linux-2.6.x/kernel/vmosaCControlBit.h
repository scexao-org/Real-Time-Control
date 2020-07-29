/****************************************************************************
Copyright Notice:
  This computer software is proprietary to VMETRO. The use of this software
  is governed by a licensing agreement. VMETRO retains all rights under
  the copyright laws of the United States of America and other countries.
  This software may not be furnished or disclosed to any third party and
  may not be copied or reproduced by any means, electronic, mechanical, or
  otherwise, in whole or in part, without specific authorization in writing
  from VMETRO.
 
    Copyright (c) 1996-2005 by VMETRO, ASA.  All Rights Reserved.


Module      : vmosaCControlBit.h

Description : Header file declaring the CControlBit class.
              A CControlBit object represents one bit in a control register,
              with methods for setting, clearing, and reading.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
 01a,28jun2004,meh  Created based on CWControlBit.h for Windows

*****************************************************************************/

#ifndef CCONTROLBIT_H_INCLUDED
#define CCONTROLBIT_H_INCLUDED

#include "vmosa-dpio2.h"



class CControlBit
{
  public:
    CControlBit(HANDLE hDpio2,
		UINT32 offset, 
		int bitNumber);

    CControlBit(const CControlBit& right);

    inline void set();

    inline void clear();

    inline void toggle();

    inline BOOL checkIsSet();


  private:

    HANDLE  m_hDpio2;

    UINT32  m_dwOffset;

    UINT32  m_dwBitMask;

    int     m_bitNumber;

};


void CControlBit::set()
{

  int status;


  status = vmosa_reg_32_read_modify_write_atomic (m_hDpio2,
						  m_dwOffset,
						  m_dwBitMask,
						  0xFFFFFFFF);

  if (status != 0) {

    throw;

  }

}


void CControlBit::clear() 
{

  int status;


  status = vmosa_reg_32_read_modify_write_atomic (m_hDpio2,
						  m_dwOffset,
						  m_dwBitMask,
						  0x00000000);
  

  if (status != 0) {

    throw;

  }

}



void CControlBit::toggle()
{

  int status;


  status = vmosa_reg_32_bit_toggle_atomic (m_hDpio2, m_dwOffset, m_bitNumber);

  if (status != 0) {
    
    throw;
    
  } 

}


BOOL CControlBit::checkIsSet()
{

  int      status;
  UINT32   value;

  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset, &value);

  if (status != 0) {
    throw;
  }
   
  if (value & m_dwBitMask)
    return 1;


  return 0;

}

#endif /* CCONTROLBIT_H_INCLUDED */
