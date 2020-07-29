/************************************************************************/
/*                                                                      */
/*  AMD Flash Memory Drivers                                            */
/*  File name: FLASH.H                                                  */
/*  Revision:  1.1  5/07/98                                             */
/*                                                                      */
/* Copyright (c) 1998 ADVANCED MICRO DEVICES, INC. All Rights Reserved. */
/* This software is unpublished and contains the trade secrets and      */
/* confidential proprietary information of AMD. Unless otherwise        */
/* provided in the Software Agreement associated herewith, it is        */
/* licensed in confidence "AS IS" and is not to be reproduced in whole  */
/* or part by any means except for backup. Use, duplication, or         */
/* disclosure by the Government is subject to the restrictions in       */
/* paragraph (b) (3) (B) of the Rights in Technical Data and Computer   */
/* Software clause in DFAR 52.227-7013 (a) (Oct 1988).                  */
/* Software owned by                                                    */
/* Advanced Micro Devices, Inc.,                                        */
/* One AMD Place,                                                       */
/* P.O. Box 3453                                                        */
/* Sunnyvale, CA 94088-3453.                                            */
/************************************************************************/
/*  This software constitutes a basic shell of source code for          */
/*  programming all AMD Flash components. AMD                           */
/*  will not be responsible for misuse or illegal use of this           */
/*  software for devices not supported herein. AMD is providing         */
/*  this source code "AS IS" and will not be responsible for            */
/*  issues arising from incorrect user implementation of the            */
/*  source code herein. It is the user's responsibility to              */
/*  properly design-in this source code.                                */
/*                                                                      */ 
/************************************************************************/
#ifndef _FLASH_H
#define _FLASH_H

#define MAXSECTORS  35      /* maximum number of sectors supported */

/* A structure for identifying a flash part.  There is one for each
 * of the flash part definitions.  We need to keep track of the
 * sector organization, the address register used, and the size
 * of the sectors.
 */
struct flashinfo {

  char *name;         /* "Am29DL800T", etc. */
  unsigned long addr; /* physical address, once translated */
  int areg;           /* Can be set to zero for all parts */
  int nsect;          /* # of sectors -- 19 in LV, 22 in DL */
  int bank1start;     /* first sector # in bank 1 */
  int bank2start;     /* first sector # in bank 2, if DL part */

  struct {

    long size;           /* # of bytes in this sector */
    long base;           /* offset from beginning of device */
    int bank;            /* 1 or 2 for DL; 1 for LV */

  } sec[MAXSECTORS];  /* per-sector info */
};

/* Standard Boolean declarations */
#if 0
#define TRUE 				1
#define FALSE 				0
#endif /* 0 */

/* Command codes for the flash_command routine */
#define FLASH_SELECT    0       /* no command; just perform the mapping */
#define FLASH_RESET     1       /* reset to read mode */
#define FLASH_READ      1       /* reset to read mode, by any other name */
#define FLASH_AUTOSEL   2       /* autoselect (fake Vid on pin 9) */
#define FLASH_PROG      3       /* program a word */
#define FLASH_CERASE    4       /* chip erase */
#define FLASH_SERASE    5       /* sector erase */
#define FLASH_ESUSPEND  6       /* suspend sector erase */
#define FLASH_ERESUME   7       /* resume sector erase */
#define FLASH_UB        8       /* go into unlock bypass mode */
#define FLASH_UBPROG    9       /* program a word using unlock bypass */
#define FLASH_UBRESET   10      /* reset to read mode from unlock bypass mode */
#define FLASH_LASTCMD   10      /* used for parameter checking */

/* Return codes from flash_status */
#define FLASH_STATUS_READY    0       /* ready for action */
#define FLASH_STATUS_BUSY     1       /* operation in progress */
#define FLASH_STATUS_ERSUSP   2       /* erase suspended */
#define FLASH_STATUS_TIMEOUT  3       /* operation timed out */
#define FLASH_STATUS_ERROR    4       /* unclassified but unhappy status */

/* Typedefs which should be set by every compiler */
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

/* Used to mask of bytes from word data */
#define HIGH_BYTE(a) (a >> 8)
#define LOW_BYTE(a)  (a & 0xFF)

/* AMD's manufacturer ID */
#define AMDPART   	0x01

/* A list of 4 AMD device ID's - add others as needed */
#define ID_AM29DL800T   0x4A
#define ID_AM29DL800B   0xCB
#define ID_AM29LV800T   0xDA
#define ID_AM29LV800B   0x5B
#define ID_AM29LV160B   0x49
#define ID_AM29LV160T   0xC4
#define ID_AM29LV400B   0xBA

/* An index into the memdesc organization array.  Is set by init_flash */
/* so the proper sector tables are used in the correct device          */
/* Add others as needed, and as added to the device ID section         */

#define AM29DL800T 	0
#define AM29DL800B 	1
#define AM29LV800T 	2
#define AM29LV800B 	3
#define AM29LV160B 4
#define AM29LV400B 5

/* NOTE: some compilers are unhappy if function prototypes are not     */
/* provided before they are called.  They can be added in the rare     */
/* case they are needed below.                                         */
 

#endif
