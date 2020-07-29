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


Module      : vmosa-linux.h

Description : Header file declaring linux in vmosa

Copyright(c): 2004 VMETRO asa. All Rights Reserved.

Revision History:
  01a,06jul2004,meh  Created.

*****************************************************************************/

#ifndef VMOSA_LINUX_INCLUDED
#define VMOSA_LINUX_INCLUDED



#if 1
#ifdef __cplusplus
extern "C" {
#endif
#endif


#define OK      (0)
#define ERROR   (-1)

#define TRUE    (1)
#define FALSE   (0)

#ifndef NULL
#define   NULL  (0)
#endif


/* Typedefs
 */
#ifndef VM_KERNEL /*meh*/
#ifndef BOOL
typedef  int             BOOL;
#endif
#ifndef STATUS
typedef  int             STATUS;
#endif
#ifndef PWOID
typedef  void*           PWOID;
#endif

#ifndef UINT
typedef  unsigned int    UINT;
#endif
#ifndef UINT32
typedef  unsigned int    UINT32;
#endif
#ifndef DWORD
typedef  unsigned long   DWORD;
#endif
#ifndef UINT16
typedef  unsigned short  UINT16;
#endif
#ifndef UINT8
typedef  unsigned char   UINT8;
#endif
#endif


#ifdef __cplusplus 

typedef  int (*FUNCPTR)(...);

#else

typedef  int (*FUNCPTR)(void*);

#endif



#if 1
#ifdef __cplusplus
}
#endif
#endif



#endif /* VMOSA_LINUX_INCLUDED */


