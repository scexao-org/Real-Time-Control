/* SrecToBin.h - Library function that converts the contents of a SRECORD file
 *               to a binary image.
 */

/* ---------------------------------------------------------------------------
 * This computer software is proprietary to VMETRO. The use of this software
 * is governed by a licensing agreement. VMETRO retains all rights under
 * the copyright laws of the United States of America and other countries.
 * This software may not be furnished or disclosed to any third party and
 * may not be copied or reproduced by any means, electronic, mechanical, or
 * otherwise, in whole or in part, without specific authorization in writing
 * from VMETRO.
 *
 *      Copyright (c) 1996-2000 by VMETRO, Inc. 
 *      All Rights Reserved.
 *
 --------------------------------------------------------------------------- */
/* Copyright 1996-2000 VMETRO ASA */

/*
modification history
--------------------
01c,21mar2002,nib  Updated to compile in an MCOS environment.
01b,20nov2001,nib  Updated to compile in a Windows environment.
01a,25jul2000,nib  Written

*/

/*
DESCRIPTION
This header file defines the function prototype for function that converts the
contents of a SRECORD file to a binary image.

*/


#ifndef _SREC_TO_BIN_H
#define _SREC_TO_BIN_H


#ifdef VXWORKS
#include "vxWorks.h"
#endif

#ifdef WIN32
#include "WindowsTypes.h"
#endif

#ifdef MCOS
#include "McosTypes.h"
#endif /* MCOS */


#if (defined(__STDC__) || defined(__cplusplus)) 

#ifdef __cplusplus
extern "C" {
#endif

STATUS SrecToBin (unsigned char * SrecBuf,
		  unsigned int    SrecBufLength,
		  unsigned char * BinBuf,
		  unsigned int  * BinBufLength);

#ifdef __cplusplus
}
#endif

#else	/* __STDC__ */

STATUS SrecToBin ();

#endif	/* __STDC__ */

#endif /* _SREC_TO_BIN_H */
