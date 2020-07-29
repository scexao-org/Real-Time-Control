/****************************************************************************
Module      : CDpio2Flash.h

Description : Header file declaring the CDpio2Flash class.
              A CDpio2Flash object has the functionality for erasing,
              programming, and verifying the FLASH device on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01f,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01e,14aug2002,nib  Made readByte() public.
  01d,21mar2002,nib  Updated to compile in an MCOS environment.
  01c,19dec2001,nib  Updated file to support DPIO2 PCB B.
  01b,20nov2001,nib  Updated to compile in a Windows environment.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifdef VXWORKS
#include "vxWorks.h"
#endif

#ifdef WIN32
#include <windows.h>
#include "WindowsTypes.h"
#endif

#ifdef MCOS
#include "McosTypes.h"
#endif /* MCOS */

#ifdef VMOSA
#include "vmosa.h"
#endif


#include "flash.h"

class CDpio2Flash
{

public:

  CDpio2Flash(UINT32 baseAddress);
  virtual ~CDpio2Flash();

  STATUS eraseAndProgramSector(UINT8 sector, UINT8* pFlashData, UINT32 numberOfBytes);
  STATUS verifySector(UINT8 sector, UINT8* pFlashData, UINT32 numberOfBytes);
  UINT8 getNumberOfSectors();
  STATUS getSizeOfSector(UINT8 sector, UINT32& numberOfBytes);

  inline UINT8 readByte(int sector, int offset);

  UINT8 getDeviceId(int sector);
  UINT8 getManufacturerId(int sector);

  UINT32 getSizeOfFlash();

protected:

  struct flashinfo memdesc;

private:

  UINT32   m_baseAddress;


  inline UINT32 getFlashBaseAddress();
  inline int getStatusOfFlash(UINT32 flashAddress);
  inline void issueCommand(int command, int sector, int offset, UINT8 data);
  inline int writeToFlash(int sector, unsigned offset, UINT8* pBuf, int nbytes, int ub);
  inline UINT8 eraseSector(int sector);
  inline UINT8 eraseSectorAndWaitForCompletion(int sector);
  inline UINT8 reset(void);
  inline UINT8 checkIsSectorProtected(int sector);
  inline UINT8 getStatusOfSector(int sector);
  inline UINT8 eraseFlash(int sector);
  inline UINT8 writeByte(int sector, int offset, UINT8 data);
  inline UINT8 writeString(int sector, int offset, UINT8 *buffer, int numbytes);
  inline UINT8 suspendSectorErase(int sector);
  inline UINT8 resumeSectorErase(int sector);
  inline UINT8 enableUnlockBypass(int sector);
  inline UINT8 writeByteWithUnlockBypassed(int sector, int offset, UINT8 data);
  inline UINT8 writeStringWithUnlockBypassed(int sector, int offset, UINT8 *buffer, int numbytes);
  inline UINT8 disableUnlockBypass(void);

  virtual UINT32 convertToDpio2FlashOffset(UINT32 offset) = 0;

};


class CDpio2Pci33Flash : public CDpio2Flash
{

public:

  CDpio2Pci33Flash(UINT32 baseAddress);
  virtual ~CDpio2Pci33Flash();

private:

};


class CDpio2Pci33PcbAFlash : public CDpio2Pci33Flash
{

public:

  CDpio2Pci33PcbAFlash(UINT32 baseAddress);
  virtual ~CDpio2Pci33PcbAFlash();

private:

  virtual UINT32 convertToDpio2FlashOffset(UINT32 offset);

};


class CDpio2Pci33PcbBFlash : public CDpio2Pci33Flash
{

public:

  CDpio2Pci33PcbBFlash(UINT32 baseAddress);
  virtual ~CDpio2Pci33PcbBFlash();

private:

  virtual UINT32 convertToDpio2FlashOffset(UINT32 offset);

};


class CDpio2Pci66Flash : public CDpio2Flash
{

public:

  CDpio2Pci66Flash(UINT32 baseAddress);
  virtual ~CDpio2Pci66Flash();


  void setAddressBit20ControlFunction(void (*pFunction)(BOOL, void*), void* p_arg);


private:

  void (*m_pAddressBit20ControlFunction)(BOOL setBit, void* p_arg);

  void*  m_pControlFunctionArg;

  BOOL   m_addressBit20IsSet;


  virtual UINT32 convertToDpio2FlashOffset(UINT32 offset);

};


