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


Module      : vmosaCControlQuadByte.h

Description : Header file declaring the CControlQuadByte class.
              A CControlQuadByte object represents the least significant 
              bytes of four consequtive 32 bits control registers.  
              These four bytes are read and written as one 32 bit value.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlQuadByte.H for Windows

*****************************************************************************/

#ifndef CCONTROLQUADBYTE_H_INCLUDED
#define CCONTROLQUADBYTE_H_INCLUDED


#include "vmosa-dpio2.h"



class CControlQuadByte
{

  public:
    CControlQuadByte(HANDLE hDpio2, UINT32 offsetByte0);
    CControlQuadByte(const CControlQuadByte& right);

    inline UINT32 getValue();

    inline void setValue(UINT32 value);

  private:

    HANDLE   m_hDpio2;

    UINT32   m_dwOffset;

};


UINT32 CControlQuadByte::getValue()
{
  int      status;
  UINT32   byte0;
  UINT32   byte1;
  UINT32   byte2;
  UINT32   byte3;
  

  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset, &byte0);

  if (status != 0)

    throw;


  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset + 4, &byte1);

  if (status != 0)

    throw;


  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset + 8, &byte2);

  if (status != 0)

    throw;


  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset + 12, &byte3);

  if (status != 0)

    throw;

				      
  return ( ( (byte0 & 0x000000FF) |
	     ((byte1 & 0x000000FF) << 8) |
	     ((byte2 & 0x000000FF) << 16) |
	     ((byte3 & 0x000000FF) << 24) ) );
 
}



void CControlQuadByte::setValue(UINT32 value)
{
  int status;


  status = vmosa_reg_32_write (m_hDpio2, m_dwOffset,
			      (UINT32) (value & 0x000000FF));

  if (status != 0)

    throw;


  status = vmosa_reg_32_write (m_hDpio2, m_dwOffset + 4,
			       (UINT32) ((value >> 8) & 0x000000FF));

  if (status != 0)

    throw;


  status = vmosa_reg_32_write (m_hDpio2, m_dwOffset + 8,
			       (UINT32) ((value >> 16) & 0x000000FF));

  if (status != 0)

    throw;


  status = vmosa_reg_32_write (m_hDpio2, m_dwOffset + 12,
			       (UINT32) ((value >> 24) & 0x000000FF));

  if (status != 0)

    throw;


}

#endif /* CCONTROLQUADBYTE_H_INCLUDED */
