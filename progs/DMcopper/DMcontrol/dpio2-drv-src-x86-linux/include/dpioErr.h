/*---------------------------------------------------------------------------
Copyright Notice:
  This computer software is proprietary to VMETRO. The use of this software
  is governed by a licensing agreement. VMETRO retains all rights under
  the copyright laws of the United States of America and other countries.
  This software may not be furnished or disclosed to any third party and
  may not be copied or reproduced by any means, electronic, mechanical, or
  otherwise, in whole or in part, without specific authorization in writing
  from VMETRO.
 
    Copyright (c) 1997, 1998 by VMETRO, Inc.  All Rights Reserved.
 
Description:

     Contains definition for DPIO errors.  See "USAGE" below.

$Id: dpioErr.h,v 1.1.1.1 2001/07/12 08:54:01 cvs Exp $

MODIFICATION HISTORY:
01a,21jan97 bqv written

$Log: dpioErr.h,v $
Revision 1.1.1.1  2001/07/12 08:54:01  cvs
Source from 1.0 Beta A

Revision 1.2  1998/02/19 22:58:41  jbaker
Changed to use standar VMETRO banner.

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
USAGE:
You must:
    Link or copy this file to $(WIND_BASE)/target/h
    Change directory to $(WIND_BASE)/target/src/usr
    Remove statTbl.c and call make
    Change to $(WIND_BASE)/target/config/midas and call make

Only err codes in the VxWorks system error table produce meaningful
output from printErrno. If you don't carry out the steps above, VxWorks
still functions correctly, but you will get, for example,

    ->printErrno
    Unknown errno:0x8001

instead of a helpful string to tell you what kind of error it is.

SYSTEM DEPENDENCIES:
    VxWorks
---------------------------------------------------------------------------*/

#ifndef _DPIO_ERR_H_
#define _DPIO_ERR_H_

#include "dpioErrMod.h"

/* define err codes */

    /* DMA error codes */
#define S_dpio_DMA_ERR_MASTER_ABORT                         (M_dpio | 1)
#define S_dpio_DPIO_DMA_ERR_TARGET_ABORT_TARGET             (M_dpio | 2)
#define S_dpio_DPIO_DMA_ERR_TARGET_ABORT_MASTER             (M_dpio | 3)
#define S_dpio_DPIO_DMA_ERR_MASTER_PARITY_ERROR             (M_dpio | 4)
#define S_dpio_DPIO_DMA_ERR_SERR                            (M_dpio | 5)
#define S_dpio_DPIO_DMA_ERR_CTRL_NOT_RUNNING                (M_dpio | 6)
#define S_dpio_DPIO_DMA_ERR_CTRL_DONE                       (M_dpio | 7)
#define S_dpio_DPIO_DMA_ERR_INVALID_ARG                     (M_dpio | 8)
#define S_dpio_DPIO_DMA_ERR_INVALID_DESC_ID                 (M_dpio | 9)

    /* PCI error codes */
#define S_dpio_DPIO_PCI_ERR_BUSERR_WR_MODE                  (M_dpio | 10)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_MODE                  (M_dpio | 11)
#define S_dpio_DPIO_PCI_ERR_BUSERR_WR_PIOCTR                (M_dpio | 12)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_PIOCTR                (M_dpio | 13)
#define S_dpio_DPIO_PCI_ERR_BUSERR_WR_INTCTR                (M_dpio | 14)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_INTCTR                (M_dpio | 15)
#define S_dpio_DPIO_PCI_ERR_BUSERR_WR_DMACTR                (M_dpio | 16)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_DMACTR                (M_dpio | 17)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_FIFO_STAT             (M_dpio | 18)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_PIO_STAT              (M_dpio | 19)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_INT_STAT              (M_dpio | 20)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RD_MOD_ID                (M_dpio | 21)
#define S_dpio_DPIO_PCI_ERR_BUSERR_MODE_REG                 (M_dpio | 22)
#define S_dpio_DPIO_PCI_ERR_BUSERR_RESET_OCCURRED           (M_dpio | 23)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_CTRL_RESUME          (M_dpio | 24)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_CTRL_ABORT           (M_dpio | 25)
#define S_dpio_DPIO_PCI_ERR_BUSERR_960BUS_RESET             (M_dpio | 26)
#define S_dpio_DPIO_PCI_ERR_BUSERR_960BUS_DESC              (M_dpio | 27)
#define S_dpio_DPIO_PCI_ERR_BUSERR_SPACE_0_ENABLE           (M_dpio | 28)
#define S_dpio_DPIO_PCI_ERR_BUSERR_VENDOR_ID_RD             (M_dpio | 29)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DEVICE_ID_RD             (M_dpio | 30)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DEVICE_TYPE_RD           (M_dpio | 31)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_MODE_REG_SET         (M_dpio | 32)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_NXT_DESC_REG_SET     (M_dpio | 33)
#define S_dpio_DPIO_PCI_ERR_BUSERR_CFG_CMD_MASTER_ENABLE    (M_dpio | 34)
#define S_dpio_DPIO_PCI_ERR_BUSERR_INTR_ENABLE              (M_dpio | 35)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_CTRL_START           (M_dpio | 36)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_CTRL_SUSPEND         (M_dpio | 37)
#define S_dpio_DPIO_PCI_ERR_BUSERR_PCI9060_STAT_RD          (M_dpio | 38)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_CMD_STAT_RD          (M_dpio | 39)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_DESC_PTR_RD          (M_dpio | 40)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_INTR_ENABLE          (M_dpio | 41)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DMA_INTR_DISABLE         (M_dpio | 42)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DPIO_MEM_WR              (M_dpio | 43)
#define S_dpio_DPIO_PCI_ERR_BUSERR_DPIO_MEM_RD              (M_dpio | 44)
#define S_dpio_DPIO_PCI_ERR_BUSERR_PCI9060_WR               (M_dpio | 45)
#define S_dpio_DPIO_PCI_ERR_BUSERR_INTR_SRC_0_DETECT        (M_dpio | 46)
#define S_dpio_DPIO_PCI_ERR_BUSERR_INTR_SRC_1_DETECT        (M_dpio | 47)
#define S_dpio_DPIO_PCI_ERR_BUSERR_INTR_SRC_2_DETECT        (M_dpio | 48)
#define S_dpio_DPIO_PCI_ERR_BUSERR_INTR_SRC_3_DETECT        (M_dpio | 49)
#define S_dpio_DPIO_PCI_ERR_BUSERR_960BUS_RANGE             (M_dpio | 50)
#define S_dpio_DPIO_PCI_ERR_INVALID_VENDOR_ID               (M_dpio | 51)
#define S_dpio_DPIO_PCI_ERR_INVALID_DEVICE_ID               (M_dpio | 52)
#define S_dpio_DPIO_PCI_ERR_INVALID_DEVICE_TYPE             (M_dpio | 53)
#define S_dpio_DPIO_PCI_ERR_DMA_CTRL_START                  (M_dpio | 54)
#define S_dpio_DPIO_PCI_ERR_MODE_REG                        (M_dpio | 55)
#define S_dpio_DPIO_PCI_ERR_INVALID_ID                      (M_dpio | 56)
/* Note the jump in number to be compatible with old versions */
#define S_dpio_DPIO_PCI_ERR_BUSSERR_LATENCY_TIMER_GET	    (M_dpio | 62)
#define S_dpio_DPIO_PCI_ERR_BUSSERR_LATENCY_TIMER_SET	    (M_dpio | 63)

    /* miscellaneous error codes */
#define S_dpio_DPIO_MISC_ERR_INVALID_ARG                    (M_dpio | 57)
#define S_dpio_DPIO_MISC_ERR_DRV_NOT_INSTALLED              (M_dpio | 58)
#define S_dpio_DPIO_MISC_ERR_DEV_NAME_IN_USE                (M_dpio | 59)
#define S_dpio_DPIO_MISC_ERR_DEV_LOCKED                     (M_dpio | 60)
#define S_dpio_DPIO_MISC_ERR_INVALID_IOCTL_CMD              (M_dpio | 61)


#endif
        /* Do not add anything after this line! */
