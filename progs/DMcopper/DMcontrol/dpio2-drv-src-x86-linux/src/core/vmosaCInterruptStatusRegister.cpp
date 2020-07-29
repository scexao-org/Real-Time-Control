/****************************************************************************
Module      : vmosaCInterruptStatusRegister.cpp

Description : Source file defining the CInterruptStatusRegister class.
              A CInterruptStatusRegister object represents one 32 bits
              status register containing the status bits of one or more 
              interrupt sources.

              It is used by CInterruptManager objects to prevent
              interrupt status register from being read more than necessary.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,29jun2004,meh  Ported from CInterruptStatusRegister.cpp
 
*****************************************************************************/

#include "DpioCommon.h"

#include "vmosaCInterruptStatusRegister.h"




CInterruptStatusRegister::CInterruptStatusRegister(HANDLE hDpio2, UINT32 offset)
{

  int   iBit;


  m_hDpio2 = hDpio2;
  m_offset = offset;
  m_bitMask = 0;


  /* Initialize the array of source IDs.
   */
  for (iBit = 0; iBit < MAX_NUMBER_OF_STATUS_BITS; iBit++) {

    m_aSourceId[iBit] = (-1);
    
  }

}



int CInterruptStatusRegister::getPendingInterruptSource()
{

  UINT32   regValue;
  UINT32   bitMask;
  int      sourceId;
  int      iBit;


  /* Read the status register and mask out the bits 
   * which are not tied to a interrupt source.
   */

  vmosa_reg_32_read (m_hDpio2, m_offset, &regValue);

  regValue &= m_bitMask;


  /* Check whether any of the associated interrupt sources are pending.
   */
  if ( regValue == 0 ) {

    sourceId = (-1);

  } else {

    /* Find ID of the interrupt source tied
     * to the least significant status bit which is set.
     */
    iBit = 0;
    bitMask = 0x00000001;
    while ( (regValue & bitMask) == 0 ) {
      bitMask <<= 1;
      iBit++;
    }

    sourceId = m_aSourceId[iBit];

  }


  return (sourceId);

}


STATUS CInterruptStatusRegister::attachSourceToStatusBit(int sourceId, int bitNumber)
{

  STATUS   returnValue;


  /* Check that the parameter values are valid.
   */
  if ( (bitNumber < 0) || (bitNumber >= MAX_NUMBER_OF_STATUS_BITS) || (sourceId == (-1)) ) {

    returnValue = ERROR;

  } 

  /* Check that the specified status bit isn't tied to an interrupt source already.
   */
  else if ( m_aSourceId[bitNumber] != (-1) ) {

    returnValue = ERROR;

  } else {

    m_aSourceId[bitNumber] = sourceId;
    m_bitMask |= (0x00000001 << bitNumber);

    returnValue = OK;

  }


  return (returnValue);

}

