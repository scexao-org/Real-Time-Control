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


Module      : vmosaCControlBitField.h

Description : Header file declaring the CControlBitField class.
              An CControlBitField object represents two or more consequtive
              bits in a control register, with methods for reading and writing.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlBitField.h for Windows

*****************************************************************************/

#ifndef CCONTROLBITFIELD_H_INCLUDED
#define CCONTROLBITFIELD_H_INCLUDED


#include "vmosa-dpio2.h"



class CControlBitField
{
  public:
    CControlBitField ( HANDLE hDpio2, 
		       UINT32 offset,
		       int mostSignificantBitNumber, 
		       int leastSignificantBitNumber );

    CControlBitField(const CControlBitField& right);

    inline void setValue ( UINT32 value );

    inline UINT32 getValue ();


  private:

    HANDLE  m_hDpio2;

    UINT32  m_dwOffset;

    UINT32  m_dwBitFieldMask;

    UINT32  m_dwBitFieldValue;
      
    int   m_leastSignificantBitNumber;

};


void CControlBitField::setValue ( UINT32 value )
{

  int status;
  
  
  m_dwBitFieldValue = ( value << m_leastSignificantBitNumber );
  
  status = vmosa_reg_32_read_modify_write_atomic ( m_hDpio2,
						   m_dwOffset,
						   m_dwBitFieldMask,
						   m_dwBitFieldValue );

  if ( status != 0 )

    throw;

}



UINT32 CControlBitField::getValue() 
{

  int      status;


  status = vmosa_reg_32_read ( m_hDpio2, m_dwOffset, &m_dwBitFieldValue );

  if ( status != 0 )
    throw;

  return ((m_dwBitFieldValue & m_dwBitFieldMask) >> m_leastSignificantBitNumber);

}

#endif /* CCONTROLBITFIELD_H_INCLUDED */
