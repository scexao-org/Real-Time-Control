/****************************************************************************
Module      : DpioCommon.h

Description : Header file declaring constants and macros that are
              common to both DPIO and DPIO2.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01f,02aug2004,meh  Added logMsg ("ERROR %s %d\n", __FILE__, __LINE__
                     to DPIO2_ERROR_MSG.
  01e,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01d,23jan2004,meh  include logLib.h only if VXWORKS.
  01c,01oct2003,meh  Added DPIO2_ERROR_MSG and include of
                     logLib.h
  01b,28oct2001,nib  Updated for a Windows environment.
  01a,22jun2001,nib  Created.

*****************************************************************************/
#ifndef DPIOCOMMON_H
#define DPIOCOMMON_H

#include <stdio.h>

#ifdef VXWORKS
#include "vxWorks.h"
#endif

#ifdef WIN32
#include <windows.h>
#include "WindowsTypes.h"
#endif

#ifdef MCOS
#include <mcos.h>
#include "McosTypes.h"
#endif

#ifdef VMOSA
#include "vmosa.h"
#endif




typedef enum
{

  DPIO_FIFO_EMPTY                  = 0x05,
  DPIO_FIFO_ALMOST_EMPTY           = 0x04,
  DPIO_FIFO_LESS_THAN_HALF_FULL    = 0x00,
  DPIO_FIFO_GREATER_THAN_HALF_FULL = 0x02,
  DPIO_FIFO_ALMOST_FULL            = 0x06,
  DPIO_FIFO_FULL                   = 0x07

} DPIO_FIFO_STATE;


enum
{

  PCI_INTERRUPT_ACKNOWLEDGE_COMMAND       = 0x0,
  PCI_SPECIAL_CYCLE_COMMAND               = 0x1,
  PCI_IO_READ_COMMAND                     = 0x2,
  PCI_IO_WRITE_COMMAND                    = 0x3,
  PCI_MEMORY_READ_COMMAND                 = 0x6,
  PCI_MEMORY_WRITE_COMMAND                = 0x7,
  PCI_CONFIGURATION_READCOMMAND           = 0xa,
  PCI_CONFIGURATION_WRITE_COMMAND         = 0xb,
  PCI_MEMORY_READ_MULTIPLE_COMMAND        = 0xc,
  PCI_DUAL_ADDRESS_CYCLE_COMMAND          = 0xd,
  PCI_MEMORY_READ_LINE_COMMAND            = 0xe,
  PCI_MEMORY_WRITE_AND_INVALIDATE_COMMAND = 0xf

};




/* Define __PROTOTYPE_5_0 before including logLib.
   This prevents that the 'old' prototype for logMsg is included,
   so we do not need to send all the parameters to logMsg() in order
   to relax the compiler */
#define __PROTOTYPE_5_0

#ifdef VXWORKS
#include "logLib.h"
#else
#define logMsg printf
#endif


#ifdef __cplusplus
extern "C" {
#endif

  extern BOOL dpio2ErrorMessageEnable;
  extern BOOL dpio2WarningMessageEnable;

#ifdef __cplusplus
}
#endif


#define DPIO2_ERROR_MSG(args) {\
  if (dpio2ErrorMessageEnable) {\
     logMsg ("ERROR %s %d\n", __FILE__, __LINE__); \
     logMsg args;\
  }\
}
#define DPIO2_WARNING_MSG(args, flag)\
  if (dpio2WarningMessageEnable)\
    if (flag)\
      logMsg args;
#define DEBUG_MSG(paramList, flag) if (flag) printf paramList
#define ERROR_MSG(paramList) printf paramList



#ifndef VMOSA

inline UINT32 convertToLittleEndian(UINT32 value) {

#if (CPU == I960JX)

  return (value);

#elif ((CPU == PPC603) || (CPU == PPC604) || (CPU == PPC440))

  return ( ((value & 0xFF000000) >> 24) 
           | ((value & 0x00FF0000) >> 8) 
           | ((value & 0x0000FF00) << 8) 
           | ((value & 0x000000FF) << 24) );

#elif (CPU == I386)

  return (value);

#else 

#error "UINT32 convertToLittleEndian(UINT32 value) is not implemented"

#endif

};


inline UINT32 convertFromLittleEndian(UINT32 value) {

#if (CPU == I960JX)

  return (value);

#elif ((CPU == PPC603) || (CPU == PPC604) || (CPU == PPC440))

  return ( ((value & 0xFF000000) >> 24) 
           | ((value & 0x00FF0000) >> 8) 
           | ((value & 0x0000FF00) << 8) 
           | ((value & 0x000000FF) << 24) );

#elif (CPU == I386)

  return (value);

#else

#error "UINT32 convertFromLittleEndian(UINT32 value) is not implemented"

#endif

};

#endif /* !VMOSA */



#endif  /* DPIOCOMMON_H */

