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


Module      : vmosaCStatusBit.h

Description : Header file declaring the CStatusBit class.
              A CStatusBit object represents one status bit in a 32 bits
              status register, and offers a method for checking whether
              the status bit is set or not.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWStatusBit.h for Windows

*****************************************************************************/

#ifndef CSTATUSBIT_H_INCLUDED
#define CSTATUSBIT_H_INCLUDED

#include "vmosa-dpio2.h"



class CStatusBit
{
  public:
    CStatusBit(HANDLE hDpio2, UINT32 offset, int bitNumber);

    CStatusBit(const CStatusBit& right);

    inline BOOL checkIsSet();


  private:

    HANDLE  m_hDpio2;

    UINT32  m_dwOffset;

    UINT32  m_dwBitMask;

};


BOOL CStatusBit::checkIsSet()
{

  int      status;
  UINT32   value;


  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset, &value);

  if (status != 0)

    throw;


  if (value & m_dwBitMask)

    return 1;


  return 0;

}

#endif /* CSTATUSBIT_H_INCLUDED */
