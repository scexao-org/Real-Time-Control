/****************************************************************************
Module      : vmosaCDpio2DmaController.cpp

Description : Source file defining the CDpio2DmaController class.
              A CDpio2DmaController object represents the functionality
              of the DMA Controller on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,29jun2004,meh  Ported from CDpio2DmaController.cpp

*****************************************************************************/

#include "vmosaCDpio2DmaController.h"
#include "Dpio2Defs.h"



CDpio2DmaController::CDpio2DmaController(HANDLE hDpio2,
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
                                         const CStatusBitField& bitFieldTransferredByteCount)
  : m_bitEnableEotFunctionality(bitEnableEotFunctionality),
    m_bitEndOfDmaChain(bitEndOfDmaChain),
    m_bitDmaDirection(bitDmaDirection),
    m_bitEnableInterrupt(bitEnableInterrupt),
    m_bitEnableDmaDemandMode(bitEnableDmaDemandMode),
    m_bitStartStop(bitStartStop),
    m_bitPauseContinue(bitPauseContinue),
    m_bitEnableClearAllOnStop(bitEnableClearAllOnStop),
    m_bitEnableDescriptorFromLocalBus(bitEnableDescriptorFromLocalBus),
    m_bitEnableNextDescriptorOnEot(bitEnableNextDescriptorOnEot),
    m_bitEotOnSyncCountEnable(bitEotOnSyncCountEnable),

    m_bitFieldNextDescriptorAddress(bitFieldNextDescriptorAddress),
    m_bitFieldTransferSize(bitFieldTransferSize),
    m_bitFieldWriteCommand(bitFieldWriteCommand),
    m_bitFieldReadCommand(bitFieldReadCommand),
    m_bitFieldEndOfTransferMode(bitFieldEndOfTransferMode),

    m_eotSyncCounter(eotSyncCounter),

    m_registerPciAddress(registerPciAddress),

    m_bitDmaChainDone(bitDmaChainDone),
    m_bitByteCountOverflowFlag(bitByteCountOverflowFlag),

    m_bitFieldRemainingByteCount(bitFieldRemainingByteCount),
    m_bitFieldTransferredByteCount(bitFieldTransferredByteCount),

    m_listStartOffset(startOffsetDmaList0),
    m_numDescriptors(numDescriptorsDmaList0)
{

  m_hDpio2 = hDpio2;

}


STATUS CDpio2DmaController::setDescriptor(UINT32 descriptorId,
					  const CDpio2InputDmaDescriptor* pDescriptor)
{

  UINT32   val32;

  UINT32   descriptorOffset;



  if ( !checkIsDescriptorIdValid(descriptorId) )

    return ERROR;

  
  if ( !checkIsDescriptorValid(pDescriptor) )

    return ERROR;



  descriptorOffset = m_listStartOffset + (descriptorId * sizeof (UINT32) * 4);

  val32 = ( (pDescriptor->m_flagEndOfDmaChain 
	     ? (1 << DPIO2_BIT_NUM_END_OF_CHAIN) : 0)
	    | (1 << DPIO2_BIT_NUM_ENABLE_DESCRIPTOR_FROM_LOCAL_BUS)
	    | (1 << DPIO2_BIT_NUM_DIRECTION)
	    | (pDescriptor->m_flagGenerateDmaBlockEndInterrupt
	       ? (1 << DPIO2_BIT_NUM_ENABLE_INTERRUPT) : 0)
	    | (sizeof(CDpio2PhysicalDescriptor) * pDescriptor->m_nextDescriptorId));
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset, val32);
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset + 4, 
		      pDescriptor->m_destinationAddress);
  
  val32 = (pDescriptor->m_flagUse64BitsDataAccess
	   ? (1 << DPIO2_BIT_NUM_ENABLE_D64_ACCESSES) : 0);
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset + 8, val32);
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset + 12, 
		      pDescriptor->m_transferLength);
  

  return OK;

}


STATUS CDpio2DmaController::setDescriptor(UINT32 descriptorId,
					  const CDpio2OutputDmaDescriptor* pDescriptor)
{

  UINT32   val32;

  UINT32   descriptorOffset;



  if ( !checkIsDescriptorIdValid(descriptorId) )

    return ERROR;

 
  if ( !checkIsDescriptorValid(pDescriptor) )

    return ERROR;



  descriptorOffset = m_listStartOffset + (descriptorId * sizeof (UINT32) * 4);

  printf ("\nDMA descriptor\n");

  val32 = ( (pDescriptor->m_flagEndOfDmaChain 
	     ? (1 << DPIO2_BIT_NUM_END_OF_CHAIN) : 0)
	    | (1 << DPIO2_BIT_NUM_ENABLE_DESCRIPTOR_FROM_LOCAL_BUS)
	    | (pDescriptor->m_flagGenerateDmaBlockEndInterrupt
	       ? (1 << DPIO2_BIT_NUM_ENABLE_INTERRUPT) : 0)
	    | (sizeof(CDpio2PhysicalDescriptor) * pDescriptor->m_nextDescriptorId));

  printf ("Next descriptor = %x\n", val32);
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset, val32);
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset + 4, 
		      pDescriptor->m_sourceAddress);

  printf ("pci Address = %x\n", pDescriptor->m_sourceAddress);
  
  val32 = ( (pDescriptor->m_flagUse64BitsDataAccess
	     ? (1 << DPIO2_BIT_NUM_ENABLE_D64_ACCESSES) : 0)
	    | (pDescriptor->m_flagNotLastBlockInFrame
	       ? (1 << DPIO2_BIT_NUM_BLOCK_IS_NOT_LAST_IN_FRAME) : 0));

  printf ("Option register = %x\n", val32);
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset + 8, val32);
  
  vmosa_reg_32_write (m_hDpio2, descriptorOffset + 12, 
		      pDescriptor->m_transferLength);

  printf ("Buffer size = %d\n\n", pDescriptor->m_transferLength);
  
  return OK;
  
}


BOOL CDpio2DmaController::checkIsDescriptorIdValid(UINT32 id)
{

  BOOL   result;


  if ( id >= m_numDescriptors ) {

    DPIO2_ERROR_MSG(("ID of next descriptor (%d) is not valid (must be below %d).\n",
               (int) id, (int) m_numDescriptors));

    result = FALSE;

  } else {

    result = TRUE;

  }


  return (result);

}


BOOL CDpio2DmaController::checkIsDescriptorValid(const CDpio2InputDmaDescriptor* pDescriptor)
{

  BOOL   result;

  UINT32   minResolution;


  result = TRUE;


  /* Determine how the destination address is required to be aligned
   * and check that it is aligned in that way.
   */
  minResolution = (pDescriptor->m_flagUse64BitsDataAccess ? 8 : 4);

  if ( (pDescriptor->m_destinationAddress & (minResolution - 1)) != 0 ) {

    DPIO2_ERROR_MSG(("The destination address %08X is not aligned to a %d byte boundary.\n",
               (int) pDescriptor->m_destinationAddress, (int) minResolution));

    result = FALSE;

  }


  if ( pDescriptor->m_transferLength > DPIO2_DMA_MAX_TRANSFER_LENGTH ) {

    DPIO2_ERROR_MSG(("DMA block size (%d bytes), is longer than allowed (%d bytes).\n",
               (int) pDescriptor->m_transferLength, DPIO2_DMA_MAX_TRANSFER_LENGTH));

    result = FALSE;

  }


  if ( pDescriptor->m_transferLength < DPIO2_DMA_MIN_TRANSFER_LENGTH ) {

    DPIO2_ERROR_MSG(("DMA block size (%d bytes), is shorter than allowed (%d bytes).\n",
               (int) pDescriptor->m_transferLength, DPIO2_DMA_MIN_TRANSFER_LENGTH));

    result = FALSE;

  }


  if ( (pDescriptor->m_transferLength & (minResolution - 1)) != 0 ) {

    DPIO2_ERROR_MSG(("DMA block size (%d bytes) must be a multiplum of %d bytes.\n",
               (int) pDescriptor->m_transferLength, (int) minResolution));

    result = FALSE;

  }


  if ( !checkIsDescriptorIdValid(pDescriptor->m_nextDescriptorId) ) {

    result = FALSE;

  }


  return (result);

}


BOOL CDpio2DmaController::checkIsDescriptorValid(const CDpio2OutputDmaDescriptor* pDescriptor)
{

  BOOL   result;

  UINT32   minResolution;


  result = TRUE;


  /* Determine how the source address is required to be aligned
   * and check that it is aligned that way.
   */
  minResolution = (pDescriptor->m_flagUse64BitsDataAccess ? 8 : 4);

  if ( (pDescriptor->m_sourceAddress & (minResolution - 1)) != 0 ) {

    DPIO2_ERROR_MSG(("The source address %08X is not aligned to a %d byte boundary.\n",
               (int) pDescriptor->m_sourceAddress, (int) minResolution));

    result = FALSE;

  }


  if ( pDescriptor->m_transferLength > DPIO2_DMA_MAX_TRANSFER_LENGTH ) {

    DPIO2_ERROR_MSG(("DMA block size (%d bytes), is longer than allowed (%d bytes).\n",
               (int) pDescriptor->m_transferLength, DPIO2_DMA_MAX_TRANSFER_LENGTH));

    result = FALSE;

  }


  if ( pDescriptor->m_transferLength < DPIO2_DMA_MIN_TRANSFER_LENGTH ) {

    DPIO2_ERROR_MSG(("DMA block size (%d bytes), is shorter than allowed (%d bytes).\n",
               (int) pDescriptor->m_transferLength, DPIO2_DMA_MIN_TRANSFER_LENGTH));

    result = FALSE;

  }


  if ( (pDescriptor->m_transferLength & (minResolution - 1)) != 0 ) {

    DPIO2_ERROR_MSG(("DMA block size (%d bytes) must be a multiplum of %d bytes.\n",
               (int) pDescriptor->m_transferLength, (int) minResolution));

    result = FALSE;

  }


  if ( !checkIsDescriptorIdValid(pDescriptor->m_nextDescriptorId) ) {

    result = FALSE;

  }

  return (result);

}


