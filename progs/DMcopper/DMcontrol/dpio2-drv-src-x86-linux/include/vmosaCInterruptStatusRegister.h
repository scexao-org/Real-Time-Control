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


Module      : vmosaCInterruptStatusRegister.h

Description : Header file declaring the CInterruptStatusRegister class.
              A CInterruptStatusRegister object represents one 32 bits
              status register containing the status bits of one or more 
              interrupt sources.

              It is used by CInterruptManager objects to prevent
              interrupt status register from being read more than necessary.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,29jun2004,meh  Ported from CInterruptStatusRegister.h

*****************************************************************************/

#ifndef CINTERRUPTSTATUSREGISTER_H_INCLUDED
#define CINTERRUPTSTATUSREGISTER_H_INCLUDED


#include "vmosa-dpio2.h"


const int   MAX_NUMBER_OF_STATUS_BITS = 32;



//## This class represent an interrupt status register with one or more status
//## bits which signal when a corresponding interrupt source is pending.

class CInterruptStatusRegister
{
  public:

    CInterruptStatusRegister(HANDLE hDpio2, UINT32 offset);

    inline UINT32 getOffset();

    inline UINT32 getBitMask();


    //## This method returns the ID of the interrupt source associated with the
    //## most significant status bit in the register that are set.  If none of
    //## the status bits are set, this method returns (-1).
    int getPendingInterruptSource();

    STATUS attachSourceToStatusBit(int sourceId, int bitNumber);

  private:

    HANDLE m_hDpio2;

    UINT32 m_offset;

    UINT32 m_bitMask;

    int m_aSourceId[32];

};


UINT32 CInterruptStatusRegister::getOffset()
{

  return (m_offset);

}


UINT32 CInterruptStatusRegister::getBitMask() 
{

  return (m_bitMask);

}




#endif /* CINTERRUPTSTATUSREGISTER_H_INCLUDED */
