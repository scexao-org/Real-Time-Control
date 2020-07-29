/****************************************************************************
Module      : Dpio2Defs.h

Description : Header file declaring constants representing HW entities.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01l,08jul2004,meh Added PCI_CONFIG_STATUS  PCI_CONFIG_LATENCY_TIMER.
  01k,08jul2004,meh Made changes necessary to compile file under VMOSA.
  01j,11jan2004,men Added DPIO2_BIT_NUM_3STATE_DATA_PROGRAMMABLE_OSCILLATOR
  01i,08jan2004,meh Added DPIO2_PROGRAMMABLE_OSCILLATOR_ID_ICD2053B
                          DPIO2_PROGRAMMABLE_OSCILLATOR_ID_CY22150
                          DPIO2_ECO_BO
                          DPIO2_BIT_NUM_SERIAL_DATA_INPUT
                          DPIO2_BIT_NUM_SERIAL_CLOCK
                          DPIO2_BIT_NUM_SERIAL_RESET     
  01h,01nov2002,nib  Added PCI_DEVICE_ID_DPIO2_66MHZ.
  01g,19sep2002,nib  Renamed CONSTANT_PATTERN to COUNTER_PATTERN_WITH_PROGRAMMABLE_START.
  01f,13sep2002,nib  Added definitions for Transferred Byte Count Register.
  01e,18mar2002,nib  Updated file for use in a MCOS environment.
  01d,28oct2001,nib  Updated file to compile in a Windows environment.
  01c,24sep2001,nib  Updated register definitions according to HW update.
  01b,10aug2001,nib  Corrected constants representing input bits 
                     of PIO2, PIO1, RES3, and RES1.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef DPIO2DEFS_H
#define DPIO2DEFS_H

#ifdef VXWORKS
#include "vxWorks.h"
#endif

#ifdef WIN32
#include <windows.h>
#include "WindowsTypes.h"
#endif

#ifdef MCOS
#include "McosTypes.h"
#endif

#ifdef VMOSA
#include "vmosa.h"
#endif


#ifndef PCI_VENDOR_ID_VMETRO
#define   PCI_VENDOR_ID_VMETRO   (0x129A)
#endif

#ifndef PCI_DEVICE_ID_DPIO2
#define   PCI_DEVICE_ID_DPIO2    (0xDD11)
#endif

#ifndef PCI_DEVICE_ID_DPIO2_66MHZ
#define   PCI_DEVICE_ID_DPIO2_66MHZ    (0xDD12)
#endif


#define PCI_CONFIG_STATUS         (0x06)
#define PCI_CONFIG_LATENCY_TIMER  (0x0d)







typedef enum
{

  DPIO2_REVISION_ID_PCB_A = 0x00,
  DPIO2_REVISION_ID_PCB_B = 0x01

} DPIO2_REVISION_ID;


#define DPIO2_FPDP_MODULE          0x20
#define DPIO2_LVDS_MODULE          0x26
#define DPIO2_PECL_INPUT_MODULE    0x22
#define DPIO2_PECL_OUTPUT_MODULE   0x23
#define DPIO2_RS422_INPUT_MODULE   0x24
#define DPIO2_RS422_OUTPUT_MODULE  0x25


typedef enum
{

  DPIO2_DMA_DONE_INTERRUPT,
  DPIO2_DMA_ERROR_INTERRUPT,

  DPIO2_MASTER_WRITE_ERROR_INTERRUPT,
  DPIO2_MASTER_READ_ERROR_INTERRUPT,

  DPIO2_FIFO_NOT_EMPTY_INTERRUPT,
  DPIO2_FIFO_EMPTY_INTERRUPT,
  DPIO2_FIFO_ALMOST_EMPTY_INTERRUPT,
  DPIO2_FIFO_HALF_FULL_INTERRUPT,
  DPIO2_FIFO_ALMOST_FULL_INTERRUPT,
  DPIO2_FIFO_FULL_INTERRUPT,
  DPIO2_FIFO_OVERFLOW_INTERRUPT,
  DPIO2_PIO1_ASSERTED_INTERRUPT,
  DPIO2_PIO2_ASSERTED_INTERRUPT,
  DPIO2_SYNC_ASSERTED_INTERRUPT,
  DPIO2_SUSPEND_ASSERTED_INTERRUPT,

  DPIO2_NEXT_FREE_INTERRUPT_SOURCE_ID,

  DPIO2_INVALID_INTERRUPT_SOURCE_ID = (-1)

} DPIO2_INTERRUPT_SOURCE_ID;


typedef enum
{

  DPIO2_NO_SYNC_GENERATION     = 0x0,
  DPIO2_SYNC_BEFORE_DATA       = 0x1,
  DPIO2_SYNC_AT_END_OF_FRAME   = 0x2,
  DPIO2_SYNC_AT_START_OF_FRAME = 0x3,
  DPIO2_SYNC_ON_ODD_FRAME      = 0x4,
  DPIO2_SYNC_ON_COUNT          = 0x5

} DPIO2_SYNC_GENERATION_MODE;


typedef enum
{

  DPIO2_NO_SYNC_HANDLING           = 0x0,
  DPIO2_SYNC_STARTS_DATA_RECEPTION = 0x1

} DPIO2_SYNC_RECEPTION_MODE;


typedef enum
{

  DPIO2_NO_EOT         = 0x0,
  DPIO2_SYNC_MARKS_EOT = 0x2,
  DPIO2_PIO1_MARKS_EOT = 0x3,
  DPIO2_PIO2_MARKS_EOT = 0x4,
  DPIO2_RES1_MARKS_EOT = 0x5

} DPIO2_END_OF_TRANSFER_MODE;


typedef enum
{

  NO_SWAP,
  SWAP_8BIT,
  SWAP_16BIT,
  SWAP_8BIT_16BIT,
  SWAP_32BIT,
  SWAP_8BIT_32BIT,
  SWAP_16BIT_32BIT,
  SWAP_8BIT_16BIT_32BIT

} DPIO2_DATA_SWAP_MODE;


typedef enum
{

  BELOW_OR_EQUAL_TO_5MHZ,
  ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ,
  ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ,
  ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ,
  ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ,
  ABOVE_100MHZ,
  /*meh, new 5MHZ range, used from _66 eco B0 and newer */
  ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ,
  ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ,
  ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ,
  ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ,
  ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ,
  ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ,
  ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ,
  ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ,
  ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ,
  ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ,
  ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ,
  ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ,
  ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ,
  ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ,
  ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ,
  ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ,
  ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ,
  ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ,

} DPIO2_STROBE_FREQUENCY_RANGE;


typedef enum
{

  NO_PACKING = 0,
  PACK_16_LEAST_SIGNIFICANT_FPDP_BITS = 2,
  PACK_10_LEAST_SIGNIFICANT_FPDP_BITS = 3,
  PACK_8_LEAST_SIGNIFICANT_FPDP_BITS = 4,
  PACK_4_LEAST_SIGNIFICANT_FPDP_BITS = 5,
  PACK_16_MOST_SIGNIFICANT_FPDP_BITS = 6

} DPIO2_PACKING_MODE;


typedef enum
{

  WALKING_ONE_PATTERN = 0,
  WALKING_ZERO_PATTERN = 1,
  COUNTER_PATTERN = 2,
  COUNTER_PATTERN_WITH_PROGRAMMABLE_START = 3

} DPIO2_TEST_PATTERN;


/* Private structure type used to access descriptors in memory
 */
typedef struct
{

  UINT32   nextDescriptorAddress;
  UINT32   pciAddress;
  UINT32   d64;
  UINT32   transferSize;

} CDpio2PhysicalDescriptor;



/*meh*/
#define   DPIO2_PROGRAMMABLE_OSCILLATOR_ID_ICD2053B           (0x2053B)
#define   DPIO2_PROGRAMMABLE_OSCILLATOR_ID_CY22150            (0x22150)
#define   DPIO2_ECO_BO                                        (0xB0)

#define   DPIO2_PROGRAMMABLE_OSCILLATOR_REFERENCE_FREQUENCY   (20*1000*1000)
#define   DPIO2_MIN_PLL_LOCK_FREQUENCY                        (10*1000*1000)


#define   DPIO2_PCI_VERSION_REGISTER   0x00000000

#define   DPIO2_MSB_NUM_PCI_FPGA_VERSION   7
#define   DPIO2_LSB_NUM_PCI_FPGA_VERSION   0


#define   DPIO2_RETRY_REGISTER   0x00000004


#define   DPIO2_GENERAL_PURPOSE_REGISTER   0x00000008


#define   DPIO2_CONFIGURATION_REGISTER   0x0000000C

#define   DPIO2_BIT_NUM_PLL_OPT1_MID                           31
#define   DPIO2_BIT_NUM_PLL_OPT1_HIGH                          30
#define   DPIO2_BIT_NUM_PLL_OPT0_MID                           29
#define   DPIO2_BIT_NUM_PLL_OPT0_HIGH                          28
#define   DPIO2_BIT_NUM_PLL_FS_MID                             27
#define   DPIO2_BIT_NUM_PLL_FS_HIGH                            26
#define   DPIO2_BIT_NUM_FLASH_ADDRESS_BIT_20_LATCH             22
#define   DPIO2_BIT_NUM_DATA_PROGRAMMABLE_OSCILLATOR           21
#define   DPIO2_BIT_NUM_CLOCK_PROGRAMMABLE_OSCILLATOR          20
/*meh*/
#define   DPIO2_BIT_NUM_3STATE_DATA_PROGRAMMABLE_OSCILLATOR    19
#define   DPIO2_BIT_NUM_SERIAL_DATA_INPUT                      18
#define   DPIO2_BIT_NUM_SERIAL_CLOCK                           17
#define   DPIO2_BIT_NUM_SERIAL_RESET                           16

#define   DPIO2_BIT_NUM_ENABLE_DMA_DEMAND_MODE                 15
#define   DPIO2_BIT_NUM_ENABLE_WORD_SWAP                       14
#define   DPIO2_BIT_NUM_ENABLE_END_OF_TRANSFER_FUNCTIONALITY   13
#define   DPIO2_BIT_NUM_RESET_FIFO                             12
#define   DPIO2_BIT_NUM_ENABLE_PLL                             11
#define   DPIO2_BIT_NUM_ENABLE_CLOCKING_ON_BOTH_STROBE_EDGES   10
#define   DPIO2_BIT_NUM_ENABLE_100MHZ_FIFO_CLOCK                9
#define   DPIO2_BIT_NUM_ENABLE_PECL_STROBE                      8
#define   DPIO2_BIT_NUM_DISABLE_JUMPER_FOR_STROBE_GENERATION    6
#define   DPIO2_BIT_NUM_DISABLE_JUMPER_FOR_MASTER_INTERFACE     5
#define   DPIO2_BIT_NUM_DISABLE_JUMPER_FOR_INPUT_OR_OUTPUT      4
#define   DPIO2_BIT_NUM_ENABLE_STROBE_GENERATION                2
#define   DPIO2_BIT_NUM_ENABLE_AS_MASTER_INTERFACE              1
#define   DPIO2_BIT_NUM_ENABLE_AS_OUTPUT                        0


#define   DPIO2_STATUS_REGISTER   0x00000010

#define   DPIO2_BIT_NUM_FPDP2_SENSE            7
#define   DPIO2_BIT_NUM_GENERATE_STROBE_STATE  6
#define   DPIO2_BIT_NUM_ACT_AS_MASTER_STATE    5
#define   DPIO2_BIT_NUM_DIRECTION_STATE        4
#define   DPIO2_MSB_NUM_FIFO_SIZE              1
#define   DPIO2_LSB_NUM_FIFO_SIZE              0

#define   DPIO2_FIFO_SIZE_32K    0x00000000
#define   DPIO2_FIFO_SIZE_128K   0x00000001


#define   DPIO2_DMA_NEXT_DESCRIPTOR_ADDRESS_REGISTER   0x00000100

#define   DPIO2_BIT_NUM_END_OF_CHAIN                       31
#define   DPIO2_BIT_NUM_ENABLE_DESCRIPTOR_FROM_LOCAL_BUS   30
#define   DPIO2_BIT_NUM_DIRECTION                          29
#define   DPIO2_BIT_NUM_ENABLE_INTERRUPT                   28
#define   DPIO2_MSB_NUM_NEXT_DESCRIPTOR_ADDRESS            23
#define   DPIO2_LSB_NUM_NEXT_DESCRIPTOR_ADDRESS             0


#define   DPIO2_DMA_PCI_ADDRESS_REGISTER   0x00000104

#define   DPIO2_DMA_D64_REGISTER   0x00000108

#define   DPIO2_BIT_NUM_ENABLE_D64_ACCESSES          29
#define   DPIO2_BIT_NUM_BLOCK_IS_NOT_LAST_IN_FRAME   28


#define   DPIO2_DMA_TRANSFER_SIZE_REGISTER   0x0000010C

#define   DPIO2_MSB_NUM_TRANSFER_SIZE   23
#define   DPIO2_LSB_NUM_TRANSFER_SIZE    0

#define   DPIO2_DMA_MAX_TRANSFER_LENGTH   ((1 << (DPIO2_MSB_NUM_TRANSFER_SIZE + 1)) - 1)
#define   DPIO2_DMA_MIN_TRANSFER_LENGTH   (16)


#define   DPIO2_DMA_CONTROL_REGISTER   0x00000110

#define   DPIO2_BIT_NUM_START_DMA                  31
#define   DPIO2_BIT_NUM_PAUSE_DMA                  30
#define   DPIO2_BIT_NUM_ENABLE_CLEAR_ALL_ON_STOP   29
#define   DPIO2_BIT_NUM_EOT_NEXT_DMA               28
#define   DPIO2_MSB_NUM_PCI_COMMAND_MASTER_WRITE    7
#define   DPIO2_LSB_NUM_PCI_COMMAND_MASTER_WRITE    4
#define   DPIO2_MSB_NUM_PCI_COMMAND_MASTER_READ     3
#define   DPIO2_LSB_NUM_PCI_COMMAND_MASTER_READ     0


#define   DPIO2_DMA_STATUS_REGISTER   0x00000114

#define   DPIO2_BIT_NUM_ERROR                15
#define   DPIO2_BIT_NUM_ABORTED              11
#define   DPIO2_BIT_NUM_D64_ACCESSES_USED     9
#define   DPIO2_BIT_NUM_RETRY_EXPIRED         8
#define   DPIO2_BIT_NUM_END_OF_BLOCK_DETECTED 2
#define   DPIO2_BIT_NUM_END_OF_CHAIN_DETECTED 1
#define   DPIO2_BIT_NUM_DMA_ACTIVE            0


#define   DPIO2_DMA_BYTE_COUNT_REGISTER   0x00000118

#define   DPIO2_BIT_NUM_BYTE_COUNT_OVERFLOW_FLAG 31
#define   DPIO2_MSB_NUM_REMAINING_BYTE_COUNT     23
#define   DPIO2_LSB_NUM_REMAINING_BYTE_COUNT      0


#define   DPIO2_DMA_TRANSFERRED_BYTE_COUNT_REGISTER   0x0000011C
#define   DPIO2_MSB_NUM_TRANSFERRED_BYTE_COUNT     23
#define   DPIO2_LSB_NUM_TRANSFERRED_BYTE_COUNT      0


#define   DPIO2_INTERRUPT_STATUS_REGISTER   0x00000700

#define DPIO2_BIT_NUM_MASTER_WRITE_ERROR_INTERRUPT_STATUS   11
#define DPIO2_BIT_NUM_MASTER_READ_ERROR_INTERRUPT_STATUS    10
#define DPIO2_BIT_NUM_DMA_ERROR_INTERRUPT_STATUS             5
#define DPIO2_BIT_NUM_DMA_DONE_INTERRUPT_STATUS              0


#define   DPIO2_INTERRUPT_MASK_REGISTER   0x00000704

#define DPIO2_BIT_NUM_ENABLE_INTERRUPT_PIN_B                31
#define DPIO2_BIT_NUM_MASTER_WRITE_ERROR_INTERRUPT_ENABLE   11
#define DPIO2_BIT_NUM_MASTER_READ_ERROR_INTERRUPT_ENABLE    10
#define DPIO2_BIT_NUM_DMA_ERROR_INTERRUPT_ENABLE             5
#define DPIO2_BIT_NUM_DMA_DONE_INTERRUPT_ENABLE              0


#define   DPIO2_INTERRUPT_GLOBAL_MASK_REGISTER   0x00000708

#define DPIO2_BIT_NUM_ROUTE_INTERRUPT_TO_PCI_DISABLE    0

#define DPIO2_ROUTE_INTERRUPT_TO_PCI_BITMASK   0x00000001


#define   DPIO2_FRONT_END_CONTROL_REGISTER_0   0x00100000

#define   DPIO2_BIT_NUM_DISABLE_NRDY_FLOW_CONTROL       7
#define   DPIO2_BIT_NUM_ASSERT_SUSPEND                  6
#define   DPIO2_BIT_NUM_DISABLE_SUSPEND_FLOW_CONTROL    5
#define   DPIO2_BIT_NUM_ENABLE_D0_FOR_SYNC              4
#define   DPIO2_MSB_NUM_SYNC_AND_EOT_MODE               3
#define   DPIO2_LSB_NUM_SYNC_AND_EOT_MODE               1
#define   DPIO2_BIT_NUM_ACTIVATE_TRANSFER               0


#define   DPIO2_FRONT_END_CONTROL_REGISTER_1   0x00100004

#define   DPIO2_MSB_NUM_PATTERN_CODE                 7
#define   DPIO2_LSB_NUM_PATTERN_CODE                 6
#define   DPIO2_BIT_NUM_ENABLE_PATTERN_GENERATION    5
#define   DPIO2_BIT_NUM_RESET_OCCURRED_FLAGS         4
#define   DPIO2_BIT_NUM_FLUSH_16BIT_PACKING_PIPELINE 3
#define   DPIO2_MSB_NUM_PACKING_MODE                 2
#define   DPIO2_LSB_NUM_PACKING_MODE                 1
#define   DPIO2_BIT_NUM_8BIT_4BIT_PACKING_AVAILABLE  0

#define   DPIO2_FRONT_END_LED_CONTROL_REGISTER   0x00100008

#define   DPIO2_BIT_NUM_ENABLE_LED_TEST                7
#define   DPIO2_BIT_NUM_ASSUME_DVALID_ALWAYS_ASSERTED  6
#define   DPIO2_BIT_NUM_ENABLE_NRDY_LED                4
#define   DPIO2_BIT_NUM_ENABLE_CLK_LED                 3
#define   DPIO2_BIT_NUM_ENABLE_SUSPEND_LED             2
#define   DPIO2_BIT_NUM_ENABLE_SYNC_LED                1
#define   DPIO2_BIT_NUM_ENABLE_DVALID_LED              0


#define   DPIO2_FRONT_END_ADDRESSING_REGISTER   0x0010000C

#define   DPIO2_MSB_NUM_PIO_MASK                   5
#define   DPIO2_LSB_NUM_PIO_MASK                   4
#define   DPIO2_MSB_NUM_PIO_ADDRESS                3
#define   DPIO2_LSB_NUM_PIO_ADDRESS                2
#define   DPIO2_BIT_NUM_ENABLE_PIO_DECODING        1
#define   DPIO2_BIT_NUM_ENABLE_COUNTER_DECODING    0


#define   DPIO2_FRONT_END_SYNC_OPTIONS_REGISTER   0x00100010

#define DPIO2_BIT_NUM_FLUSH_INTERNAL_FIFOS_IN_FRONTEND   7
#define DPIO2_BIT_NUM_EOT_ON_SYNC_COUNT_ENABLE           3
#define DPIO2_BIT_NUM_VIDEO_MODE_ENABLE                  2
#define DPIO2_BIT_NUM_ENABLE_8BIT_SWAP                   1
#define DPIO2_BIT_NUM_ENABLE_16BIT_SWAP                  0


#define   DPIO2_FRONT_END_INTERRUPT_REGISTER_0   0x00100014

#define DPIO2_BIT_NUM_SYNC_ASSERTED_INTERRUPT_ENABLE      7
#define DPIO2_BIT_NUM_FIFO_OVERFLOW_INTERRUPT_ENABLE      6
#define DPIO2_BIT_NUM_SUSPEND_ASSERTED_INTERRUPT_ENABLE   5
#define DPIO2_BIT_NUM_PIO2_ASSERTED_INTERRUPT_ENABLE      4
#define DPIO2_BIT_NUM_PIO1_ASSERTED_INTERRUPT_ENABLE      3
#define DPIO2_BIT_NUM_FRONT_END_INTERRUPTS_ENABLE         0


#define   DPIO2_FRONT_END_INTERRUPT_REGISTER_1   0x00100018

#define DPIO2_BIT_NUM_FIFO_NOT_EMPTY_INTERRUPT_ENABLE      5
#define DPIO2_BIT_NUM_FIFO_EMPTY_INTERRUPT_ENABLE          4
#define DPIO2_BIT_NUM_FIFO_ALMOST_EMPTY_INTERRUPT_ENABLE   3
#define DPIO2_BIT_NUM_FIFO_HALF_FULL_INTERRUPT_ENABLE      2
#define DPIO2_BIT_NUM_FIFO_ALMOST_FULL_INTERRUPT_ENABLE    1
#define DPIO2_BIT_NUM_FIFO_FULL_INTERRUPT_ENABLE           0


#define   DPIO2_FRONT_END_OUTPUT_VALUE_REGISTER   0x0010001C

#define   DPIO2_BIT_NUM_PERSONALITY_PLL_FREQUENCY_SELECT_OUTPUT_VALUE    5
#define   DPIO2_BIT_NUM_RES3_OUTPUT_VALUE                                4
#define   DPIO2_BIT_NUM_RES2_OUTPUT_VALUE                                3
#define   DPIO2_BIT_NUM_RES1_OUTPUT_VALUE                                2
#define   DPIO2_BIT_NUM_PIO2_OUTPUT_VALUE                                1
#define   DPIO2_BIT_NUM_PIO1_OUTPUT_VALUE                                0


#define   DPIO2_FRONT_END_OUTPUT_ENABLE_REGISTER   0x00100020

#define   DPIO2_BIT_NUM_ENABLE_PERSONALITY_PLL_FREQUENCY_SELECT_OUTPUT    5
#define   DPIO2_BIT_NUM_ENABLE_RES3_OUTPUT                                4
#define   DPIO2_BIT_NUM_ENABLE_RES2_OUTPUT                                3
#define   DPIO2_BIT_NUM_ENABLE_RES1_OUTPUT                                2
#define   DPIO2_BIT_NUM_ENABLE_PIO2_OUTPUT                                1
#define   DPIO2_BIT_NUM_ENABLE_PIO1_OUTPUT                                0


#define   DPIO2_FRONT_END_START_COUNTER_BYTE_0_REGISTER   0x00100028


#define   DPIO2_FRONT_END_ENABLE_COUNTER_BYTE_0_REGISTER   0x00100038


#define   DPIO2_FRONT_END_WAIT_COUNTER_BYTE_0_REGISTER   0x00100048


#define   DPIO2_FRONT_END_SYNC_COUNTER_BYTE_0_REGISTER   0x00100058


#define   DPIO2_FRONT_EOT_SYNC_COUNTER_BYTE_0_REGISTER   0x00100068


#define   DPIO2_FRONT_END_FIFO_STATUS_REGISTER   0x00100100

#define   DPIO2_MSB_NUM_FIFO_STATUS               4
#define   DPIO2_BIT_NUM_FIFO_EMPTY_FLAG           4
#define   DPIO2_BIT_NUM_FIFO_ALMOST_EMPTY_FLAG    3
#define   DPIO2_BIT_NUM_FIFO_HALF_FULL_FLAG       2
#define   DPIO2_BIT_NUM_FIFO_ALMOST_FULL_FLAG     1
#define   DPIO2_BIT_NUM_FIFO_FULL_FLAG            0
#define   DPIO2_LSB_NUM_FIFO_STATUS               0


#define   DPIO2_FRONT_END_PIO_STATUS_REGISTER   0x00100104

#define   DPIO2_BIT_NUM_DIR_INPUT_VALUE        7
#define   DPIO2_BIT_NUM_NRDY_INPUT_VALUE       6
#define   DPIO2_BIT_NUM_SUSPEND_INPUT_VALUE    5
#define   DPIO2_BIT_NUM_RES3_INPUT_VALUE       4
#define   DPIO2_BIT_NUM_RES2_INPUT_VALUE       3
#define   DPIO2_BIT_NUM_RES1_INPUT_VALUE       2
#define   DPIO2_BIT_NUM_PIO2_INPUT_VALUE       1
#define   DPIO2_BIT_NUM_PIO1_INPUT_VALUE       0


#define   DPIO2_FRONT_END_OCCURRED_REGISTER   0x00100108

#define   DPIO2_BIT_NUM_FIFO_OVERFLOW_HAS_OCCURRED                6
#define   DPIO2_BIT_NUM_PIPELINE_IS_NOT_EMPTY                     5
#define   DPIO2_BIT_NUM_FIFO_EMPTY_FLAG_HAS_BEEN_ACTIVE           4
#define   DPIO2_BIT_NUM_FIFO_ALMOST_EMPTY_FLAG_HAS_BEEN_ACTIVE    3
#define   DPIO2_BIT_NUM_FIFO_HALF_FULL_FLAG_HAS_BEEN_ACTIVE       2
#define   DPIO2_BIT_NUM_FIFO_ALMOST_FULL_FLAG_HAS_BEEN_ACTIVE     1
#define   DPIO2_BIT_NUM_FIFO_FULL_FLAG_HAS_BEEN_ACTIVE            0


#define   DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_0   0x0010010c

#define DPIO2_BIT_NUM_SYNC_ASSERTED_INTERRUPT_STATUS      7
#define DPIO2_BIT_NUM_FIFO_OVERFLOW_INTERRUPT_STATUS      6
#define DPIO2_BIT_NUM_SUSPEND_ASSERTED_INTERRUPT_STATUS   5
#define DPIO2_BIT_NUM_PIO2_ASSERTED_INTERRUPT_STATUS      4
#define DPIO2_BIT_NUM_PIO1_ASSERTED_INTERRUPT_STATUS      3


#define   DPIO2_FRONT_END_INTERRUPT_STATUS_REGISTER_1   0x00100110

#define DPIO2_BIT_NUM_FIFO_NOT_EMPTY_INTERRUPT_STATUS      5
#define DPIO2_BIT_NUM_FIFO_EMPTY_INTERRUPT_STATUS          4
#define DPIO2_BIT_NUM_FIFO_ALMOST_EMPTY_INTERRUPT_STATUS   3
#define DPIO2_BIT_NUM_FIFO_HALF_FULL_INTERRUPT_STATUS      2
#define DPIO2_BIT_NUM_FIFO_ALMOST_FULL_INTERRUPT_STATUS    1
#define DPIO2_BIT_NUM_FIFO_FULL_INTERRUPT_STATUS           0


#define   DPIO2_FRONT_END_PERSONALITY_ID_REGISTER   0x00100114

#define DPIO2_MSB_NUM_MAIN_BOARD_ID     7
#define DPIO2_LSB_NUM_MAIN_BOARD_ID     4
#define DPIO2_MSB_NUM_PERSONALITY_ID    3
#define DPIO2_LSB_NUM_PERSONALITY_ID    0


#define DPIO2_FIFO_STATE_EMPTY_BITMASK          0x00000010
#define DPIO2_FIFO_STATE_ALMOST_EMPTY_BITMASK   0x00000008
#define DPIO2_FIFO_STATE_HALF_FULL_BITMASK      0x00000004
#define DPIO2_FIFO_STATE_ALMOST_FULL_BITMASK    0x00000002
#define DPIO2_FIFO_STATE_FULL_BITMASK           0x00000001


#define   DPIO2_FRONT_END_VERSION_REGISTER   0x00100118

#define   DPIO2_MSB_NUM_FRONT_END_FPGA_VERSION   7
#define   DPIO2_LSB_NUM_FRONT_END_FPGA_VERSION   0



#define   DPIO2_SRAM_BASE_ADDRESS   0x00200000

#define   DPIO2_NUM_DMA_DESCRIPTORS_IN_SRAM   0x00020000


#define   DPIO2_FLASH_BASE_ADDRESS   0x00400000


#endif /* DPIO2DEFS_H */

