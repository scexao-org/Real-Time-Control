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


Module      : vmosaCStatusBitField.h

Description : Header file declaring the CStatusBitField class.
              A CStatusBitField object represents two or more consecutive
              bits in a 32 bits status register, with method reading their
              values.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWStatusBitField.h for Windows

*****************************************************************************/

#ifndef CSTATUSBITFIELD_H_INCLUDED
#define CSTATUSBITFIELD_H_INCLUDED

#include "vmosa-dpio2.h"



class CStatusBitField
{
  public:
    CStatusBitField(HANDLE hDpio2, 
                    UINT32 offset, 
                    int numMostSignificantBit, 
                    int numLeastSignificantBit);

    CStatusBitField(const CStatusBitField& right);

    inline UINT32 getValue();

  private:

    HANDLE  m_hDpio2;

    UINT32  m_dwOffset;

    UINT32  m_dwBitFieldMask;

    UINT32  m_dwBitFieldValue;

    int     m_leastSignificantBitNumber;

};


UINT32 CStatusBitField::getValue()
{

   int      status;

   
   status = vmosa_reg_32_read (m_hDpio2, m_dwOffset, &m_dwBitFieldValue);
   
   if (status != 0)
     
     throw;
   

   return ((m_dwBitFieldValue & m_dwBitFieldMask) >> m_leastSignificantBitNumber);
   
}



#endif /* CSTATUSBITFIELD_H_INCLUDED */
