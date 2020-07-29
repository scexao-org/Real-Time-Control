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


Module      : vmosaCControlRegister.h

Description : Header file declaring the CControlRegister class.
              A CControlRegister object represents one 32 bits control 
              register, with methods for reading and writing.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Created based on CWControlRegister.h for Windows

*****************************************************************************/

#ifndef CCONTROLREGISTER_H_INCLUDED
#define CCONTROLREGISTER_H_INCLUDED


#include "vmosa-dpio2.h"



class CControlRegister
{
public:

  CControlRegister(HANDLE hDpio2, UINT32 offset);
  
  inline UINT32 getValue();

  inline void setValue(UINT32 value);

private:
  
  HANDLE  m_hDpio2;

  UINT32  m_dwOffset;

  UINT32  m_dwData;


};



UINT32 CControlRegister::getValue() 
{

  int status;

  
  status = vmosa_reg_32_read (m_hDpio2, m_dwOffset, &m_dwData);

  if (status != 0)

    throw;


  return (m_dwData);

}



void CControlRegister::setValue(UINT32 value) 
{

  int status;


  status = vmosa_reg_32_write (m_hDpio2, m_dwOffset, value);
  
  if (status != 0)

    throw;

}


#endif /* CCONTROLREGISTER_H_INCLUDED */
