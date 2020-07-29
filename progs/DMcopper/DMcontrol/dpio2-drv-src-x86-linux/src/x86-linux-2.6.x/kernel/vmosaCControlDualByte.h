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


Module      : vmosaCControlDualByte.h

Description : Header file declaring the CControlDualByte class.
              A CControlDualByte object represents the least significant 
              bytes of two consequtive 32 bits control registers.  
              These two bytes are read and written as one 16 bit value.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlDualByte.h for Windows

*****************************************************************************/

#ifndef CCONTROLDUALBYTE_H_INCLUDED
#define CCONTROLDUALBYTE_H_INCLUDED


#include "vmosa-dpio2.h"



class CControlDualByte
{

  public:
    CControlDualByte(HANDLE hDpio2, UINT32 offsetByte0);
    CControlDualByte(const CControlDualByte& right);

    inline UINT16 getValue();

    inline void setValue(UINT16 value);

  private:

    HANDLE m_hDpio2;
    
    UINT32  m_dwOffset;

};



UINT16 CControlDualByte::getValue()
{

  int      status;
  UINT32   byte0;
  UINT32   byte1;
  

  status = vmosa_reg_32_read (m_hDpio2,m_dwOffset, &byte0);

  if (status != 0)

    throw;


  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset + 4, &byte1);

  if (status != 0)

    throw;


  return ((UINT16) ( (byte0 & 0x000000FF) |
		     ((byte1 & 0x000000FF) << 8) ));

}


void CControlDualByte::setValue(UINT16 value)
{

  int status;


  status = vmosa_reg_32_write (m_hDpio2, m_dwOffset, (UINT32) (value & 0x00FF));

  if (status != 0)

    throw;


  status = vmosa_reg_32_write (m_hDpio2, m_dwOffset + 4,
			       (UINT32) ((value >> 8) & 0x00FF));

  if (status != 0)

    throw;

  
}

#endif /* CCONTROLDUALBYTE_H_INCLUDED */
