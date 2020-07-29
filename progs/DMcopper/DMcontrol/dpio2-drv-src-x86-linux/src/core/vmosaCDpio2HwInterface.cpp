/****************************************************************************
Module      : vmosaCDpio2HwInterface.cpp

Description : Source file defining the CDpio2HwInterface class.
              A CDpio2HwInterface object creates and holds the aggregate
              objects which make up the main functionality of a DPIO2 module.
              In addition a CDpio2HwInterface object represents high level
              functionality, such as controlling the direction of the module.
              
Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Ported from CDpio2HwInterface.h

*****************************************************************************/
#include "vmosa.h"

#include "vmosaCDpio2HwInterface.h"
#include "vmosaCDpio2DmaController.h"
#include "CDpio2Fifo.h"
#include "CDpio2FrontEnd.h"
#include "CDpio2Leds.h"
#include "CDpio2Oscillator.h"
#include "vmosaCInterruptManager.h"

#include "Dpio2Defs.h"




#if 1 /*meh*/
#define MARKM printf ("Was Here %s,%d\n", __FILE__, __LINE__);
#endif
#ifndef DB
#define DB(args) \
       printf ( "DB: %s %d:\n", __FILE__, __LINE__); \
       printf args;
#endif



CDpio2HwInterface::CDpio2HwInterface ( HANDLE hDpio2 )
{

  m_hDpio2 = hDpio2;



  /* Define the control bits introduced in the HW interface for DPIO2.
   */
  m_pBitDisableDirectionJumper 
    = new CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
		      DPIO2_BIT_NUM_DISABLE_JUMPER_FOR_INPUT_OR_OUTPUT);

  m_pBitEnableAsOutput
    = new CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
		      DPIO2_BIT_NUM_ENABLE_AS_OUTPUT);

  m_pBitEnableInterruptPinB
    = new CControlBit(m_hDpio2, DPIO2_INTERRUPT_MASK_REGISTER, 
		      DPIO2_BIT_NUM_ENABLE_INTERRUPT_PIN_B);

  m_pFlashAddressBit20
    = new CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                      DPIO2_BIT_NUM_FLASH_ADDRESS_BIT_20_LATCH);

  m_pBitDirectionIsInput
    = new CStatusBit(m_hDpio2, DPIO2_STATUS_REGISTER, 
		     DPIO2_BIT_NUM_DIRECTION_STATE);

  m_pPciFpgaVersion
    = new CStatusBitField(m_hDpio2, DPIO2_PCI_VERSION_REGISTER,
                          DPIO2_MSB_NUM_PCI_FPGA_VERSION,
                          DPIO2_LSB_NUM_PCI_FPGA_VERSION);
                          
  m_pFrontEndFpgaVersion 
    = new CStatusBitField(m_hDpio2, DPIO2_FRONT_END_VERSION_REGISTER,
                          DPIO2_MSB_NUM_FRONT_END_FPGA_VERSION,
                          DPIO2_LSB_NUM_FRONT_END_FPGA_VERSION);
                          
  m_pTestRegister
    = new CControlRegister(m_hDpio2, DPIO2_GENERAL_PURPOSE_REGISTER);




  /* Create the aggregate objects.
   */
  createInterruptManager();
  createPrimaryDmaController();
  createFifo();
  createFrontEnd();
  createOscillator();
  createLeds();
  

  /* Initialize the DPIO2.
   */
  initialize();

}


CDpio2HwInterface::~CDpio2HwInterface()
{

  /* Delete the aggregate objects.
   */
  delete m_pLeds;
  delete m_pOscillator;
  delete m_pFrontEnd;
  delete m_pFifo;
  delete m_pPrimaryDmaController;
  delete m_pInterruptManager;


  /* Delete the control bits, status bits, and control registers created in this object.
   */
  delete m_pTestRegister;
  delete m_pBitDirectionIsInput;
  delete m_pBitEnableInterruptPinB;
  delete m_pFlashAddressBit20;
  delete m_pBitEnableAsOutput;
  delete m_pBitDisableDirectionJumper;

}



void CDpio2HwInterface::createInterruptManager()
{

  /* Local structure type for storing information about all interrupts in a table.
   */
  struct InterruptSourceInfo
  {

    int      sourceId;
    UINT32   enableRegisterOffset;
    int      enableBitNum;
    UINT32   statusRegisterOffset;
    int      statusBitNum;

  };


  struct InterruptSourceInfo   interruptInfo[] = 
  {
    { DPIO2_DMA_DONE_INTERRUPT, 
      DPIO2_INTERRUPT_MASK_REGISTER, DPIO2_BIT_NUM_DMA_DONE_INTERRUPT_ENABLE,
      DPIO2_INTERRUPT_STATUS_REGISTER, DPIO2_BIT_NUM_DMA_DONE_INTERRUPT_STATUS },

    { DPIO2_DMA_ERROR_INTERRUPT,
      DPIO2_INTERRUPT_MASK_REGISTER, DPIO2_BIT_NUM_DMA_ERROR_INTERRUPT_ENABLE,
      DPIO2_INTERRUPT_STATUS_REGISTER, DPIO2_BIT_NUM_DMA_ERROR_INTERRUPT_STATUS },

    { DPIO2_MASTER_WRITE_ERROR_INTERRUPT,
      DPIO2_INTERRUPT_MASK_REGISTER, DPIO2_BIT_NUM_MASTER_WRITE_ERROR_INTERRUPT_ENABLE,
      DPIO2_INTERRUPT_STATUS_REGISTER, DPIO2_BIT_NUM_MASTER_WRITE_ERROR_INTERRUPT_STATUS },

    { DPIO2_MASTER_READ_ERROR_INTERRUPT,
      DPIO2_INTERRUPT_MASK_REGISTER, DPIO2_BIT_NUM_MASTER_READ_ERROR_INTERRUPT_ENABLE,
      DPIO2_INTERRUPT_STATUS_REGISTER, DPIO2_BIT_NUM_MASTER_READ_ERROR_INTERRUPT_STATUS },

    { DPIO2_FIFO_NOT_EMPTY_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_1, DPIO2_BIT_NUM_FIFO_NOT_EMPTY_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_1, DPIO2_BIT_NUM_FIFO_NOT_EMPTY_INTERRUPT_STATUS },

    { DPIO2_FIFO_EMPTY_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_1, DPIO2_BIT_NUM_FIFO_EMPTY_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_1, DPIO2_BIT_NUM_FIFO_EMPTY_INTERRUPT_STATUS },

    { DPIO2_FIFO_HALF_FULL_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_1, DPIO2_BIT_NUM_FIFO_HALF_FULL_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_1, DPIO2_BIT_NUM_FIFO_HALF_FULL_INTERRUPT_STATUS },

    { DPIO2_FIFO_FULL_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_1, DPIO2_BIT_NUM_FIFO_FULL_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_1, DPIO2_BIT_NUM_FIFO_FULL_INTERRUPT_STATUS },

    { DPIO2_PIO1_ASSERTED_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_0, DPIO2_BIT_NUM_PIO1_ASSERTED_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_0, DPIO2_BIT_NUM_PIO1_ASSERTED_INTERRUPT_STATUS },

    { DPIO2_PIO2_ASSERTED_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_0, DPIO2_BIT_NUM_PIO2_ASSERTED_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_0, DPIO2_BIT_NUM_PIO2_ASSERTED_INTERRUPT_STATUS },

    { DPIO2_SYNC_ASSERTED_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_0, DPIO2_BIT_NUM_SYNC_ASSERTED_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_0, DPIO2_BIT_NUM_SYNC_ASSERTED_INTERRUPT_STATUS },

    { DPIO2_FIFO_OVERFLOW_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_0, DPIO2_BIT_NUM_FIFO_OVERFLOW_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_0, DPIO2_BIT_NUM_FIFO_OVERFLOW_INTERRUPT_STATUS },

    { DPIO2_SUSPEND_ASSERTED_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_0, DPIO2_BIT_NUM_SUSPEND_ASSERTED_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_0, DPIO2_BIT_NUM_SUSPEND_ASSERTED_INTERRUPT_STATUS },

    { DPIO2_FIFO_ALMOST_FULL_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_1, DPIO2_BIT_NUM_FIFO_ALMOST_FULL_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_1, DPIO2_BIT_NUM_FIFO_ALMOST_FULL_INTERRUPT_STATUS },

    { DPIO2_FIFO_ALMOST_EMPTY_INTERRUPT,
      DPIO2_FRONT_END_INTERRUPT_REGISTER_1, DPIO2_BIT_NUM_FIFO_ALMOST_EMPTY_INTERRUPT_ENABLE,
      DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_1, DPIO2_BIT_NUM_FIFO_ALMOST_EMPTY_INTERRUPT_STATUS }
		
  };


  CInterruptSource*    pSource;

  STATUS   status;
  int      iSource;
  int      numSourcesInTable;



  /**/
  m_pInterruptManager = new CInterruptManager(m_hDpio2,
                                              DPIO2_NEXT_FREE_INTERRUPT_SOURCE_ID,
                                              DPIO2_MAX_NUM_ACTIONS_PER_SOURCE);
  if ( m_pInterruptManager == NULL ) {

    throw;

  }


  /* Create all interrupt sources based on information in the private table
   * and add them to the interrupt server.
   */
  numSourcesInTable = sizeof(interruptInfo) / sizeof(struct InterruptSourceInfo);

  for (iSource = 0; iSource < numSourcesInTable; iSource++) {


    pSource = 
      new CInterruptSource(CControlBit(m_hDpio2, 
				       interruptInfo[iSource].enableRegisterOffset,
				       interruptInfo[iSource].enableBitNum),
			   CStatusBit(m_hDpio2, 
				      interruptInfo[iSource].statusRegisterOffset,
				      interruptInfo[iSource].statusBitNum));


    status = 
      m_pInterruptManager->addInterruptSource(interruptInfo[iSource].sourceId,
					      pSource,
					      interruptInfo[iSource].statusRegisterOffset,
					      interruptInfo[iSource].statusBitNum);

    if ( status != OK ) {
      
      DPIO2_ERROR_MSG(("Failed to add Interrupt Source with ID %d.\n",
                       interruptInfo[iSource].sourceId));
      
    }

  }

}


void CDpio2HwInterface::createPrimaryDmaController()
{

  m_pPrimaryDmaController = new CDpio2DmaController(

						    /**/
    m_hDpio2,
    DPIO2_SRAM_BASE_ADDRESS,
    DPIO2_NUM_DMA_DESCRIPTORS_IN_SRAM,
	
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
		DPIO2_BIT_NUM_ENABLE_END_OF_TRANSFER_FUNCTIONALITY),
    CControlBit(m_hDpio2, DPIO2_DMA_NEXT_DESCRIPTOR_ADDRESS_REGISTER,
		DPIO2_BIT_NUM_END_OF_CHAIN),
    CControlBit(m_hDpio2, DPIO2_DMA_NEXT_DESCRIPTOR_ADDRESS_REGISTER, 
		DPIO2_BIT_NUM_DIRECTION),
    CControlBit(m_hDpio2, DPIO2_DMA_NEXT_DESCRIPTOR_ADDRESS_REGISTER,
		DPIO2_BIT_NUM_ENABLE_INTERRUPT),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_DMA_DEMAND_MODE),
    CControlBit(m_hDpio2, DPIO2_DMA_CONTROL_REGISTER, 
		DPIO2_BIT_NUM_START_DMA),
    CControlBit(m_hDpio2, DPIO2_DMA_CONTROL_REGISTER, 
		DPIO2_BIT_NUM_PAUSE_DMA),
    CControlBit(m_hDpio2, DPIO2_DMA_CONTROL_REGISTER, 
		DPIO2_BIT_NUM_ENABLE_CLEAR_ALL_ON_STOP),
    CControlBit(m_hDpio2, DPIO2_DMA_NEXT_DESCRIPTOR_ADDRESS_REGISTER,
		DPIO2_BIT_NUM_ENABLE_DESCRIPTOR_FROM_LOCAL_BUS),
    CControlBit(m_hDpio2, DPIO2_DMA_CONTROL_REGISTER, 
		DPIO2_BIT_NUM_EOT_NEXT_DMA),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_SYNC_OPTIONS_REGISTER, 
		DPIO2_BIT_NUM_EOT_ON_SYNC_COUNT_ENABLE),
      
    CControlBitField(m_hDpio2, DPIO2_DMA_NEXT_DESCRIPTOR_ADDRESS_REGISTER,
                     DPIO2_MSB_NUM_NEXT_DESCRIPTOR_ADDRESS,
                     DPIO2_LSB_NUM_NEXT_DESCRIPTOR_ADDRESS),
    CControlBitField(m_hDpio2, DPIO2_DMA_TRANSFER_SIZE_REGISTER,
                     DPIO2_MSB_NUM_TRANSFER_SIZE,
                     DPIO2_LSB_NUM_TRANSFER_SIZE),
    CControlBitField(m_hDpio2, DPIO2_DMA_CONTROL_REGISTER,
                     DPIO2_MSB_NUM_PCI_COMMAND_MASTER_WRITE,
                     DPIO2_LSB_NUM_PCI_COMMAND_MASTER_WRITE),
    CControlBitField(m_hDpio2, DPIO2_DMA_CONTROL_REGISTER,
                     DPIO2_MSB_NUM_PCI_COMMAND_MASTER_READ,
                     DPIO2_LSB_NUM_PCI_COMMAND_MASTER_READ),
    CControlBitField(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_0,
                     DPIO2_MSB_NUM_SYNC_AND_EOT_MODE,
                     DPIO2_LSB_NUM_SYNC_AND_EOT_MODE),
    
    CControlDualByte(m_hDpio2, DPIO2_FRONT_EOT_SYNC_COUNTER_BYTE_0_REGISTER),

    CControlRegister(m_hDpio2, DPIO2_DMA_PCI_ADDRESS_REGISTER),

    CStatusBit(m_hDpio2, DPIO2_DMA_STATUS_REGISTER,
	       DPIO2_BIT_NUM_END_OF_CHAIN_DETECTED),
    CStatusBit(m_hDpio2, DPIO2_DMA_BYTE_COUNT_REGISTER,
	       DPIO2_BIT_NUM_BYTE_COUNT_OVERFLOW_FLAG),
    
    CStatusBitField(m_hDpio2, DPIO2_DMA_BYTE_COUNT_REGISTER,
                    DPIO2_MSB_NUM_REMAINING_BYTE_COUNT,
                    DPIO2_LSB_NUM_REMAINING_BYTE_COUNT),
    CStatusBitField(m_hDpio2, DPIO2_DMA_TRANSFERRED_BYTE_COUNT_REGISTER,
                    DPIO2_MSB_NUM_TRANSFERRED_BYTE_COUNT,
                    DPIO2_LSB_NUM_TRANSFERRED_BYTE_COUNT));

}


void CDpio2HwInterface::createFifo()
{

  m_pFifo = new CDpio2Fifo(
										
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_0,
		DPIO2_BIT_NUM_DISABLE_SUSPEND_FLOW_CONTROL),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_0,
		DPIO2_BIT_NUM_DISABLE_NRDY_FLOW_CONTROL),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_1,
		DPIO2_BIT_NUM_RESET_OCCURRED_FLAGS),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_RESET_FIFO),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_100MHZ_FIFO_CLOCK),

    CStatusBit(m_hDpio2, DPIO2_FRONT_END_OCCURRED_REGISTER, 
	       DPIO2_BIT_NUM_FIFO_FULL_FLAG_HAS_BEEN_ACTIVE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_OCCURRED_REGISTER,
	       DPIO2_BIT_NUM_FIFO_HALF_FULL_FLAG_HAS_BEEN_ACTIVE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_OCCURRED_REGISTER,
	       DPIO2_BIT_NUM_FIFO_EMPTY_FLAG_HAS_BEEN_ACTIVE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_OCCURRED_REGISTER, 
	       DPIO2_BIT_NUM_FIFO_ALMOST_EMPTY_FLAG_HAS_BEEN_ACTIVE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_OCCURRED_REGISTER,
	       DPIO2_BIT_NUM_FIFO_ALMOST_FULL_FLAG_HAS_BEEN_ACTIVE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_OCCURRED_REGISTER,
	       DPIO2_BIT_NUM_FIFO_OVERFLOW_HAS_OCCURRED),

    CStatusBitField(m_hDpio2, DPIO2_FRONT_END_FIFO_STATUS_REGISTER,
                    DPIO2_MSB_NUM_FIFO_STATUS,
                    DPIO2_LSB_NUM_FIFO_STATUS),
    CStatusBitField(m_hDpio2, DPIO2_STATUS_REGISTER,
                    DPIO2_MSB_NUM_FIFO_SIZE,
                    DPIO2_LSB_NUM_FIFO_SIZE));
}


void CDpio2HwInterface::createFrontEnd()
{

  m_pFrontEnd = new CDpio2FrontEnd(

    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_STROBE_GENERATION),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_0,
		DPIO2_BIT_NUM_ACTIVATE_TRANSFER),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
		DPIO2_BIT_NUM_ENABLE_PECL_STROBE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_ENABLE_REGISTER,
		DPIO2_BIT_NUM_ENABLE_RES1_OUTPUT),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_VALUE_REGISTER,
		DPIO2_BIT_NUM_RES1_OUTPUT_VALUE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_ENABLE_REGISTER,
		DPIO2_BIT_NUM_ENABLE_RES2_OUTPUT),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_VALUE_REGISTER,
		DPIO2_BIT_NUM_RES2_OUTPUT_VALUE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_ENABLE_REGISTER,
		DPIO2_BIT_NUM_ENABLE_PIO1_OUTPUT),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_VALUE_REGISTER, 
		DPIO2_BIT_NUM_PIO1_OUTPUT_VALUE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_ENABLE_REGISTER, 
		DPIO2_BIT_NUM_ENABLE_PIO2_OUTPUT),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_VALUE_REGISTER,
		DPIO2_BIT_NUM_PIO2_OUTPUT_VALUE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_INTERRUPT_REGISTER_0, 
		DPIO2_BIT_NUM_FRONT_END_INTERRUPTS_ENABLE),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_WORD_SWAP),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_AS_MASTER_INTERFACE),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_PLL),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_CLOCKING_ON_BOTH_STROBE_EDGES),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_0,
		DPIO2_BIT_NUM_ASSERT_SUSPEND),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_ADDRESSING_REGISTER,
		DPIO2_BIT_NUM_ENABLE_PIO_DECODING),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_ADDRESSING_REGISTER, 
		DPIO2_BIT_NUM_ENABLE_COUNTER_DECODING),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_ENABLE_REGISTER,
		DPIO2_BIT_NUM_ENABLE_RES3_OUTPUT),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_VALUE_REGISTER,
		DPIO2_BIT_NUM_RES3_OUTPUT_VALUE),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_DISABLE_JUMPER_FOR_STROBE_GENERATION),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_DISABLE_JUMPER_FOR_MASTER_INTERFACE),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_ENABLE_PECL_STROBE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_1,
		DPIO2_BIT_NUM_FLUSH_16BIT_PACKING_PIPELINE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_SYNC_OPTIONS_REGISTER,
		DPIO2_BIT_NUM_VIDEO_MODE_ENABLE),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_SYNC_OPTIONS_REGISTER,
		DPIO2_BIT_NUM_ENABLE_8BIT_SWAP),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_SYNC_OPTIONS_REGISTER,
		DPIO2_BIT_NUM_ENABLE_16BIT_SWAP),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_0,
		DPIO2_BIT_NUM_ENABLE_D0_FOR_SYNC),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_LED_CONTROL_REGISTER,
		DPIO2_BIT_NUM_ASSUME_DVALID_ALWAYS_ASSERTED),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_PLL_OPT1_MID),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_PLL_OPT1_HIGH),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_PLL_OPT0_MID),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
		DPIO2_BIT_NUM_PLL_OPT0_HIGH),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER,
		DPIO2_BIT_NUM_PLL_FS_MID),
    CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
		DPIO2_BIT_NUM_PLL_FS_HIGH),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_1,
		DPIO2_BIT_NUM_ENABLE_PATTERN_GENERATION),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_SYNC_OPTIONS_REGISTER,
		DPIO2_BIT_NUM_FLUSH_INTERNAL_FIFOS_IN_FRONTEND),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_ENABLE_REGISTER,
		DPIO2_BIT_NUM_ENABLE_PERSONALITY_PLL_FREQUENCY_SELECT_OUTPUT),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_OUTPUT_VALUE_REGISTER,
		DPIO2_BIT_NUM_PERSONALITY_PLL_FREQUENCY_SELECT_OUTPUT_VALUE),

    CControlBitField(m_hDpio2, DPIO2_FRONT_END_ADDRESSING_REGISTER,
                     DPIO2_MSB_NUM_PIO_ADDRESS,
                     DPIO2_LSB_NUM_PIO_ADDRESS),
    CControlBitField(m_hDpio2, DPIO2_FRONT_END_ADDRESSING_REGISTER,
                     DPIO2_MSB_NUM_PIO_MASK,
                     DPIO2_LSB_NUM_PIO_MASK),
    CControlBitField(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_0,
                     DPIO2_MSB_NUM_SYNC_AND_EOT_MODE,
                     DPIO2_LSB_NUM_SYNC_AND_EOT_MODE),
    CControlBitField(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_1,
                     DPIO2_MSB_NUM_PACKING_MODE,
                     DPIO2_LSB_NUM_PACKING_MODE),
    CControlBitField(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_1,
                     DPIO2_MSB_NUM_PATTERN_CODE,
                     DPIO2_LSB_NUM_PATTERN_CODE),
		
    CControlQuadByte(m_hDpio2, DPIO2_FRONT_END_START_COUNTER_BYTE_0_REGISTER),
    CControlQuadByte(m_hDpio2, DPIO2_FRONT_END_ENABLE_COUNTER_BYTE_0_REGISTER),
    CControlQuadByte(m_hDpio2, DPIO2_FRONT_END_WAIT_COUNTER_BYTE_0_REGISTER),
    CControlQuadByte(m_hDpio2, DPIO2_FRONT_END_SYNC_COUNTER_BYTE_0_REGISTER),
		
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_OCCURRED_REGISTER,
	       DPIO2_BIT_NUM_PIPELINE_IS_NOT_EMPTY),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_RES1_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_RES2_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_RES3_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_PIO1_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_PIO2_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_DIR_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_NRDY_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_PIO_STATUS_REGISTER,
	       DPIO2_BIT_NUM_SUSPEND_INPUT_VALUE),
    CStatusBit(m_hDpio2, DPIO2_STATUS_REGISTER,
	       DPIO2_BIT_NUM_GENERATE_STROBE_STATE),
    CStatusBit(m_hDpio2, DPIO2_STATUS_REGISTER,
	       DPIO2_BIT_NUM_ACT_AS_MASTER_STATE),
    CStatusBit(m_hDpio2, DPIO2_FRONT_END_CONTROL_REGISTER_1,
	       DPIO2_BIT_NUM_8BIT_4BIT_PACKING_AVAILABLE),
    CStatusBit(m_hDpio2, DPIO2_STATUS_REGISTER, 
	       DPIO2_BIT_NUM_FPDP2_SENSE),
    
    CStatusBitField(m_hDpio2, DPIO2_FRONT_END_PERSONALITY_ID_REGISTER,
                    DPIO2_MSB_NUM_PERSONALITY_ID,
                    DPIO2_LSB_NUM_PERSONALITY_ID),
    CStatusBitField(m_hDpio2, DPIO2_FRONT_END_PERSONALITY_ID_REGISTER,
                    DPIO2_MSB_NUM_MAIN_BOARD_ID,
                    DPIO2_LSB_NUM_MAIN_BOARD_ID));

}




void CDpio2HwInterface::createOscillator()
{

  UINT16   deviceId = 0;

  UINT8    revisionId = 0;

  STATUS   status;
 
  int   clockGeneratorId;




  /* Set default clock generator ID 
   */
  clockGeneratorId = DPIO2_PROGRAMMABLE_OSCILLATOR_ID_ICD2053B;


  /* Read the Device ID of the PCI device.
   */
  status = getDeviceId(deviceId);

  if ( status != OK ) {

    DPIO2_ERROR_MSG(("Failed to read the Device ID.\n"));

    return;

  }

#if 0
  printf ("PCI device number = 0x%x\n",deviceId);
#endif


  /* Check if it is a DPIO2_66 
   */
  if (deviceId == PCI_DEVICE_ID_DPIO2_66MHZ) {
    
    
   /* Read the Revision ID of the PCI device.
     */
    status = getRevisionId(revisionId);
    
    if ( status != OK ) {
      
      DPIO2_ERROR_MSG(("Failed to read the Revision ID.\n"));
      
      return;
      
    }

#if 0
    printf ("PCI revision id = 0x%x\n",revisionIdId);
#endif

   if (revisionId == DPIO2_REVISION_ID_PCB_B) {

      clockGeneratorId = DPIO2_PROGRAMMABLE_OSCILLATOR_ID_CY22150;

    }


  }



  /* Create oscillator object 
   */
  switch (clockGeneratorId) {


  case DPIO2_PROGRAMMABLE_OSCILLATOR_ID_ICD2053B:

    m_pOscillator = new CDpio2OscillatorICD2053B(

      DPIO2_PROGRAMMABLE_OSCILLATOR_REFERENCE_FREQUENCY,
    
      CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                  DPIO2_BIT_NUM_CLOCK_PROGRAMMABLE_OSCILLATOR),
      CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                  DPIO2_BIT_NUM_DATA_PROGRAMMABLE_OSCILLATOR));

    break;

    
  case DPIO2_PROGRAMMABLE_OSCILLATOR_ID_CY22150:

   m_pOscillator = new CDpio2OscillatorCY22150(

      DPIO2_PROGRAMMABLE_OSCILLATOR_REFERENCE_FREQUENCY,
    
      CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                  DPIO2_BIT_NUM_CLOCK_PROGRAMMABLE_OSCILLATOR),
      CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                  DPIO2_BIT_NUM_DATA_PROGRAMMABLE_OSCILLATOR),
      CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                  DPIO2_BIT_NUM_3STATE_DATA_PROGRAMMABLE_OSCILLATOR));

    break;


  default:

    DPIO2_ERROR_MSG(("Failed to determ the oscillator ID\n"));

    m_pOscillator = new CDpio2OscillatorICD2053B(

      DPIO2_PROGRAMMABLE_OSCILLATOR_REFERENCE_FREQUENCY,
    
      CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                  DPIO2_BIT_NUM_CLOCK_PROGRAMMABLE_OSCILLATOR),
      CControlBit(m_hDpio2, DPIO2_CONFIGURATION_REGISTER, 
                  DPIO2_BIT_NUM_DATA_PROGRAMMABLE_OSCILLATOR));
    
    break;

  }

  
  return;

}





void CDpio2HwInterface::createLeds()
{

  m_pLeds = new CDpio2Leds(

    CControlBit(m_hDpio2, DPIO2_FRONT_END_LED_CONTROL_REGISTER,
		DPIO2_BIT_NUM_ENABLE_NRDY_LED),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_LED_CONTROL_REGISTER, 
		DPIO2_BIT_NUM_ENABLE_CLK_LED),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_LED_CONTROL_REGISTER,
		DPIO2_BIT_NUM_ENABLE_SUSPEND_LED),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_LED_CONTROL_REGISTER,
		DPIO2_BIT_NUM_ENABLE_SYNC_LED),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_LED_CONTROL_REGISTER,
		DPIO2_BIT_NUM_ENABLE_DVALID_LED),
    CControlBit(m_hDpio2, DPIO2_FRONT_END_LED_CONTROL_REGISTER, 
		DPIO2_BIT_NUM_ENABLE_LED_TEST));

}




void CDpio2HwInterface::initialize()
{

  UINT32   resultingFrequency;

  CDpio2FrontEnd*   pDpio2FrontEnd;


  pDpio2FrontEnd = static_cast<CDpio2FrontEnd*>(m_pFrontEnd);


  /* Enable all LEDs and force them on while initializing.
   */
  m_pLeds->enableNrdyLed();
  m_pLeds->enableClkLed();
  m_pLeds->enableSuspendLed();
  m_pLeds->enableSyncLed();
  m_pLeds->enableDvalidLed();
  
  m_pLeds->enableLedTest();


  /* Set default frequencies based on type of personality module.
   */
  m_pOscillator->setFrequency((20*1000*1000), resultingFrequency);

  DEBUG_MSG(("Programmable oscillator set to %dHz\n", (int)resultingFrequency), FALSE);


  /* Set the default direction of the DPIO2 module according to the jumper.
   */
  m_pBitDisableDirectionJumper->clear();
  if ( checkIsOutput() ) {
    
    enableAsOutput();

  } else {

    enableAsInput();

  }
  m_pBitDisableDirectionJumper->set();



  /* Configure the DPIO2 Module to receive strobe using the TTL strobe signal.
   */
  pDpio2FrontEnd->disableStrobeGenerationJumper();
  pDpio2FrontEnd->disableStrobeGeneration();
  pDpio2FrontEnd->selectPrimaryStrobe();
  


  /* Set the default state of Master Interface of the DPIO2 module according to jumper.
   */
  pDpio2FrontEnd->enableMasterInterfaceJumper();
  if ( pDpio2FrontEnd->checkIsMasterInterfaceEnabled() ) {

    pDpio2FrontEnd->enableMasterInterface();

  } else {

    pDpio2FrontEnd->disableMasterInterface();

  }
  pDpio2FrontEnd->disableMasterInterfaceJumper();


  /* Make sure that the FPDP interface is disabled.
   */
  m_pFrontEnd->stopTransfer();


  /* Make sure that the DMA controller is cleared.
   */
  m_pPrimaryDmaController->enableClearingOnStop();
  m_pPrimaryDmaController->stopDmaTransfer();
  m_pPrimaryDmaController->disableClearingOnStop();


  /* Reset the FIFO.
   */
  m_pFifo->flushFifo();


  /* Configure the DMA controller to use Memory Write as the default write command,
   * and Memory Read Multiple as the default read command.
   */
  m_pPrimaryDmaController->setPciWriteCommand(PCI_MEMORY_WRITE_COMMAND);
  m_pPrimaryDmaController->setPciReadCommand(PCI_MEMORY_READ_MULTIPLE_COMMAND);


  /* Enable Front End interrupts.
   */
  m_pFrontEnd->enableFrontEndInterrupts();


  /* Disable LED testing to signal that initialization is done.
   */
  m_pLeds->disableLedTest();


}












UINT8 CDpio2HwInterface::getLatencyTimer()
{

  STATUS status;

  UINT8  value;


  status = vmosa_pci_config_8_read ( m_hDpio2, PCI_CONFIG_LATENCY_TIMER, &value );

  if ( status == ERROR ) {

    throw;

  }

  return ( value );

}


void CDpio2HwInterface::setLatencyTimer(UINT8 timerValue)
{

  STATUS status;


  status = vmosa_pci_config_8_write ( m_hDpio2, PCI_CONFIG_LATENCY_TIMER, timerValue );

  if ( status == ERROR ) {

    throw;

  }

}


BOOL CDpio2HwInterface::checkHasMasterDataParityErrorOccurred()
{

  BOOL    flag;
  
  UINT16  value;


  value = readPciConfigStatus();

  if ( (value & STATUS_BIT_MASK_MASTER_DATA_PARITY_ERROR) == 0 ) {

    flag = FALSE;
    
  } else {
      
    flag = TRUE;

  }


  return (flag);

}


BOOL CDpio2HwInterface::checkIsTargetAbortSignaled()
{

  UINT16   value;

  BOOL     flag;


  value = readPciConfigStatus();

  if ( (value & STATUS_BIT_SIGNALED_TARGET_ABORT) == 0 ) {

    flag = FALSE;
    
  } else {
      
    flag = TRUE;

  }


  return (flag);

}


BOOL CDpio2HwInterface::checkIsTargetAbortReceived()
{

  UINT16   value;

  BOOL     flag;


  value = readPciConfigStatus();

  if ( (value & STATUS_BIT_RECEIVED_TARGET_ABORT) == 0 ) {

    flag = FALSE;
    
  } else {
      
    flag = TRUE;

  }


  return (flag);

}


BOOL CDpio2HwInterface::checkIsMasterAbortReceived()
{

  UINT16   value;

  BOOL     flag;


  value = readPciConfigStatus();

  if ( (value & STATUS_BIT_RECEIVED_MASTER_ABORT) == 0 ) {

    flag = FALSE;
    
  } else {
      
    flag = TRUE;

  }


  return (flag);

}


BOOL CDpio2HwInterface::checkIsSystemErrorSignaled()
{

  UINT16   value;

  BOOL     flag;


  value = readPciConfigStatus();

  if ( (value & STATUS_BIT_SIGNALED_SYSTEM_ERROR) == 0 ) {

    flag = FALSE;
    
  } else {
      
    flag = TRUE;

  }


  return (flag);

}


UINT16 CDpio2HwInterface::readPciConfigStatus()
{

  STATUS status;

  UINT16  value;


  status = vmosa_pci_config_16_read ( m_hDpio2, PCI_CONFIG_STATUS, &value );

  if ( status == ERROR ) {

    throw;

  }


  return ( value );

}
