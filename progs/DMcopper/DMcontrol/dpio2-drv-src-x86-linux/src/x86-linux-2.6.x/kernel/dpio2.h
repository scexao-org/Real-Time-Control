/*---------------------------------------------------------------------------
Copyright Notice:
  This computer software is proprietary to VMETRO. The use of this software
  is governed by a licensing agreement. VMETRO retains all rights under
  the copyright laws of the United States of America and other countries.
  This software may not be furnished or disclosed to any third party and
  may not be copied or reproduced by any means, electronic, mechanical, or
  otherwise, in whole or in part, without specific authorization in writing
  from VMETRO.
 
    Copyright (c) 1996-2001 by VMETRO, ASA.  All Rights Reserved.
 
Description:

     Header file for dpio2 driver.

MODIFICATION HISTORY:
01l,14otk2004,meh  Copied this defines from Dpio2Defs.h: PCI_VENDOR_ID_VMETRO,
                   PCI_DEVICE_ID_DPIO2 and PCI_DEVICE_ID_DPIO2_66MHZ
01k,24sep2002,nib  Added command to specify start value for counter pattern
                   in test pattern generator - DPIO2_CMD_TEST_PATTERN_START_VALUE_SET.
01j,17sep2002,nib  Added the commands DPIO2_CMD_EOT_COUNT_ENABLE 
                   and DPIO2_CMD_EOT_COUNT_DISABLE
01i,13sep2002,nib  Added the command DPIO2_CMD_TRANSFERRED_BYTE_COUNT_GET.
01h,12feb2002,nib  Added the command DPIO2_CMD_SYNC_GENERATION_COUNTER_SET.
01g,14nov2001,nib  Updated file to compile in a Windows environment.
01f,26sep2001,nib  Added constants for arguments to DPIO2_CMD_DATA_SWAP_MODE_SELECT
                   Added the commands DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT,
                   DPIO2_CMD_DATA_PACKING_ENABLE, DPIO2_CMD_DATA_PACKING_DISABLE,
                   DPIO2_CMD_DATA_PACKING_PIPELINE_CHECK, and
                   DPIO2_CMD_DATA_PACKING_PIPELINE_FLUSH.
                   Added constants for arguments to DPIO2_CMD_DATA_PACKING_ENABLE
01e,17sep2001,nib  Removed DPIO2_CMD_STROBE_GENERATION_SELECT and 
                   DPIO2_CMD_FIXED_OSCILLATOR_SELECT.
                   Added DPIO2_CMD_STROBE_GENERATION_ENABLE and
                   DPIO2_CMD_STROBE_RECEPTION_ENABLE.
                   Added the constants DPIO2_FIXED_OSCILLATOR,
                   DPIO2_PROGRAMMABLE_OSCILLATOR, DPIO2_TTL_STROBE, and DPIO2_PECL_STROBE.
01d,08aug2001,nib  Added ID for command for flushing the FIFO; DPIO2_CMD_FIFO_FLUSH.
                   Added ID for command for configuring SUSPEND data flow control;
                   DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT
                   Added ID for command for configuring NRDY data flow control;
                   DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT.
01c,20jul2001,nib  Added ID for command for data swap; DPIO2_CMD_DATA_SWAP_MODE_SELECT
01a,07may2001,nib  Created based on 'dpio.h' from the DPIO device driver

---------------------------------------------------------------------------*/

#ifndef _DPIO2_H
#define _DPIO2_H


#ifdef MCOS
#include <mcos.h>
#include "McosTypes.h"
#endif


#ifdef WIN32
#define DllExport	__declspec( dllexport )
#endif /* WIN32 */


#ifdef VMOSA
#include "vmosa.h"
#include <semaphore.h>                                                                     //rsr
#endif /* VMOSA */


#ifdef __cplusplus
extern "C" {
#endif


#ifdef MCOS
typedef   unsigned int   DPIO2_ID;
#endif

/**********************************/
/********** defines ***************/
/**********************************/

#define   DPIO_OUTPUT_DEFAULT_MODE_REGISTER_VALUE                 (0x00)
#define   DPIO_OUTPUT_DEFAULT_PIO_CONTROL_REGISTER_VALUE          (0x00)
#define   DPIO_OUTPUT_DEFAULT_INTERRUPT_CONTROL_REGISTER_VALUE    (0x00)
#define   DPIO_OUTPUT_DEFAULT_DMA_CONTROL_REGISTER_VALUE          (0x00)
#define   DPIO_INPUT_DEFAULT_MODE_REGISTER_VALUE                  (0x00)
#define   DPIO_INPUT_DEFAULT_PIO_CONTROL_REGISTER_VALUE           (0x00)
#define   DPIO_INPUT_DEFAULT_INTERRUPT_CONTROL_REGISTER_VALUE     (0x00)
#define   DPIO_INPUT_DEFAULT_DMA_CONTROL_REGISTER_VALUE           (0x20)


#define DPIO2_FB_MODULE       0x20
#define DPIO2_LB_MODULE       0x26
#define DPIO2_EI_MODULE       0x22
#define DPIO2_EO_MODULE       0x23
#define DPIO2_DI_MODULE       0x24
#define DPIO2_DO_MODULE       0x25


/* Definition of control bits and register on DPIO lacking in 'dpio.h'
 */
#define ENFIFO        0x40


/* Values returned by the ioctl command DPIO2_CMD_GET_CURRENT_FIFO_STATUS.
 */
#define DPIO2_FIFO_EMPTY                    (0x05)
#define DPIO2_FIFO_ALMOST_EMPTY             (0x04)
#define DPIO2_FIFO_LESS_THAN_HALF_FULL      (0x00)
#define DPIO2_FIFO_GREATER_THAN_HALF_FULL   (0x02)
#define DPIO2_FIFO_ALMOST_FULL              (0x06)
#define DPIO2_FIFO_FULL                     (0x07)


/* Values used by the ioctl command DPIO2_CMD_SYNC_GENERATION_SELECT.
 */
#define DPIO2_SYNC_GENERATION_DISABLED            (0x0)
#define DPIO2_SYNC_GENERATION_BEFORE_DATA         (0x1)
#define DPIO2_SYNC_GENERATION_AT_END_OF_FRAME     (0x2)
#define DPIO2_SYNC_GENERATION_AT_START_OF_FRAME   (0x3)
#define DPIO2_SYNC_GENERATION_ON_ODD_FRAME        (0x4)
#define DPIO2_SYNC_GENERATION_ON_COUNT            (0x5)


/* Values used by the ioctl command DPIO2_CMD_EOT_ENABLE.
 */
#define DPIO2_SYNC_MARKS_END_OF_TRANSFER   (2)
#define DPIO2_PIO1_MARKS_END_OF_TRANSFER   (3)
#define DPIO2_PIO2_MARKS_END_OF_TRANSFER   (4)
#define DPIO2_RES1_MARKS_END_OF_TRANSFER   (5)


/* Values used by the ioctl command DPIO2_CMD_SYNC_RECEPTION_SELECT.
 */
#define DPIO2_SYNC_RECEPTION_DISABLED                (0x0)
#define DPIO2_SYNC_RECEPTION_STARTS_DATA_RECEPTION   (0x1)


/* Values used by the ioctl command DPIO2_CMD_STROBE_FREQUENCY_RANGE_SET.
 */
#define DPIO2_STROBE_FREQUENCY_BELOW_OR_EQUAL_TO_5MHZ                 (0)
#define DPIO2_STROBE_FREQUENCY_ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ     (1)
#define DPIO2_STROBE_FREQUENCY_ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ    (2)
#define DPIO2_STROBE_FREQUENCY_ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ    (3)
#define DPIO2_STROBE_FREQUENCY_ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ   (4)
#define DPIO2_STROBE_FREQUENCY_ABOVE_100MHZ                           (5)
  /*meh, new 5MHZ range, used from _66 eco B0 and newer */
#define DPIO2_STROBE_FREQUENCY_ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ    (6)
#define DPIO2_STROBE_FREQUENCY_ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ    (7)
#define DPIO2_STROBE_FREQUENCY_ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ    (8)
#define DPIO2_STROBE_FREQUENCY_ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ    (9)
#define DPIO2_STROBE_FREQUENCY_ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ    (10)
#define DPIO2_STROBE_FREQUENCY_ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ    (11)
#define DPIO2_STROBE_FREQUENCY_ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ    (12)
#define DPIO2_STROBE_FREQUENCY_ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ    (13)
#define DPIO2_STROBE_FREQUENCY_ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ    (14)
#define DPIO2_STROBE_FREQUENCY_ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ    (15)
#define DPIO2_STROBE_FREQUENCY_ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ    (16)
#define DPIO2_STROBE_FREQUENCY_ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ    (17)
#define DPIO2_STROBE_FREQUENCY_ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ    (18)
#define DPIO2_STROBE_FREQUENCY_ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ    (19)
#define DPIO2_STROBE_FREQUENCY_ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ    (20)
#define DPIO2_STROBE_FREQUENCY_ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ    (21)
#define DPIO2_STROBE_FREQUENCY_ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ    (22)
#define DPIO2_STROBE_FREQUENCY_ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ   (23)





/* Values used by the ioctl command DPIO2_CMD_DATA_SWAP_MODE_SELECT. 
 */
#define DPIO2_NO_SWAP                (0)
#define DPIO2_8BIT_SWAP              (1)
#define DPIO2_16BIT_SWAP             (2)
#define DPIO2_8BIT_16BIT_SWAP        (3)
#define DPIO2_32BIT_SWAP             (4)
#define DPIO2_8BIT_32BIT_SWAP        (5)
#define DPIO2_16BIT_32BIT_SWAP       (6)
#define DPIO2_8BIT_16BIT_32BIT_SWAP  (7)


/* Values returned by the ioctl command DPIO2_CMD_DATA_PACKING_CAPABILITY_GET. 
 */
#define DPIO2_8BIT_4BIT_PACKING_AVAILABLE    (0)
#define DPIO2_16BIT_10BIT_PACKING_AVAILABLE  (1)


/* Values used by the ioctl command DPIO2_CMD_DATA_PACKING_ENABLE. 
 */
#define DPIO2_PACK_16_LSB_ON_FPDP   (0)
#define DPIO2_PACK_16_MSB_ON_FPDP   (1)
#define DPIO2_PACK_4_LSB_ON_FPDP    (2)
#define DPIO2_PACK_8_LSB_ON_FPDP    (3)
#define DPIO2_PACK_10_LSB_ON_FPDP   (4)


/* Values used by the ioctl command DPIO2_CMD_STROBE_GENERATION_ENABLE.
 */
#define DPIO2_FIXED_OSCILLATOR          (1)
#define DPIO2_PROGRAMMABLE_OSCILLATOR   (2)


/* Values used by the ioctl command DPIO2_CMD_STROBE_RECEPTION_ENABLE.
 */
#define DPIO2_PRIMARY_STROBE    (1)
#define DPIO2_SECONDARY_STROBE  (2)
#define DPIO2_TTL_STROBE        DPIO2_PRIMARY_STROBE
#define DPIO2_PECL_STROBE       DPIO2_SECONDARY_STROBE


/* Values used by the ioctl command DPIO2_CMD_TEST_PATTERN_GENERATION_ENABLE.
 */

#define DPIO2_WALKING_ONE_PATTERN                       (0)
#define DPIO2_WALKING_ZERO_PATTERN                      (1)
#define DPIO2_COUNTER_PATTERN                           (2)
#define DPIO2_COUNTER_PATTERN_WITH_PROGRAMMABLE_START   (3)



/* DPIO2 ioctl commands
 */
#define DPIO2_CMD_REG_SET_MODE                 1
#define DPIO2_CMD_REG_GET_MODE                 2
#define DPIO2_CMD_REG_SET_PIO_CTR              3
#define DPIO2_CMD_REG_GET_PIO_CTR              4
#define DPIO2_CMD_REG_SET_INT_CTR              5
#define DPIO2_CMD_REG_GET_INT_CTR              6
#define DPIO2_CMD_REG_SET_DMA_CTR              7
#define DPIO2_CMD_REG_GET_DMA_CTR              8
#define DPIO2_CMD_REG_GET_FIFO_STAT            9
#define DPIO2_CMD_REG_GET_PIO_STAT             10
#define DPIO2_CMD_REG_GET_INT_STAT             11
#define DPIO2_CMD_REG_GET_MOD_ID               12   
#define DPIO2_CMD_REG_SET_DEMAND_MD            13     /* Enable Demand Mode DMA */
#define DPIO2_CMD_REG_GET_DEMAND_MD            14     /* Get Demand Mode DMA status */
#define DPIO2_CMD_REG_CLR_DEMAND_MD            15     /* Disable Demand Mode DMA */
#define DPIO2_CMD_LBUS_TIMEOUT_EN              16     /* Enable Local bus timeout */
#define DPIO2_CMD_LBUS_TIMEOUT_DIS             17     /* Disable Local bus timeout */
#define DPIO2_CMD_DO_NOT_USE                   18     /* Something is wrong with this number */
#define DPIO2_CMD_EOT_ENABLE                   19     /* En for 'End Of Transfer' */
#define DPIO2_CMD_EOT_DISABLE                  20     /* Dis for 'End Of Transfer' */
#define DPIO2_CMD_EOTB3_ENABLE                 21     /* En for 'End Of Transfer' eco B3*/
#define DPIO2_CMD_GET_CURRENT_FIFO_STATUS      22
#define DPIO2_CMD_FPDP_ACTIVATION_SELECT       24
#define DPIO2_CMD_SYNC_GENERATION_SELECT       25
#define DPIO2_CMD_SYNC_RECEPTION_SELECT        26
#define DPIO2_CMD_STROBE_FREQUENCY_SET         28
#define DPIO2_CMD_STROBE_FREQUENCY_RANGE_SET   29
#define DPIO2_CMD_VIDEO_MODE_SELECT            30
#define DPIO2_CMD_DATA_SWAP_MODE_SELECT        31
#define DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT  32
#define DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT     33
#define DPIO2_CMD_RES1_DIRECTION_SELECT        34
#define DPIO2_CMD_RES1_OUTPUT_VALUE_SET        35
#define DPIO2_CMD_RES1_VALUE_GET               36
#define DPIO2_CMD_RES2_DIRECTION_SELECT        37
#define DPIO2_CMD_RES2_OUTPUT_VALUE_SET        38
#define DPIO2_CMD_RES2_VALUE_GET               39
#define DPIO2_CMD_RES3_DIRECTION_SELECT        40
#define DPIO2_CMD_RES3_OUTPUT_VALUE_SET        41
#define DPIO2_CMD_RES3_VALUE_GET               42
#define DPIO2_CMD_PIO1_DIRECTION_SELECT        43
#define DPIO2_CMD_PIO1_OUTPUT_VALUE_SET        44
#define DPIO2_CMD_PIO1_VALUE_GET               45
#define DPIO2_CMD_PIO2_DIRECTION_SELECT        46
#define DPIO2_CMD_PIO2_OUTPUT_VALUE_SET        47
#define DPIO2_CMD_PIO2_VALUE_GET               48
#define DPIO2_CMD_STROBE_GENERATION_ENABLE     49
#define DPIO2_CMD_STROBE_RECEPTION_ENABLE      50

/* To avoid changing the values of the constants below,
 * new constants are given values starting at 10000.
 */
#define DPIO2_CMD_D0_TO_BE_USED_FOR_SYNC_SELECT          10000
#define DPIO2_CMD_DATA_PACKING_ENABLE                    10001
#define DPIO2_CMD_DATA_PACKING_DISABLE                   10002
#define DPIO2_CMD_DATA_PACKING_PIPELINE_CHECK            10003
#define DPIO2_CMD_DATA_PACKING_PIPELINE_FLUSH            10004
#define DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT   10005
#define DPIO2_CMD_DATA_PACKING_CAPABILITY_GET            10006
#define DPIO2_CMD_COUNTER_ADDRESSING_ENABLE              10007
#define DPIO2_CMD_COUNTER_ADDRESSING_DISABLE             10008
#define DPIO2_CMD_SYNC_GENERATION_COUNTER_SET            10009
#define DPIO2_CMD_TEST_PATTERN_GENERATION_ENABLE         10010
#define DPIO2_CMD_TEST_PATTERN_GENERATION_DISABLE        10011
#define DPIO2_CMD_SUSPEND_ASSERTION_FORCE                10012
#define DPIO2_CMD_STROBE_SKEW_SET                        10013
#define DPIO2_CMD_DEFAULT_STROBE_SKEW_SET                10014
#define DPIO2_CMD_EOT_COUNT_ENABLE                       10015
#define DPIO2_CMD_EOT_COUNT_DISABLE                      10016
#define DPIO2_CMD_TEST_PATTERN_START_VALUE_SET           10017


/* Control register commands */
#define DPIO2_CMD_RESET_OCCURRED_FLAGS                  51
#define DPIO2_CMD_GET_FIFO_OVERFLOW_OCCURRED_FLAG       52
#define DPIO2_CMD_GET_FIFO_FULL_OCCURRED_FLAG           53
#define DPIO2_CMD_GET_FIFO_ALMOST_FULL_OCCURRED_FLAG    54
#define DPIO2_CMD_GET_FIFO_HALF_FULL_OCCURRED_FLAG      55
#define DPIO2_CMD_GET_FIFO_ALMOST_EMPTY_OCCURRED_FLAG   56
#define DPIO2_CMD_GET_FIFO_EMPTY_OCCURRED_FLAG          57
#define DPIO2_CMD_FIFO_FLUSH                            58

/* DMA commands */
#define DPIO2_CMD_DMA_SET_DESC          201
#define DPIO2_CMD_DMA_START             202
#define DPIO2_CMD_DMA_SUSPEND           203
#define DPIO2_CMD_DMA_RESUME            204
#define DPIO2_CMD_DMA_ABORT             205
#define DPIO2_CMD_DMA_GET_STATUS        206
#define DPIO2_CMD_DMA_INT_ENABLE        207    /* Enable DMA interrupt */
#define DPIO2_CMD_DMA_INT_DISABLE       208    /* Disable DMA interrupt */
#define DPIO2_CMD_DMA_HALTP_EN          209    /* En. proc. halt during DMA */
#define DPIO2_CMD_DMA_HALTP_DIS         210    /* Dis. proc. halt during DMA */
#define DPIO2_CMD_DMA_GET_DONE          211    /* Get DMA done status */
#define DPIO2_CMD_DMA_BSWAP_SET         212    /* En/Dis-able DMA Ch1 byte swap */
#define DPIO2_CMD_DMA_BSWAP_GET         213    /* Report DMA Ch1 byte swap */
#define DPIO2_CMD_DMA_SET_START_DESCRIPTOR    214
#define DPIO2_CMD_REMAINING_BYTE_COUNT_GET    215
#define DPIO2_CMD_FLUSH_ON_DMA_ABORT_SELECT   216
#define DPIO2_CMD_CONTINUE_ON_EOT_SELECT      217
#define DPIO2_CMD_TRANSFERRED_BYTE_COUNT_GET  218


/* ISR interrupt condition-action specification command */
#define DPIO2_CMD_SET_INT_ACTION        304
#define DPIO2_CMD_INTERRUPT_ENABLE      305
#define DPIO2_CMD_INTERRUPT_DISABLE     306
#define DPIO2_CMD_INTERRUPT_CALLBACK_ATTACH   307
#define DPIO2_CMD_INTERRUPT_CALLBACK_DETACH   308
#define DPIO2_CMD_AUTO_DISABLE_INTERRUPT_ENABLE   309
#define DPIO2_CMD_AUTO_DISABLE_INTERRUPT_DISABLE   310

/* DPIO2 Access commands */
#define DPIO2_CMD_LMEM_UINT32_WRITE    401 /* 32bit writeaccess to DPIO memory */
#define DPIO2_CMD_LMEM_UINT32_READ     402 /* 32bit readaccess to DPIO memory  */
#define DPIO2_CMD_PLX_UINT32_WRITE     403 /* 32bit writeaccess to PCI9060 chip*/
#define DPIO2_CMD_PLX_UINT32_READ      404 /* 32bit readaccess to PCI9060 chip*/

/* Special commands */
#define DPIO2_CMD_DUMP                  501
#define DPIO2_CMD_REINIT                502
#define DPIO2_CMD_DUMP_DEV              503


/* DPIO device structure commands */
#define DPIO2_CMD_GET_DEVICE_TYPE       1001   /* Get DPIO type */
#define DPIO2_CMD_GET_DEVICE_ECO        1002   /* Get DPIO ECO */
#define DPIO2_CMD_GET_DEVICE_FSIZE      1003   /* Get DPIO FIFO size */
#define DPIO2_CMD_GET_DEVICE_BUSNUM     1004   /* Get PCI bus nr. of DPIO */
#define DPIO2_CMD_GET_DEVICE_DEVNUM     1005   /* Get PCI device nr. of DPIO */


/* Get pointers to register structures for direct register access */
#define DPIO2_CMD_GET_PTR_CTRL_REGS         1011
#define DPIO2_CMD_GET_PTR_STAT_REGS         1012
#define DPIO2_CMD_GET_PTR_PCI9060_REGS      1013
#define DPIO2_CMD_GET_PTR_FIFO              1014
#define DPIO2_CMD_GET_PTR_SRAM              1015
#define DPIO2_CMD_GET_PTR_RUNTIME_BASE      1016


/* Config space commands */
#define DPIO2_CMD_LATENCY_TIMER_GET		1201
#define DPIO2_CMD_LATENCY_TIMER_SET		1202


/* Reserved commands.
 */
#define DPIO2_CMD_RESERVED_1   5000
#define DPIO2_CMD_RESERVED_2   5001


/* Interrupt condition. 
 */
#define DPIO2_INT_COND_DMA_DONE_CHAIN      0
#define DPIO2_INT_COND_DMA_DONE_BLOCK      1
#define DPIO2_INT_COND_FIFO_NOT_EMPTY      2
#define DPIO2_INT_COND_FIFO_EMPTY          3
#define DPIO2_INT_COND_FIFO_ALMOST_EMPTY   4
#define DPIO2_INT_COND_FIFO_HALF_FULL      5
#define DPIO2_INT_COND_FIFO_ALMOST_FULL    6
#define DPIO2_INT_COND_FIFO_FULL           7
#define DPIO2_INT_COND_FIFO_OVERFLOW       8
#define DPIO2_INT_COND_PIO1                9
#define DPIO2_INT_COND_PIO2                10
#define DPIO2_INT_COND_SYNC                11
#define DPIO2_INT_COND_SUSPEND             12
#define DPIO2_INT_COND_TARGET_ABORT        13
#define DPIO2_INT_COND_MASTER_ABORT        14
#define DPIO2_INT_CONDITIONS               15


#ifdef VMOSA
#define   PCI_VENDOR_ID_VMETRO       0x129A
#define   PCI_DEVICE_ID_DPIO2        0xDD11
#define   PCI_DEVICE_ID_DPIO2_66MHZ  0xDD12
#endif /*VMOSA*/





#ifdef WIN32

#define   MAX_NUM_DPIO2_MODULES   (8)

#define   DPIO2_INPUT    (1)
#define   DPIO2_OUTPUT   (2)

#define   DPIO2_DMA_PAGES   (256)

#endif /* WIN32 */


#ifdef MCOS

#define   DPIO2_INPUT    (1)
#define   DPIO2_OUTPUT   (2)

#endif /* MCOS */


#ifdef VMOSA

#define   DPIO2_INPUT                          (1)
#define   DPIO2_OUTPUT                         (2)

#define   MAX_DPIO2S   8

#endif /* VMOSA */



/* DPIO2 DMA descriptor - used with DPIO2_CMD_SET_DMA_DESC 
 */
typedef struct DPIO2_DMA_DESCRIPTOR
{

  UINT32 descriptorId;          /* Descriptor Identifier.
                                 */

  UINT32 nextDescriptorId;      /* Identifier of the next descriptor 
                                 * in a DMA chain.
                                 */

  UINT32 pciAddress;            /* Base address of buffer in PCI
                                 * Memory space, to transfer data to or
                                 * from.  The direction depends on how
                                 * the DPIO2 module is configured.
                                 */

  UINT32 blockSizeInBytes;      /* Size of DMA block in bytes 
                                 */

  BOOL   lastBlockInChain;      /* Set this flag if this descriptor
                                 * is the last in a DMA chain.
                                 */

  BOOL   endOfBlockInterrupt;   /* Set this bit to generate interrupt 
                                 * when the last word represented
                                 * by this descriptor is transferred.
                                 */

  BOOL   useD64;                /* Set this flag to instruct the
                                 * DPIO2 module to use D64 accesses
                                 * when transferring the data represented
                                 * by this descriptor.
				 */

  BOOL   notEndOfFrame;         /* The data of this descriptor is
                                 * not the last part of a frame if
                                 * this flag is set.*/

} DPIO2_DMA_DESC;


/* Struncture for attaching a callback function 
 * - used with DPIO2_CMD_ATTACH_INTERRUPT_CALLBACK.
 */
typedef struct DPIO2_INTERRUPT_CALLBACK
{

  int      condition;           /* Value identifying the interrupt
				 * condition where upon the callback
				 * function shall be called.
				 */

  FUNCPTR  pCallbackFunction;   /* Pointer to the callback function.
				 */

  sem_t*   argument;            /* Argument to be passed along
				 * the callback function.
				 */

} DPIO2_INTERRUPT_CALLBACK;


/* Structure for enabling Counter Addressing on modules
 * configures as input (DPIO2_CMD_COUNTER_ADDRESSING_ENABLE).
 */
typedef struct DPIO2_COUNTER_ADDRESSING_INFO
{

  UINT32   initialSkipCount;
  UINT32   skipCount;
  UINT32   receiveCount;

} DPIO2_COUNTER_ADDRESSING_INFO;


/* Structure for information about remaing bytes associated
 *  with a DMA descriptor (DPIO2_CMD_REMAINING_BYTE_COUNT_GET).
 */
typedef struct {

  UINT32   byteCount;
  BOOL     overflowFlag;

} DPIO2_REMAINING_BYTE_COUNT_INFO;

/*
#ifdef WIN32

typedef struct DPIO2_INFO {

  int    devno;
  UINT32 slot;
  UINT8  type;

} DPIO2_INFO;


typedef struct
{

  PVOID pPhysicalAddr;    // physical address of page
  DWORD dwBytes;          // size of page

} DPIO2_DMA_PAGE;


typedef struct 
{

  DWORD     hDma;             // handle of dma buffer
  PVOID     pUserAddr;        // begining of buffer
  DWORD     dwBytes;          // size of buffer
  DWORD     dwOptions;        // allocation options: mast be 0
  DWORD     dwPages;          // number of pages in buffer
  DPIO2_DMA_PAGE Page[DPIO2_DMA_PAGES];

} DPIO2_DMA;

#endif
*/


#ifdef VMOSA


typedef struct 
{

  device*      hDma;             /* handle of dma buffer */
  void*        pUserAddr;        /* begining of buffer */
  size_t       dwBytes;          /* size of buffer */
  unsigned int dmaBusAddress;    /* we have to define this as unsigned int, since
                                    dma_addr_t isn't defined outside the kernel */
} DPIO2_DMA;


typedef struct DPIO2_INFO {

  int     devno;
  UINT16  deviceId;
  UINT16  vendorId;
  UINT8   revisionId;
  UINT8   moduleType;
  UINT8   pciBusNumber;
  UINT8   pciDeviceNumber;
  UINT8   pciFpgaVersion;
  UINT8   frontEndFpgaVersion;

} DPIO2_INFO;


typedef struct DPIO2_INFO_T {

  DPIO2_INFO  *device[MAX_DPIO2S];

} DPIO2_INFO_T;


#endif /* VMOSA */




/* Function declarations 
 */
#ifdef __GNUC__

#if defined(__STDC__) || defined(__cplusplus)

BOOL dpio2DetectDevice(int pmcSlotNumber);
STATUS dpio2Drv(BOOL useDpioApi, FUNCPTR pProcessorHaltHook);
STATUS dpio2DevCreate(char *name, int pmcSlotNumber);
int dpio2GetDeviceNames(char *nameTab[], int maxNames);

#else

BOOL dpio2DetectDevice();
STATUS dpio2Drv();
STATUS dpio2DevCreate();
int dpio2GetDeviceNames();

#endif

#endif /* __GNUC__ */


#ifdef WIN32

DllExport STATUS dpio2Open(int *nocards, DPIO2_INFO *info);
DllExport STATUS dpio2Init(int devno, int mode);
DllExport STATUS dpio2Ioctl (int devno, int command, int argument);
DllExport void dpio2Close(void);
DllExport STATUS dpio2DMALock (DPIO2_DMA *pDma);
DllExport void dpio2DMAUnlock (DPIO2_DMA *pDma);

#endif /* WIN32 */


#ifdef MCOS

DPIO2_ID dpio2Open(char* pMidasName,
                   const char* pMidasModelName,
                   int pmcSlotNumber,
                   int readWriteFlag,
                   unsigned long pollPeriodInMs);

STATUS dpio2Close(DPIO2_ID dpio2Id);

STATUS dpio2MapSmb(DPIO2_ID dpio2Id, SMB_handle hSmb, UINT32* pPciAddress);

STATUS dpio2UnmapSmb(DPIO2_ID dpio2Id, SMB_handle hSmb);

STATUS dpio2Ioctl (DPIO2_ID devno, int command, int argument);

#define DPIO2_CMD_USE_RACEWAY_PRIORITY_SET    40000
#define DPIO2_CMD_USE_RACEWAY_PRIORITY_GET    40001

#endif /* WIN32 */


#ifdef VMOSA
int    dpio2Scan  (DPIO2_INFO_T *pInfoArray);
STATUS dpio2Open  (int devno, int mode);
STATUS dpio2Close (int devno );

STATUS dpio2CopyToKernel (void* kernelAddress, void* userAddress, int iLength);

STATUS dpio2Ioctl ( int devno, int command, int argument );

STATUS dpio2DMALock ( DPIO2_DMA* pDma );
void   dpio2DMAUnlock ( DPIO2_DMA* pDma );

#  ifdef VMOSA_M5000
void* dpio2CacheDmaMalloc (unsigned int, DPIO2_DMA*);
void  dpio2CacheDmaFree   (DPIO2_DMA*);

STATUS dpio2CacheFlush      (DPIO2_DMA *pDma);
STATUS dpio2CacheInvalidate (DPIO2_DMA *pDma);

#  endif /*VMOSA_M5000*/
#endif /* VMOSA */



#ifdef __cplusplus
}
#endif

#endif  /* _DPIO2_H */
