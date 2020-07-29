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


Module      : vmosaCDpio2DmaController.h

Description : Header file declaring the CDpio2DmaController class.
              A CDpio2DmaController object represents the functionality
              of the DMA Controller on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,29jun2004,meh  Ported from CDpio2DmaController.h

*****************************************************************************/

#ifndef CDPIO2DMACONTROLLER_H_INCLUDED
#define CDPIO2DMACONTROLLER_H_INCLUDED

#include "vmosa.h"

#include "vmosaCControlBit.h"
#include "vmosaCControlBitField.h"
#include "vmosaCControlDualByte.h"
#include "vmosaCControlRegister.h"
#include "vmosaCStatusBit.h"
#include "vmosaCStatusBitField.h"

#include "CDpio2DmaDescriptor.h"

#include "DpioCommon.h"
#include "Dpio2Defs.h"


class CDpio2DmaListIterator;


class CDpio2DmaController
{
public:

  CDpio2DmaController(HANDLE hDpio2,
		      UINT32 startOffsetDmaList0,
                      UINT32 numDescriptorsDmaList0,
                      
                      const CControlBit& bitEnableEotFunctionality,
                      const CControlBit& bitEndOfDmaChain,
                      const CControlBit& bitDmaDirection,
                      const CControlBit& bitEnableInterrupt,
                      const CControlBit& bitEnableDmaDemandMode,
                      const CControlBit& bitStartStop,
                      const CControlBit& bitPauseContinue,
                      const CControlBit& bitEnableClearAllOnStop,
                      const CControlBit& bitEnableDescriptorFromLocalBus,
                      const CControlBit& bitEnableNextDescriptorOnEot,
                      const CControlBit& bitEotOnSyncCountEnable,
                      
                      const CControlBitField& bitFieldNextDescriptorAddress,
                      const CControlBitField& bitFieldTransferSize,
                      const CControlBitField& bitFieldWriteCommand,
                      const CControlBitField& bitFieldReadCommand,
                      const CControlBitField& bitFieldEndOfTransferMode,
                      
                      const CControlDualByte& eotSyncCounter,

                      const CControlRegister& registerPciAddress,
                      
                      const CStatusBit& bitDmaChainDone,
                      const CStatusBit& bitByteCountOverflowFlag,

                      const CStatusBitField& bitFieldRemainingByteCount,
                      const CStatusBitField& bitFieldTransferredByteCount);


  STATUS setDescriptor(UINT32 descriptorId, 
		       const CDpio2InputDmaDescriptor* pDescriptor);
  STATUS setDescriptor(UINT32 descriptorId, 
		       const CDpio2OutputDmaDescriptor* pDescriptor);


  inline STATUS setNextDescriptorInDmaChain(UINT32 descriptorId);
  inline UINT32 getNextDescriptorInDmaChain();

  inline void startDmaTransfer();
  inline void stopDmaTransfer();
  inline BOOL checkIsChainCompleted();

  inline void suspendDmaTransfer();
  inline void resumeDmaTransfer();
  inline BOOL checkIsDmaTransferSuspended();


  inline void enableClearingOnStop();
  inline void disableClearingOnStop();


  //## Chooses which signal shall mark End Of Transfer and enable this
  //## mechanism.
  inline void selectEndOfTransferMode(DPIO2_END_OF_TRANSFER_MODE mode);
  inline void enableEndOfTransfer();
  inline void disableEndOfTransfer();

  inline void enableEotSyncCount(UINT16 countValue);
  inline void disableEotSyncCount();

  inline void enableContinueOnEndOfTransfer();
  inline void disableContinueOnEndOfTransfer();

  inline void enableDmaDemandMode();
  inline void disableDmaDemandMode();
  inline BOOL checkIsDmaDemandModeEnabled();

  inline void setPciWriteCommand(UINT32 command);
  inline void setPciReadCommand(UINT32 command);

  inline void setDirectionAsOut();
  inline void setDirectionAsIn();


  inline void getRemainingByteCount(UINT32& byteCount, 
                                    BOOL& byteCountHasOverflowed);
  inline void getTransferredByteCount(UINT32& byteCount);


protected:
  CControlBit m_bitEnableEotFunctionality;
  CControlBit m_bitEndOfDmaChain;
  CControlBit m_bitDmaDirection;
  CControlBit m_bitEnableInterrupt;
  CControlBit m_bitEnableDmaDemandMode;
  CControlBit m_bitStartStop;
  CControlBit m_bitPauseContinue;
  CControlBit m_bitEnableClearAllOnStop;
  CControlBit m_bitEnableDescriptorFromLocalBus;
  CControlBit m_bitEnableNextDescriptorOnEot;
  CControlBit m_bitEotOnSyncCountEnable;

  CControlBitField m_bitFieldNextDescriptorAddress;
  CControlBitField m_bitFieldTransferSize;
  CControlBitField m_bitFieldWriteCommand;
  CControlBitField m_bitFieldReadCommand;
  CControlBitField m_bitFieldEndOfTransferMode;

  CControlDualByte m_eotSyncCounter;

  CControlRegister m_registerPciAddress;

  CStatusBit m_bitDmaChainDone;
  CStatusBit m_bitByteCountOverflowFlag;

  CStatusBitField m_bitFieldRemainingByteCount;
  CStatusBitField m_bitFieldTransferredByteCount;


private:

  HANDLE m_hDpio2;

  BOOL m_moduleIsOutput;

  UINT32 m_listStartOffset;

  UINT32 m_numDescriptors;


  BOOL checkIsDescriptorIdValid(UINT32 id);

  BOOL checkIsDescriptorValid(const CDpio2InputDmaDescriptor* pDescriptor);
  BOOL checkIsDescriptorValid(const CDpio2OutputDmaDescriptor* pDescriptor);
    
};



STATUS CDpio2DmaController::setNextDescriptorInDmaChain(UINT32 descriptorId)
{

  STATUS   status;


  status = OK;

  /* Check that the desciptor ID is valid.
   */
  if ( descriptorId >= m_numDescriptors ) {

    ERROR_MSG(("Invalid descriptor ID."));
    status = ERROR;

  }


  if ( status == OK ) {

    m_bitEnableDescriptorFromLocalBus.set();

    m_bitFieldNextDescriptorAddress.setValue(descriptorId * sizeof(CDpio2PhysicalDescriptor));

  }


  return (status);

}


UINT32 CDpio2DmaController::getNextDescriptorInDmaChain()
{

  UINT32   bitFieldValue;


  bitFieldValue = m_bitFieldNextDescriptorAddress.getValue();

  return ((int) (bitFieldValue / sizeof(CDpio2PhysicalDescriptor)));

}


void CDpio2DmaController::startDmaTransfer()
{
  
  /* Make sure the control bit is cleared before it is set.
   * Otherwise setting it will have no effect.
   */
  /**/
  m_bitStartStop.clear();
  m_bitStartStop.set();

}


void CDpio2DmaController::stopDmaTransfer()
{

  m_bitStartStop.clear();

}


BOOL CDpio2DmaController::checkIsChainCompleted()
{

  return (m_bitDmaChainDone.checkIsSet());

}


void CDpio2DmaController::enableClearingOnStop()
{

  m_bitEnableClearAllOnStop.set();

}


void CDpio2DmaController::disableClearingOnStop()
{

  m_bitEnableClearAllOnStop.clear();

}


void CDpio2DmaController::suspendDmaTransfer()
{

  m_bitPauseContinue.set();

}


void CDpio2DmaController::resumeDmaTransfer()
{

  m_bitPauseContinue.clear();

}


BOOL CDpio2DmaController::checkIsDmaTransferSuspended()
{

  return (m_bitPauseContinue.checkIsSet());

}


void CDpio2DmaController::selectEndOfTransferMode(DPIO2_END_OF_TRANSFER_MODE mode)
{

  
  UINT32   currentMode;


  /* When a EOT mode is selected,
   * any previously selected SYNC reception mode will be disabled.
   * Warn about this.
   */
  currentMode = m_bitFieldEndOfTransferMode.getValue();

  switch (currentMode) {

  case DPIO2_SYNC_STARTS_DATA_RECEPTION:
    printf ("Selecting EOT mode %d disables SYNC mode %d\n", mode, (int) currentMode);
    break;
    
  }


  /* Configure the EOT mode.
   */
  m_bitFieldEndOfTransferMode.setValue(mode);

}


void CDpio2DmaController::enableEndOfTransfer()
{
  
  m_bitEnableEotFunctionality.set();

}


void CDpio2DmaController::disableEndOfTransfer()
{

  m_bitEnableEotFunctionality.clear();

}


void CDpio2DmaController::enableEotSyncCount(UINT16 countValue)
{

  m_eotSyncCounter.setValue(countValue - 1);
  m_bitEotOnSyncCountEnable.set();

}


void CDpio2DmaController::disableEotSyncCount()
{

  m_bitEotOnSyncCountEnable.clear();
  m_eotSyncCounter.setValue(0);

}


void CDpio2DmaController::enableContinueOnEndOfTransfer()
{

  m_bitEnableNextDescriptorOnEot.set();

}


void CDpio2DmaController::disableContinueOnEndOfTransfer()
{

  m_bitEnableNextDescriptorOnEot.clear();

}


void CDpio2DmaController::enableDmaDemandMode()
{

  m_bitEnableDmaDemandMode.set();

}


void CDpio2DmaController::disableDmaDemandMode()
{

  m_bitEnableDmaDemandMode.clear();

}


BOOL CDpio2DmaController::checkIsDmaDemandModeEnabled()
{

  return (m_bitEnableDmaDemandMode.checkIsSet());

}


void CDpio2DmaController::setPciWriteCommand(UINT32 command)
{

  m_bitFieldWriteCommand.setValue(command);

}


void CDpio2DmaController::setPciReadCommand(UINT32 command)
{

  m_bitFieldReadCommand.setValue(command);

}


void CDpio2DmaController::setDirectionAsOut()
{

  m_moduleIsOutput = TRUE;

}


void CDpio2DmaController::setDirectionAsIn()
{

  m_moduleIsOutput = FALSE;

}


void CDpio2DmaController::getRemainingByteCount(UINT32& byteCount, 
                                                BOOL& byteCountHasOverflowed)
{

  /* Read the overflow flag bit first to avoid it being cleared
   * when the byte count is read.
   */
  byteCountHasOverflowed = m_bitByteCountOverflowFlag.checkIsSet();
  byteCount = m_bitFieldRemainingByteCount.getValue();
  
}


void CDpio2DmaController::getTransferredByteCount(UINT32& byteCount)
{

  byteCount = m_bitFieldTransferredByteCount.getValue();
  
}
#endif /* CDPIO2DMACONTROLLER_H_INCLUDED */
