/* SrecToBin.c - convert from S-Record format to binary format */

/* Copyright 2000 VMETRO ASA */

/*
modification history
--------------------
01d,14sep2004,meh  Updated to compile in an (VMOSA) Linux environment.
01c,21mar2002,nib  Updated to compile in an MCOS environment.
01b,20nov2001,nib  Updated to compile in a Windows environment.
01a,09may2000,caa  Created
*/

/*
DESCRIPTION

INCLUDE FILES
*/


/****** Includes ******/
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include "WindowsTypes.h"
#endif

#ifdef VMOSA
#include "vmosa.h"
#endif /* VMOSA */


#include "SrecToBin.h"


#define   ERR_MSG(paramList) printf paramList

/****** Defines ******/
/* Return codes for S-record format */
#define ErrBadSRec     -1
#define ErrUnknSRec    -2
#define ErrBadChkSum   -3
#define ErrBadAddr     -4
#define ErrBadLineNo   -5

#define StsSrecAdrs     1
#define StsRecCount     2
#define StsHeaderRec    3
#define StsModuleId     4
#define StsBootAddr     5
#define StsLastLine     6


/*****************************************************************************
 *   External Functions
 *****************************************************************************/
#if (defined(WIN32) || defined(MCOS))
#define   ns_printf   printf
#else
extern void ns_printf ();
#endif


/**** Forward Declarations *****/
static int GetHexUpperCase(char *Str,
			   int StartByte,
			   int NoOfDigits);

static int DecodeSrecord
(
 char           * SrecBuf,         /* In:  Unformatted input line  */
 UINT32         * SrecAdrs,        /* Out: Load address            */
 unsigned char  * SrecBinData,     /* Out: The binary bytes        */
 int            * SrecBytes        /* Out: No of binary bytes      */
 );



/**************************************************
 *  SrecToBin ()
 *
 *  Parameters:
 *    SrecBuf       - IN:  Pointer to memory buffer containing the S-Records
 *    SrecBufLength - IN:  Length of SrecBuf
 *    BinBuf        - OUT: Pointer to memory buffer of where to store the
 *                         resulting binary bytes.
 *    BinBufLength  - IN&OUT: Size of BinBuf as input and output.
 *
 *  Returns   : OK or ERROR
 */

STATUS SrecToBin
(
 unsigned char * SrecBuf,
 unsigned int    SrecBufLength,
 unsigned char * BinBuf,
 unsigned int  * BinBufLength
 )
{
  int             LineCount=1;
  char          * ReadPtr;
  UINT32          SrecAdrs;
  unsigned char   SrecBinData [64];
  int             SrecBytes;
  int             SrecStat;
  unsigned int    HighestSrecAdrsSoFar=0;
  
#if 0
  LOG_MSG(("Parsing S-records:\n"));
#endif
  
  /* Decode all S-Records */
  for ( ReadPtr = (char*) SrecBuf;
        ((unsigned long)ReadPtr - (unsigned long)SrecBuf) < SrecBufLength;
        ReadPtr++) {

    if (*ReadPtr == 'S') {

      SrecStat = DecodeSrecord (ReadPtr,
                                &SrecAdrs,
                                SrecBinData,
                                &SrecBytes);
      switch (SrecStat) {
#if (!defined(WIN32) && !defined(MCOS))
      case OK:
#endif
      case StsSrecAdrs:
      case StsRecCount:
      case StsHeaderRec:
      case StsModuleId:
      case StsBootAddr:
      case StsLastLine:
        /* check if new S-Record overflows the binary output buffer */
        if ((SrecAdrs + SrecBytes) > *BinBufLength) {
          ERR_MSG(("S-Record overflow the binary buffer.\n"));
          *BinBufLength = HighestSrecAdrsSoFar;
          return ERROR;
        }
        if (HighestSrecAdrsSoFar < (SrecAdrs + SrecBytes))
          HighestSrecAdrsSoFar = SrecAdrs + SrecBytes;
 
        /* move S-Record data to memory */
        memcpy ((void*)((unsigned long)BinBuf+SrecAdrs), SrecBinData, SrecBytes);

        /* update line count and print progress indicator */
        if (!(LineCount % 100))
#if 0
          LOG_MSG(("\r%d", LineCount));
#endif
        ++LineCount;

        break;
        
      case ErrBadSRec:
        ERR_MSG(("\nNot a valid S-record at line %d.\n", LineCount++));
        *BinBufLength = HighestSrecAdrsSoFar;
        return ERROR;
      case ErrBadChkSum:
        ERR_MSG(("\nChecksum error in HEX file at line %d.\n", LineCount++));
        *BinBufLength = HighestSrecAdrsSoFar;
        return ERROR;
      default:
        ERR_MSG(("\nError reading/decoding S-Record buffer. "
               "Lines processed:  %d.\n", LineCount++));
        *BinBufLength = HighestSrecAdrsSoFar;
        return ERROR;
      }
    }
  }

  /* return the highest S-Record address in BinBufLength*/
  *BinBufLength = HighestSrecAdrsSoFar;
#if 0
  LOG_MSG((" DONE.\n"));
#endif
  
  return OK;
}


static int DecodeSrecord
(
 char           * SrecBuf,         /* In:  Unformatted input line  */
 UINT32         * SrecAdrs,        /* Out: Load address            */
 unsigned char  * SrecBinData,     /* Out: The binary bytes        */
 int            * SrecBytes        /* Out: No of binary bytes      */
 )
{
  int ByteCount, CheckSum, idx, i;
  
  if (SrecBuf[0] != 'S') {              /* Does not line start with an S? */
    if (SrecBuf[0] == '$' && SrecBuf[1] == '$')
      return StsModuleId;
    return ErrBadSRec;               /* Return error status */
  }
  CheckSum = 0;

  /*Get no of HEX bytes in line*/
  ByteCount = (int) GetHexUpperCase(SrecBuf, 1, 2);
  
  for (i = 1; i <= ByteCount; i++) {
    CheckSum = (CheckSum + (int)GetHexUpperCase(SrecBuf, i, 2)) & 0xFF ;
  }
  CheckSum = 0xFF - CheckSum;                 /* Calculate finished checksum */
  
  if (GetHexUpperCase(SrecBuf, ByteCount + 1, 2) != CheckSum) {
    return (ErrBadChkSum);
  }
  *SrecAdrs = 0L;                        /* Set load address as null */
  *SrecBytes = 0;                        /* Set number of bytes as none */
  idx = 2;                               /* Start of address part of record */
  
  switch (SrecBuf[1]) {
  case '0' :                                             /* Header record */
  return StsHeaderRec;                     /* Do nothing. Return status */

  case '1' :                                        /* 16 bit data record */
    *SrecAdrs = GetHexUpperCase(SrecBuf, idx, 4);    /*  Get 4-digit address */
    idx += 2;
    for (i = 0; idx <= ByteCount; i++, idx++) {         /* Get data bytes */
      SrecBinData[i] = (unsigned char) GetHexUpperCase(SrecBuf, idx, 2);
    }
    *SrecBytes = i;
    return OK;
    
  case '2' :                                        /* 23 bit data record */
    *SrecAdrs = GetHexUpperCase(SrecBuf, idx, 6);     /* Get 6-digit address */
    idx += 3;
    for (i = 0; idx <= ByteCount; i++, idx++) {         /* Get data bytes */
      SrecBinData[i] = (unsigned char) GetHexUpperCase(SrecBuf, idx, 2);
    }
    *SrecBytes = i;
    return OK;
    
  case '3' :                                        /* 32 bit data record */
    *SrecAdrs = GetHexUpperCase(SrecBuf, idx, 8);     /* Get 8-digit address */
    idx += 4;
    for (i = 0; idx <= ByteCount; i++, idx++) {         /* Get data bytes */
      SrecBinData[i] = (unsigned char) GetHexUpperCase(SrecBuf, idx, 2);
    }
    *SrecBytes = i;
    return OK;
    
  case '5' :                                       /* Record count record */
    *SrecAdrs = GetHexUpperCase(SrecBuf, idx, 4);/* Get 4-digit record count */
    return StsRecCount;                                  /* Return status */
    
  case '7' :                               /* 8-digit load address record */
    *SrecAdrs = GetHexUpperCase(SrecBuf, idx, 8);     /* Get 8-digit address */
    return StsSrecAdrs;                                  /* Return status */
    
  case '8' :                               /* 6-digit load address record */
    *SrecAdrs = GetHexUpperCase(SrecBuf, idx, 6);     /* Get 6-digit address */
    return StsSrecAdrs;                                  /* Return status */
    
  case '9' :                               /* 6-digit load address record */
    return StsLastLine;                               /* Return Last Line */
    
  default :                                        /* Unknown Record Type */
    return (ErrUnknSRec);                          /* Return error status */
    
  } /* endswitch */
  return OK;
}


/**************************************************
 *  GetHexUpperCase ()
 *
 *  Parameters: Start of string, HEX byte number (0=first), no of digits.
 *  Returns   : The data value.
 */
static int GetHexUpperCase
(
 char  * Str,
 int     StartByte,
 int     NoOfDigits
 )
{
  short i;
  UINT32 RetVal;
  char *c;

  RetVal = 0;
  c = &Str[StartByte << 1];
  for (i = 0; i < NoOfDigits; i++, c++) {
    if (isdigit((int) *c)) /* .......................... Test number 0..9 */
      RetVal = RetVal * 16 + *c - '0';
    else if (isxdigit((int) *c)) /* ................ Test hex number A..F */
      RetVal = RetVal * 16 + *c - 55;
    else
      break;
  }
  return RetVal;
}
