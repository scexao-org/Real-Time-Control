/****************************************************************************
Module      : CDpio2InterruptAction.h

Description : Header file declaring classes for the interrupt actions
              required in the DPIO2 VxWorks Driver; CDpio2DisableInterrupt,
              CDpio2StartDma, CDpio2StopDma, CDpio2SuspendDma, 
              CDpio2ResumeDma, and CDpio2Signal.

              A CDpioDisableInterrupt object disables further interrupts
              from an interrupt source.

              A CDpio2StartDma object starts a DMA transfer, while 
              a CDpio2StopDma object stops an ongoing transfer.

              A CDpio2SuspendDma object temporarily stops an ongoing
              DMA transfer, while a CDpio2ResumeDma object restarts
              a previously suspended DMA transfer.

              A CDpio2Signal function calls a callback function provided
              by an application.
              

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,22may2001,nib  Created.

*****************************************************************************/

#ifndef __CDPIO2_INTERRUPT_ACTION_H
#define __CDPIO2_INTERRUPT_ACTION_H

#ifdef VMOSA
#include "dpio2.h"
#endif

#include "CAction.h"
#include "CDpio2DriverController.h"

#ifndef VMOSA
#include "dpio2.h"
#endif

class CDpio2DisableInterrupt : public CAction
{

public:

  CDpio2DisableInterrupt();

  void setDriverController(CDpio2DriverController* pDpioController);
  void setInterruptSourceId(DPIO2_INTERRUPT_SOURCE_ID sourceId);

  virtual void execute();

private:

  CDpio2DriverController*     m_pDpio2Controller;
  DPIO2_INTERRUPT_SOURCE_ID   m_sourceId;

};


class CDpio2StartDma : public CAction
{

public:

  CDpio2StartDma();

  void setDriverController(CDpio2DriverController* pDpio2Controller);
  void setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(), BOOL condition);

  virtual void execute();

private:

  CDpio2DriverController*   m_pDpio2Controller;

  BOOL (CDpio2DriverController::*m_pConditionMethod)();
  BOOL m_condition;

};


class CDpio2StopDma : public CAction
{

public:

  CDpio2StopDma();

  void setDriverController(CDpio2DriverController* pDpio2Controller);
  void setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(), BOOL condition);

  virtual void execute();

private:

  CDpio2DriverController*     m_pDpio2Controller;

  BOOL (CDpio2DriverController::*m_pConditionMethod)();
  BOOL m_condition;

};


class CDpio2SuspendDma : public CAction
{

public:

  CDpio2SuspendDma();

  void setDriverController(CDpio2DriverController* pDpio2Controller);
  void setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(), BOOL condition);

  virtual void execute();

private:

  CDpio2DriverController*     m_pDpio2Controller;

  BOOL (CDpio2DriverController::*m_pConditionMethod)();
  BOOL m_condition;

};


class CDpio2ResumeDma : public CAction
{

public:

  CDpio2ResumeDma();

  void setDriverController(CDpio2DriverController* pDpio2Controller);
  void setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(), BOOL condition);

  virtual void execute();

private:

  CDpio2DriverController*     m_pDpio2Controller;

  BOOL (CDpio2DriverController::*m_pConditionMethod)();
  BOOL m_condition;

};


class CDpio2Signal : public CAction
{

public:

  CDpio2Signal();

  void setDriverController(CDpio2DriverController* pDpio2Controller);
  void setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(), BOOL condition);

  virtual void execute();

  void setCallbackFunction(FUNCPTR pSignalFunction, sem_t* argument);

private:

  CDpio2DriverController*     m_pDpio2Controller;

  BOOL (CDpio2DriverController::*m_pConditionMethod)();
  BOOL m_condition;

  FUNCPTR m_pSignalFunction;
  sem_t* m_argument;

};


#endif /* __CDPIO2_INTERRUPT_ACTION_H */
