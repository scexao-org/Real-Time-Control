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


Module      : vmosaCDpio2HwInterface.h

Description : Header file declaring the CDpio2HwInterface class.
              A CDpio2HwInterface object creates and holds the aggregate
              objects which make up the main functionality of a DPIO2 module.
              In addition a CDpio2HwInterface object represents high level
              functionality, such as controlling the direction of the module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Ported from CDpio2HwInterface.h

*****************************************************************************/

#ifndef CDPIO2HWINTERFACE_H_INCLUDED
#define CDPIO2HWINTERFACE_H_INCLUDED

#include "vmosa.h"

#include "vmosaCControlBit.h"
#include "vmosaCDpio2DmaController.h"
#include "CDpio2Fifo.h"
#include "CDpio2FrontEnd.h"

#include "Dpio2Defs.h"


class CInterruptManager;
class CDpio2Oscillator;
class CDpio2Leds;
class CDpio2FrontEnd;
class CDpio2Fifo;
class CDpio2DmaListIterator;
class CDpio2DmaController;
class CControlRegister;
class CStatusBit;


const int   DPIO2_MAX_NUM_ACTIONS_PER_SOURCE = 10;


enum
{
  STATUS_BIT_MASK_MASTER_DATA_PARITY_ERROR = 0x0100,
  STATUS_BIT_SIGNALED_TARGET_ABORT = 0x0800,
  STATUS_BIT_RECEIVED_TARGET_ABORT = 0x1000,
  STATUS_BIT_RECEIVED_MASTER_ABORT = 0x2000,
  STATUS_BIT_SIGNALED_SYSTEM_ERROR = 0x4000
};




class CDpio2HwInterface
{
  public:

    CDpio2HwInterface ( HANDLE hDpio2 );

    virtual ~CDpio2HwInterface();


    inline CDpio2DmaController* getPrimaryDmaController();

    inline CDpio2Fifo* getFifo();

    inline CDpio2FrontEnd* getFrontEnd();

    inline CDpio2Oscillator* getOscillator();

    inline CDpio2Leds* getLeds();

    inline CInterruptManager* getInterruptManager();


    inline void enableAsInput();

    inline void enableAsOutput();

    inline BOOL checkIsOutput();


    //## Enables the DPIO2 module to signal interrupts on pin INTB# instead of
    //## INTA#.
    inline void enableInterruptPinB();

    //## Disables the DPIO2 module from signaling interrupts on pin INTB#
    //## instead of INTA#.
    inline void disableInterruptPinB();


    inline UINT32 getBaseAddressOfFifo();
    inline UINT32 getBaseAddressOfSram();
    inline UINT32 getBaseAddressOfFlash();

    inline UINT32 getBaseAddressSlaveImage0();

    inline UINT8 getPciFpgaVersion();
    inline UINT8 getFrontEndFpgaVersion();


    inline void setFlashAddressBit20();
    inline void clearFlashAddressBit20();


    inline STATUS getDeviceId(UINT16& id);
    inline STATUS getRevisionId(UINT8& id);
    inline UINT8  getPciDeviceNumber();
    inline UINT8  getPciBusNumber();

    UINT8  getLatencyTimer();
    void   setLatencyTimer(UINT8 timerValue);
    BOOL   checkHasMasterDataParityErrorOccurred();
    BOOL   checkIsTargetAbortSignaled();
    BOOL   checkIsTargetAbortReceived();
    BOOL   checkIsMasterAbortReceived();
    BOOL   checkIsSystemErrorSignaled();
    UINT16 readPciConfigStatus();
    

    void initialize();


  protected:

    CDpio2DmaController* m_pPrimaryDmaController;

    CDpio2DmaController *m_pSecondaryDmaController;

    CDpio2Fifo* m_pFifo;

    CDpio2FrontEnd* m_pFrontEnd;

    CDpio2Oscillator *m_pOscillator;

    CDpio2Leds *m_pLeds;

    CInterruptManager *m_pInterruptManager;

    CControlBit* m_pBitDisableDirectionJumper;

    CControlBit* m_pBitEnableAsOutput;

    CControlBit* m_pBitEnableInterruptPinB;

    CControlBit* m_pFlashAddressBit20;


    CStatusBit *m_pBitDirectionIsInput;

    CStatusBitField* m_pPciFpgaVersion;
    CStatusBitField* m_pFrontEndFpgaVersion;


    CControlRegister* m_pTestRegister;



  private:

    HANDLE              m_hDpio2;

    CControlRegister*   m_pDummyControlRegister;



    void createControlRegisters(void);
    void deleteControlRegisters(void);

    void createInterruptManager();
    void createPrimaryDmaController();
    void createFifo();
    void createFrontEnd();
    void createOscillator();
    void createLeds();

};


CDpio2DmaController* CDpio2HwInterface::getPrimaryDmaController() 
{

  return (m_pPrimaryDmaController);

}

 
CDpio2Fifo* CDpio2HwInterface::getFifo()
{

  return (m_pFifo);

}


CDpio2FrontEnd* CDpio2HwInterface::getFrontEnd() 
{

  return (m_pFrontEnd);

}


CDpio2Oscillator* CDpio2HwInterface::getOscillator() 
{

  return (m_pOscillator);

}


CDpio2Leds* CDpio2HwInterface::getLeds() 
{

  return (m_pLeds);

}


CInterruptManager* CDpio2HwInterface::getInterruptManager() 
{

  return (m_pInterruptManager);

}


void CDpio2HwInterface::enableAsInput()
{

  CDpio2FrontEnd*   pFrontEnd;


  /* The frontend object pointer can always be downcasted 
   * to a CDpio2FrontEnd object pointer, because the object it references
   * is created as a CDpio2FrontEnd object by this CDpio2HwInterface object.
   */
  pFrontEnd = static_cast<CDpio2FrontEnd*>(m_pFrontEnd);


  /* Set the direction of the DPIO2 as input,
   * and inform the objects that are dependent upon direction.
   */
  m_pBitEnableAsOutput->clear();
  
  m_pPrimaryDmaController->setDirectionAsIn();
  pFrontEnd->setDirectionAsIn();

}


void CDpio2HwInterface::enableAsOutput()
{

  CDpio2FrontEnd*   pFrontEnd;


  /* The frontend object pointer can always be downcasted 
   * to a CDpio2FrontEnd object pointer, because the object it references
   * is created as a CDpio2FrontEnd object by this CDpio2HwInterface object.
   */
  pFrontEnd = static_cast<CDpio2FrontEnd*>(m_pFrontEnd);


  /* Set the direction of the DPIO2 as input,
   * and inform the objects that are dependent upon direction.
   */
  m_pBitEnableAsOutput->set();
  
  m_pPrimaryDmaController->setDirectionAsOut();
  pFrontEnd->setDirectionAsOut();

}


BOOL CDpio2HwInterface::checkIsOutput()
{

  return (!m_pBitDirectionIsInput->checkIsSet());

}


void CDpio2HwInterface::enableInterruptPinB()
{

  m_pBitEnableInterruptPinB->set();

}


void CDpio2HwInterface::disableInterruptPinB() 
{

  m_pBitEnableInterruptPinB->clear();
      
}


/**/
UINT32 CDpio2HwInterface::getBaseAddressOfFifo()
{

  if (m_hDpio2->user.fifo_base == 0)

    throw;


  return ( (UINT32) m_hDpio2->user.fifo_base );

}


/**/
UINT32 CDpio2HwInterface::getBaseAddressOfSram()
{

  if (m_hDpio2->user.ctrl_base == 0)

    throw;
 
 
  return ( (UINT32) ( (UINT32) m_hDpio2->user.ctrl_base + DPIO2_SRAM_BASE_ADDRESS ) );

}


/**/
UINT32 CDpio2HwInterface::getBaseAddressOfFlash()
{

  if (m_hDpio2->user.ctrl_base == 0)

    throw;
 
 
  return ((UINT32) ( (UINT32) m_hDpio2->user.ctrl_base + DPIO2_FLASH_BASE_ADDRESS ));

}


/**/
#if 0 /* only used for hardware test */ 
UINT32 CDpio2HwInterface::getBaseAddressSlaveImage0() 
{

  return (m_baseAddress);

}
#endif /* only used for hardware test */ 


UINT8  CDpio2HwInterface::getPciFpgaVersion()
{

  return ((UINT8) m_pPciFpgaVersion->getValue());

}


UINT8  CDpio2HwInterface::getFrontEndFpgaVersion()
{

  return ((UINT8) m_pFrontEndFpgaVersion->getValue());

}


void CDpio2HwInterface::setFlashAddressBit20()
{

  m_pFlashAddressBit20->set();

}


void CDpio2HwInterface::clearFlashAddressBit20()
{

  m_pFlashAddressBit20->clear();

}





STATUS CDpio2HwInterface::getDeviceId(UINT16& id)
{

  id = m_hDpio2->pci.device_id;

  return OK;

}

STATUS CDpio2HwInterface::getRevisionId(UINT8& id)
{
  
  id = m_hDpio2->pci.revision_id;

  return OK;

}


UINT8  CDpio2HwInterface::getPciDeviceNumber()
{
  
  return ( m_hDpio2->pci.device_number );

}


UINT8  CDpio2HwInterface::getPciBusNumber()
{
  
  return ( m_hDpio2->pci.bus_number );

}






#endif /* CDPIO2HWINTERFACE_H_INCLUDED */


