/* 
Revision History:
01c,14sep2004,meh  Ported to vmosa (Linux).
01b,01oct2003,meh  Changed ERROR_MSG to DPIO2_ERROR_MSG.
                   Changed the DEBUG_MSG with TRUE flag to
                   DPIO2_WARNING_MSG.
*/


#ifdef VXWORKS
#include "vxWorks.h"
#include "sysLib.h"
#endif

#ifdef WIN32
#include <stdio.h>
#endif


#ifdef VMOSA
#include "vmosa.h"
#endif /* VMOSA */


#include "flash.h"


#include "CDpio2Flash.h"
#include "DpioCommon.h"


#ifdef VXWORKS

#define SYS_IN_BYTE(address)   sysInByte(address)
#define SYS_OUT_BYTE(address, value)   sysOutByte(address, value)

#endif


#ifdef WIN32

#define SYS_IN_BYTE(address)   (*((volatile UINT8*) address))
#define SYS_OUT_BYTE(address, value)   ((*((volatile UINT8*) address)) = value)

#endif


#ifdef VM_LINUX

#define SYS_IN_BYTE(address)   (*((volatile UINT8*) address))
#define SYS_OUT_BYTE(address, value)   ((*((volatile UINT8*) address)) = value)

#endif /*VM_LINUX*/



CDpio2Flash::CDpio2Flash(UINT32 baseAddress)
  : m_baseAddress(baseAddress)
{

  ;

}


CDpio2Flash::~CDpio2Flash()
{

  ;

}


STATUS CDpio2Flash::eraseAndProgramSector(UINT8 sector, UINT8* pFlashData, UINT32 numberOfBytes)
{

  STATUS   status = OK;


  if ( sector > memdesc.nsect ) {

    DPIO2_ERROR_MSG(("The specified sector (%d) is not valid\n", sector));
    status = ERROR;

  }

  if ( numberOfBytes > ((UINT32) memdesc.sec[sector].size) ) {

    DPIO2_ERROR_MSG(("The specified length (%d bytes) is too long\n", 
		     (int) numberOfBytes));
    status = ERROR;

  }


  if ( status == OK ) {

    DPIO2_WARNING_MSG(("Erasing sector %d.\n", sector), TRUE);

    eraseSectorAndWaitForCompletion(sector);

    DPIO2_WARNING_MSG(("Enabling Unlock Bypass.\n"), TRUE);

    enableUnlockBypass(sector);

    DPIO2_WARNING_MSG(("Writing data to sector.\n"), TRUE);

    writeStringWithUnlockBypassed(sector, 0, pFlashData, numberOfBytes);

    DPIO2_WARNING_MSG(("Disabling Unlock Bypass.\n"), TRUE);

    disableUnlockBypass();    

  }


  return (status);

}


STATUS CDpio2Flash::verifySector(UINT8 sector, UINT8* pFlashData, UINT32 numberOfBytes)
{

  STATUS   status = OK;

  UINT32   offFlash;
  UINT8    value;


  if ( sector > memdesc.nsect ) {

    DPIO2_ERROR_MSG(("The specified sector (%d) is not valid\n", sector));
    status = ERROR;

  }

  if ( numberOfBytes > ((UINT32) memdesc.sec[sector].size) ) {

    DPIO2_ERROR_MSG(("The specified length (%d bytes) is too long\n", 
		     (int) numberOfBytes));
    status = ERROR;

  }


  if ( status == OK ) {

    DPIO2_WARNING_MSG(("Verifying data in sector %d:\n", sector), TRUE);

    for (offFlash = 0; offFlash < numberOfBytes; offFlash++) {

      value = readByte(sector, offFlash);
      if ( value != *pFlashData ) {

        DPIO2_WARNING_MSG(("Data on offset 0x%08X has changed: 0x%02X --> 0x%02X\n", 
			   (int) offFlash, *pFlashData, value), TRUE);

        status = ERROR;

      }

      pFlashData++;

    }

    if ( status == OK ) {

      DPIO2_WARNING_MSG(("Data is OK!\n"), TRUE);

    } else {

      DPIO2_WARNING_MSG(("Data is not OK!\n"), TRUE);

    }

  }


  return (status);

}


/*********************************************************************/
/* Usefull funtion to return the number of sectors in the device.    */
/* Can be used for functions which need to loop among all the        */
/* sectors, or wish to know the number of the last sector.           */
/*********************************************************************/

UINT8 CDpio2Flash::getNumberOfSectors()
{

  return (memdesc.nsect);

}


/*********************************************************************/
/* getSizeOfSector() is provided for cases in which the size   */
/* of a sector is required by a host application.  The sector size   */
/* (in bytes) is returned in the data location pointed to by the     */
/* 'size' parameter.                                                 */
/*********************************************************************/

STATUS CDpio2Flash::getSizeOfSector(UINT8 sector, UINT32& numberOfBytes)
{

  STATUS   status = OK;


  if ( sector > memdesc.nsect ) {

    DPIO2_ERROR_MSG(("The specified sector (%d) is not valid\n", sector));
    status = ERROR;

  }

  if ( status == OK ) {

    numberOfBytes = memdesc.sec[sector].size;

  }


  return (status);

}


/*********************************************************************/
/* The purpose of get_flash_memptr() is to return a memory pointer   */
/* which points to the beginning of memory space allocated for the   */
/* flash.  All function pointers are then referenced from this       */
/* pointer. 							     */
/*                                                                   */
/* Different systems will implement this in different ways:          */
/* possibilities include:                                            */
/*  - A direct memory pointer                                        */
/*  - A pointer to a memory map                                      */
/*  - A pointer to a hardware port from which the linear             */
/*    address is translated                                          */
/*  - Output of an MMU function / service                            */
/*                                                                   */
/* Also note that this function expects the pointer to a specific    */
/* sector of the device.  This can be provided by dereferencing      */
/* the pointer from a translated offset of the sector from a         */
/* global base pointer (e.g. flashptr = base_pointer + sector_offset)*/
/*                                                                   */
/* Important: Many AMD flash devices need both bank and or sector    */
/* address bits to be correctly set (bank address bits are A18-A16,  */
/* and sector address bits are A18-A12, or A12-A15).  Flash parts    */
/* which do not need these bits will ignore them, so it is safe to   */
/* assume that every part will require these bits to be set.         */
/*********************************************************************/

UINT32 CDpio2Flash::getFlashBaseAddress()
{

  return (m_baseAddress);

}


/*********************************************************************/
/* getStatusOfFlash utilizes the DQ6, DQ5, and DQ3 polling algorithms    */
/* described in the flash data book.  It can quickly ascertain the   */
/* operational status of the flash device, and return an             */
/* appropriate status code (defined in flash.h)                      */
/*********************************************************************/
 
int CDpio2Flash::getStatusOfFlash(UINT32 flashAddress)
{

  unsigned char d, t;
  int retry = 1;

again:

  d = SYS_IN_BYTE(flashAddress);
  t = d ^ SYS_IN_BYTE(flashAddress);

  if (t == 0) {           /* no toggles, nothing's happening */

    return FLASH_STATUS_READY;

  } else if (t == 0x04) { /* erase-suspend */

    if (retry--) {

      goto again;    /* may have been write completion */

    }

    return FLASH_STATUS_ERSUSP;

  } else if (t & 0x40) {

    if (d & 0x20) {     /* timeout */

      if (retry--) {

        goto again;    /* may have been write completion */

      }

      return FLASH_STATUS_TIMEOUT;

    } else {

      return FLASH_STATUS_BUSY;

    }

  }

  if (retry--) {

    goto again;    /* may have been write completion */

  }


  return FLASH_STATUS_ERROR;

}


/*********************************************************************/
/* issueCommand() is the main driver function.  It performs         */
/* every possible command available to AMD B revision                */
/* flash parts. Note that this command is not used directly, but     */
/* rather called through the API wrapper functions provided below.   */
/* This function can be called directly if desired (see wrapper      */
/* functions defined below.                                          */
/*********************************************************************/

void CDpio2Flash::issueCommand(int command, int sector, int offset, UINT8 data)
{

  const int retrycount[] = {0,0,0,0,0,0,0,0,0,0,0};

  UINT32   flashAddress;  /* flash window (64K bytes) */
  int      retry;
       
  /******************************************************************/
  /* On systems where bus glitching is prevalent, some long command */
  /* strings may be interrupted and cause the command to fail (this */
  /* is most probable on six cycle commands such as chip erase). In */
  /* order to ensure that issueCommand executes the command        */
  /* properly, it may be necessary to issue the command more than   */
  /* once in order for it to be accepted by the flash device.  In   */
  /* these cases it is recommended that the retry number be made    */
  /* positive (such as 1 or 2), so that issueCommand will try      */
  /* to issue the command more than once.  Keep in mind that this   */
  /* will only be attempted if the command fails in the first       */
  /* attempt.                                                       */
  /******************************************************************/
         
  retry = retrycount[command];

  flashAddress = getFlashBaseAddress();

again:

  if (command == FLASH_SELECT) {

    return;

  } else if (command == FLASH_RESET || command > FLASH_LASTCMD) {

    SYS_OUT_BYTE(flashAddress, 0xF0);

  } else if (command == FLASH_ESUSPEND) {

    SYS_OUT_BYTE(flashAddress, 0xB0);

  } else if (command == FLASH_ERESUME) {

    SYS_OUT_BYTE(flashAddress, 0x30);

  } else if (command == FLASH_UBPROG) {

    SYS_OUT_BYTE(flashAddress, 0xA0);
    SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset((memdesc.sec[sector].base) + offset), data);

  } else if (command == FLASH_UBRESET) {

    SYS_OUT_BYTE(flashAddress, 0x90);
    SYS_OUT_BYTE(flashAddress, 0x00);

  } else {

    /* unlock 1 */
    SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0xAA);
    /* unlock 2 */
    SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0x555), 0x55);

    switch (command) {
    case FLASH_AUTOSEL:
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0x90);
      break;

    case FLASH_PROG:
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0xA0);
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset((memdesc.sec[sector].base) + offset), data);
      break;

    case FLASH_CERASE:
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0x80);
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0xAA);
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0x555), 0x55);
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0x10);
      break;

    case FLASH_SERASE:
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0x80);
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0xAA);
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0x555), 0x55);
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset((memdesc.sec[sector].base)), 0x30);
      break;

    case FLASH_UB:
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0x20);
      break;
    }
  }

  if (retry-- > 0 && getStatusOfFlash(flashAddress) == FLASH_STATUS_READY) {

    goto again;

  }

}


/*********************************************************************/
/* writeToFlash extends the functionality of flash_program() by       */
/* providing an faster way to program multiple data bytes, without   */
/* needing the function overhead of looping algorithms which         */
/* program byte by byte.  This function utilizes fast pointers       */
/* to quickly loop through bulk data.                                */
/*********************************************************************/
int CDpio2Flash::writeToFlash(int sector, unsigned offset, UINT8* pBuf, int nbytes, int ub)
{

  UINT32   flashAddress; /* flash window */
  UINT32   dstOffset;
  UINT8*   pSrc;
  int stat;
  int retry = 0, retried = 0;

  flashAddress = getFlashBaseAddress();
  dstOffset =  memdesc.sec[sector].base + offset;   /* (byte offset) */

  pSrc = pBuf;

again:

  /* Check to see if we're in unlock bypass mode */
  if (ub == FALSE) {

    /* reset device to read mode */
    SYS_OUT_BYTE(flashAddress, 0xF0);

  }


  while ((stat = getStatusOfFlash(flashAddress)) == FLASH_STATUS_BUSY) {}
  if (stat != FLASH_STATUS_READY) {

    return (int) (pSrc - pBuf);

  }


  while (nbytes > 0) {

    if (ub == FALSE){

      /* unlock 1 */
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0xAA);
      /* unlock 2 */
      SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0x555), 0x55);

    }

    SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(0xAAA), 0xA0);

    SYS_OUT_BYTE(flashAddress + convertToDpio2FlashOffset(dstOffset), *pSrc++);

    dstOffset++;
    
    while ((stat = getStatusOfFlash(flashAddress)) == FLASH_STATUS_BUSY) {}
    if (stat != FLASH_STATUS_READY) break;

    nbytes--;

  }


  if (stat != FLASH_STATUS_READY || nbytes != 0) {

    if (retry-- > 0) {

      ++retried;

      /* back up */
      --dstOffset;
      --pSrc;
   
      /* and retry the last word */
      goto again;     

    }

    if (ub == FALSE) {

      issueCommand(FLASH_RESET,sector,0,0);

    }
  }


  return (int)(pSrc - pBuf);

}


/*********************************************************************/
/* BEGIN API WRAPPER FUNCTIONS                                       */
/*********************************************************************/
/* Flash_sector_erase() will erase a single sector dictated by the   */
/* sector parameter.                                                 */
/* Note: this function will merely BEGIN the erase program.  Code    */
/* execution will immediately return to the calling function         */
/*********************************************************************/

UINT8 CDpio2Flash::eraseSector(int sector)
{

  issueCommand(FLASH_SERASE,sector,0,0);
  return(1);

}


/*********************************************************************/
/* Flash_sector_erase_int() is identical to eraseSector(),    */
/* except it will wait until the erase is completed before returning */
/* control to the calling function.  This can be used in cases which */
/* require the program to hold until a sector is erased, without     */
/* adding the wait check external to this function.                  */
/*********************************************************************/

UINT8 CDpio2Flash::eraseSectorAndWaitForCompletion(int sector)
{

  issueCommand(FLASH_SERASE,sector,0,0);

  while (getStatusOfFlash(getFlashBaseAddress()) == FLASH_STATUS_BUSY) { 

    ;

  }

  return(1);

}

/*********************************************************************/
/* flash_reset() will reset the flash device to reading array data.  */
/* It is good practice to call this function after autoselect        */
/* sequences had been performed.                                     */
/*********************************************************************/

UINT8 CDpio2Flash::reset(void)
{

  issueCommand(FLASH_RESET,1,0,0);
  return(1);

}

/*********************************************************************/
/* getDeviceId() will perform an autoselect sequence on the  */
/* flash device, and return the device id of the component.          */
/* This function automatically resets to read mode.                  */
/*********************************************************************/

UINT8 CDpio2Flash::getDeviceId(int sector)
{

  UINT32   flashAddress;

  UINT8   answer;


  flashAddress = getFlashBaseAddress();

  issueCommand(FLASH_AUTOSEL,sector,0,0);
  answer = SYS_IN_BYTE(flashAddress + convertToDpio2FlashOffset(0x002));
	 
  issueCommand(FLASH_RESET,sector,0,0);   /* just to be safe */

  return (answer);

}

/*********************************************************************/
/* getManufacturerId() will perform an autoselect sequence on the */
/* flash device, and return the manufacturer code of the component.  */
/* This function automatically resets to read mode.                  */
/*********************************************************************/

UINT8 CDpio2Flash::getManufacturerId(int sector)
{

  UINT32   flashAddress;
  UINT8   answer;


  flashAddress = getFlashBaseAddress();

  issueCommand(FLASH_AUTOSEL,sector,0,0);
  answer = SYS_IN_BYTE(flashAddress);

  issueCommand(FLASH_RESET,sector,0,0);   /* just to be safe */	

  return (answer);

}

/*********************************************************************/
/* checkIsSectorProtected() performs an autoselect command      */
/* sequence which checks the status of the sector protect CAM        */
/* to check if the particular sector is protected.  Function will    */
/* return a '0' is the sector is unprotected, and a '1' if it is     */
/* protected.                                                        */
/*********************************************************************/

UINT8 CDpio2Flash::checkIsSectorProtected(int sector)
{

  UINT32   flashAddress;

  UINT8   answer;

  flashAddress = getFlashBaseAddress();

  issueCommand(FLASH_AUTOSEL,sector,0,0);

  answer = SYS_IN_BYTE(flashAddress + convertToDpio2FlashOffset((memdesc.sec[sector].base) + 0x004)) & 0x01; /* Only need DQ0 to check */

  issueCommand(FLASH_RESET,sector,0,0);

  return (answer);

}

/*********************************************************************/
/* getStatusOfSector() will return the current operational status of  */
/* the flash device.  A list of return codes is outlined in flash.h  */
/* Note: for DL parts, status will be bank dependent.                */
/*********************************************************************/

UINT8 CDpio2Flash::getStatusOfSector(int sector)
{

  UINT32   flashAddress;


  flashAddress = getFlashBaseAddress() + convertToDpio2FlashOffset(memdesc.sec[sector].base);
  return getStatusOfFlash(flashAddress);

}

/*********************************************************************/
/* eraseFlash() will perform a complete erasure of the flash   */
/* device.  							     */
/*********************************************************************/

UINT8 CDpio2Flash::eraseFlash(int sector)
{

  issueCommand(FLASH_CERASE,sector,0,0);
  return(1);

}

/*********************************************************************/
/* writeByte() will program a single bute of data at the      */
/* specified offset from the beginning of the sector parameter.      */
/* Note: It is good practice to check the desired offset by first    */
/* reading the data, and checking to see if it contains 0xFFFF       */
/*********************************************************************/

UINT8 CDpio2Flash::writeByte(int sector, int offset, UINT8 data)
{

  issueCommand(FLASH_PROG, sector, offset, data);
  return (1);

}

/*********************************************************************/
/* readByte() reads a single byte of data from the specified  */
/* offset from the sector parameter.                                 */
/*********************************************************************/

UINT8 CDpio2Flash::readByte(int sector, int offset)
{

  UINT32   flashAddress;


  issueCommand(FLASH_SELECT,sector,0,0);

  flashAddress = getFlashBaseAddress();

  return (SYS_IN_BYTE(flashAddress + convertToDpio2FlashOffset((memdesc.sec[sector].base) + offset)));

}


/*********************************************************************/
/* writeString() functions like writeByte(), except    */
/* that it accepts a pointer to a buffer to be programmed.  This     */
/* function will bulk program the flash device with the provided data. */
/*********************************************************************/

UINT8 CDpio2Flash::writeString(int sector, int offset, UINT8 *buffer, int numbytes)
{

  writeToFlash(sector, offset, buffer, numbytes, FALSE);
  return (1);

}

/*********************************************************************/
/* suspendSectorErase() will suspend an erase process in the        */
/* specified sector.  Array data can then be read from other sectors */
/* (or any other sectors in other banks), and the erase can be       */
/* resumed using the flash_erase_resume function.                    */
/* Note: multiple sectors can be queued for erasure, so long as the  */
/* 80 uS erase suspend window has not terminated (see AMD data sheet */
/* concerning erase_suspend restrictions).                           */
/*********************************************************************/

UINT8 CDpio2Flash::suspendSectorErase(int sector)
{

  issueCommand(FLASH_ESUSPEND, sector, 0, 0);
  return (1);

}

/*********************************************************************/
/* resumeSectorErase() will resume all pending erases in the bank   */
/* in which the sector parameter is located.                         */
/*********************************************************************/

UINT8 CDpio2Flash::resumeSectorErase(int sector)
{

  issueCommand(FLASH_ERESUME, sector, 0, 0);
  return (1);

}

/*********************************************************************/
/* UNLOCK BYPASS FUNCTIONS                                           */
/*********************************************************************/
/* Unlock bypass mode is useful whenever the calling application     */
/* wished to program large amounts of data in minimal time.  Unlock  */
/* bypass mode remove half of the bus overhead required to program   */
/* a single word, from 4 cycles down to 2 cycles.  Programming of    */
/* individual bytes does not gain measurable benefit from unlock     */
/* bypass mode, but programming large strings can see a significant  */
/* decrease in required programming time.                            */
/*********************************************************************/

/*********************************************************************/
/* enableUnlockBypass() places the flash into unlock bypass mode.  This        */
/* is REQUIRED to be called before any of the other unlock bypass    */
/* commands will become valid (most will be ignored without first    */
/* calling this function.                                            */
/*********************************************************************/
 
UINT8 CDpio2Flash::enableUnlockBypass(int sector)
{

  issueCommand(FLASH_UB, sector, 0, 0);
  return(1);

}

/*********************************************************************/
/* writeByteWithUnlockBypassed() programs a single byte using unlock bypass  */
/* mode.  Note that the calling application will see little benefit  */
/* from programming single bytess using this mode (outlined above)    */
/*********************************************************************/

UINT8 CDpio2Flash::writeByteWithUnlockBypassed(int sector, int offset, UINT8 data)
{

  issueCommand(FLASH_UBPROG, sector, offset, data);
  return (1);

}

/*********************************************************************/
/* writeStringWithUnlockBypassed() behaves in the exact same manner as       */
/* writeString() (outlined above), expect that it utilizes    */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/

UINT8 CDpio2Flash::writeStringWithUnlockBypassed(int sector, int offset,
                                    UINT8 *buffer, int numbytes)
{

  writeToFlash(sector, offset, buffer, numbytes, TRUE);
  return (1);

}

/*********************************************************************/
/* disableUnlockBypass() is required to remove the flash from unlock      */
/* bypass mode.  This is important, as other flash commands will be  */
/* ignored while the flash is in unlock bypass mode.                 */
/*********************************************************************/

UINT8 CDpio2Flash::disableUnlockBypass(void)
{

  issueCommand(FLASH_UBRESET,1,0,0);
  return(1);

}


UINT32 CDpio2Flash::getSizeOfFlash()
{

  int   iLastSector;


  iLastSector = memdesc.nsect - 1;

  return (memdesc.sec[iLastSector].base + memdesc.sec[iLastSector].size);

}


const char *DavidString1 = "Am29LV800T";

CDpio2Pci33Flash::CDpio2Pci33Flash(UINT32 baseAddress)
  : CDpio2Flash(baseAddress)
{

  struct flashinfo Am29LV800T = { (char *)DavidString1, 0L, 0, 19, 0, 4 ,
                                  {
                                    {65536,     0x00000,    1},
                                    {65536,     0x10000,    1},
                                    {65536,     0x20000,    1},
                                    {65536,     0x30000,    1},
                                    {65536,     0x40000,    1},
                                    {65536,     0x50000,    1},
                                    {65536,     0x60000,    1},
                                    {65536,     0x70000,    1},
                                    {65536,     0x80000,    1},
                                    {65536,     0x90000,    1},
                                    {65536,     0xA0000,    1},
                                    {65536,     0xB0000,    1},
                                    {65536,     0xC0000,    1},
                                    {65536,     0xD0000,    1},
                                    {65536,     0xE0000,    1},
                                    {32768,     0xF0000,    1},
                                    {8192,      0xF8000,    1},
                                    {8192,      0xFA000,    1},
                                    {16384,     0xFC000,    1}
                                  } };


  memdesc = Am29LV800T;

}


CDpio2Pci33Flash::~CDpio2Pci33Flash()
{

  ;

}


CDpio2Pci33PcbAFlash::CDpio2Pci33PcbAFlash(UINT32 baseAddress)
  : CDpio2Pci33Flash(baseAddress)
{

  ;

}


CDpio2Pci33PcbAFlash::~CDpio2Pci33PcbAFlash()
{

  ;

}


/*********************************************************************/
/* On the PCB A version of the DPIO2 module the address bits         */
/* to the FLASH have been switched.  To correct this the method      */
/* below should be used convert all offsets when accessing the FLASH */
/*********************************************************************/
UINT32 CDpio2Pci33PcbAFlash::convertToDpio2FlashOffset(UINT32 offset)
{
 
  return ((((offset) & 0x000FFFFE) << 1) | (((offset) & 0x00000001) << 21));

}


CDpio2Pci33PcbBFlash::CDpio2Pci33PcbBFlash(UINT32 baseAddress)
  : CDpio2Pci33Flash(baseAddress)
{

  ;

}


CDpio2Pci33PcbBFlash::~CDpio2Pci33PcbBFlash()
{

  ;

}


UINT32 CDpio2Pci33PcbBFlash::convertToDpio2FlashOffset(UINT32 offset)
{
 
  return (((offset) & 0x000FFFFF) << 2);

}


const char *DavidString2 = "Am29LV160DT";

CDpio2Pci66Flash::CDpio2Pci66Flash(UINT32 baseAddress)
  : CDpio2Flash(baseAddress)
{

  struct flashinfo Am29LV160DT = { (char *)DavidString2, 0L, 0, 35, 0, 4 ,
                                  {
                                    {65536,     0x000000,    1},
                                    {65536,     0x010000,    1},
                                    {65536,     0x020000,    1},
                                    {65536,     0x030000,    1},
                                    {65536,     0x040000,    1},
                                    {65536,     0x050000,    1},
                                    {65536,     0x060000,    1},
                                    {65536,     0x070000,    1},
                                    {65536,     0x080000,    1},
                                    {65536,     0x090000,    1},
                                    {65536,     0x0A0000,    1},
                                    {65536,     0x0B0000,    1},
                                    {65536,     0x0C0000,    1},
                                    {65536,     0x0D0000,    1},
                                    {65536,     0x0E0000,    1},
                                    {65536,     0x0F0000,    1},
                                    {65536,     0x100000,    1},
                                    {65536,     0x110000,    1},
                                    {65536,     0x120000,    1},
                                    {65536,     0x130000,    1},
                                    {65536,     0x140000,    1},
                                    {65536,     0x150000,    1},
                                    {65536,     0x160000,    1},
                                    {65536,     0x170000,    1},
                                    {65536,     0x180000,    1},
                                    {65536,     0x190000,    1},
                                    {65536,     0x1A0000,    1},
                                    {65536,     0x1B0000,    1},
                                    {65536,     0x1C0000,    1},
                                    {65536,     0x1D0000,    1},
                                    {65536,     0x1E0000,    1},
                                    {32768,     0x1F0000,    1},
                                    {8192,      0x1F8000,    1},
                                    {8192,      0x1FA000,    1},
                                    {16384,     0x1FC000,    1}
                                  } };

  memdesc = Am29LV160DT;

  m_pAddressBit20ControlFunction = NULL;

}


CDpio2Pci66Flash::~CDpio2Pci66Flash()
{

  /* Make sure that address bit 20 is cleared.
   */
  if ( m_pAddressBit20ControlFunction != NULL ) {

    m_pAddressBit20ControlFunction(FALSE, m_pControlFunctionArg);

  }

}


void CDpio2Pci66Flash::setAddressBit20ControlFunction(void (*pFunction)(BOOL, void*), 
                                                      void* p_arg)
{

  m_pAddressBit20ControlFunction = pFunction;

  m_pControlFunctionArg = p_arg;


  /* Set address bit 20 to zero initially.
   */
  m_pAddressBit20ControlFunction(FALSE, m_pControlFunctionArg);

  m_addressBit20IsSet = FALSE;

}


UINT32 CDpio2Pci66Flash::convertToDpio2FlashOffset(UINT32 offset)
{
 
  if ( m_pAddressBit20ControlFunction != NULL ) {

    if ( !m_addressBit20IsSet && ((offset & 0x00100000) == 0x00100000) ) {

      m_pAddressBit20ControlFunction(TRUE, m_pControlFunctionArg);
      m_addressBit20IsSet = TRUE;

    } else if ( m_addressBit20IsSet && ((offset & 0x00100000) == 0x00000000) ) {
        
      m_pAddressBit20ControlFunction(FALSE, m_pControlFunctionArg);
      m_addressBit20IsSet = FALSE;

    }

  }

  return (((offset) & 0x000FFFFF) << 2);

}

/*********************************************************************/
