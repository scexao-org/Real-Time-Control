/****************************************************************************
Module      : CDpio2InterruptAction.cpp

Description : Source file defining classes for the interrupt actions
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

#include "CDpio2InterruptAction.h"

CDpio2DisableInterrupt::CDpio2DisableInterrupt()
  : m_pDpio2Controller(NULL),
    m_sourceId(DPIO2_INVALID_INTERRUPT_SOURCE_ID)
{

  ;

}


void CDpio2DisableInterrupt::setDriverController(CDpio2DriverController* pDpio2Controller)
{

  m_pDpio2Controller = pDpio2Controller;

}


void CDpio2DisableInterrupt::setInterruptSourceId(DPIO2_INTERRUPT_SOURCE_ID sourceId)
{

  m_sourceId = sourceId;

}


void CDpio2DisableInterrupt::execute()
{

  if ( m_pDpio2Controller != NULL ) {

    m_pDpio2Controller->disableInterruptSource(m_sourceId);

  }

}


CDpio2StartDma::CDpio2StartDma()
  : m_pDpio2Controller(NULL),
    m_pConditionMethod(NULL)
{

  ;

}


void CDpio2StartDma::setDriverController(CDpio2DriverController* pDpio2Controller)
{

  m_pDpio2Controller = pDpio2Controller;

}


void CDpio2StartDma::setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(),
                                  BOOL condition)
{

  m_pConditionMethod = pConditionMethod;
  m_condition = condition;

}


void CDpio2StartDma::execute()
{

  if ( m_pDpio2Controller != NULL ) {

    if ( m_pConditionMethod == NULL ) {

      m_pDpio2Controller->startDmaTransfer();

    } else {

      if ( (m_pDpio2Controller->*m_pConditionMethod)() == m_condition ) {

        m_pDpio2Controller->startDmaTransfer();
      
      }

    }

  }

}


CDpio2StopDma::CDpio2StopDma()
  : m_pDpio2Controller(NULL),
    m_pConditionMethod(NULL)
{

  ;

}


void CDpio2StopDma::setDriverController(CDpio2DriverController* pDpio2Controller)
{

  m_pDpio2Controller = pDpio2Controller;

}


void CDpio2StopDma::setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(),
                                 BOOL condition)
{

  m_pConditionMethod = pConditionMethod;
  m_condition = condition;

}


void CDpio2StopDma::execute()
{

  if ( m_pDpio2Controller != NULL ) {

    if ( m_pConditionMethod == NULL ) {

      m_pDpio2Controller->stopDmaTransfer();

    } else {

      if ( (m_pDpio2Controller->*m_pConditionMethod)() == m_condition ) {

        m_pDpio2Controller->stopDmaTransfer();
      
      }

    }

  }

}


CDpio2SuspendDma::CDpio2SuspendDma()
  : m_pDpio2Controller(NULL),
    m_pConditionMethod(NULL)
{

  ;

}


void CDpio2SuspendDma::setDriverController(CDpio2DriverController* pDpio2Controller)
{

  m_pDpio2Controller = pDpio2Controller;

}


void CDpio2SuspendDma::setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(),
                                    BOOL condition)
{

  m_pConditionMethod = pConditionMethod;
  m_condition = condition;

}


void CDpio2SuspendDma::execute()
{

  if ( m_pDpio2Controller != NULL ) {

    if ( m_pConditionMethod == NULL ) {

      m_pDpio2Controller->suspendDmaTransfer();

    } else {

      if ( (m_pDpio2Controller->*m_pConditionMethod)() == m_condition ) {

        m_pDpio2Controller->suspendDmaTransfer();
      
      }

    }

  }

}


CDpio2ResumeDma::CDpio2ResumeDma()
  : m_pDpio2Controller(NULL),
    m_pConditionMethod(NULL)
{

  ;

}


void CDpio2ResumeDma::setDriverController(CDpio2DriverController* pDpio2Controller)
{

  m_pDpio2Controller = pDpio2Controller;

}


void CDpio2ResumeDma::setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(),
                                   BOOL condition)
{

  m_pConditionMethod = pConditionMethod;
  m_condition = condition;

}


void CDpio2ResumeDma::execute()
{

  if ( m_pDpio2Controller != NULL ) {

    if ( m_pConditionMethod == NULL ) {

      m_pDpio2Controller->resumeDmaTransfer();

    } else {

      if ( (m_pDpio2Controller->*m_pConditionMethod)() == m_condition ) {

        m_pDpio2Controller->resumeDmaTransfer();
      
      }

    }

  }

}


CDpio2Signal::CDpio2Signal()
  : m_pDpio2Controller(NULL),
    m_pSignalFunction(NULL)
{

  ;

}


void CDpio2Signal::setDriverController(CDpio2DriverController* pDpio2Controller)
{

  m_pDpio2Controller = pDpio2Controller;

}


void CDpio2Signal::setCondition(BOOL (CDpio2DriverController::*pConditionMethod)(),
                                BOOL condition)
{

  m_pConditionMethod = pConditionMethod;
  m_condition = condition;

}

void CDpio2Signal::execute()
{

  if ( m_pDpio2Controller != NULL ) {

    if ( m_pConditionMethod == NULL ) {

      if ( m_pSignalFunction != NULL ) {

        (*m_pSignalFunction)(m_argument);

      }

    } else {

      if ( (m_pDpio2Controller->*m_pConditionMethod)() == m_condition ) {

        if ( m_pSignalFunction != NULL ) {

          (*m_pSignalFunction)(m_argument);

        }
      
      }

    }

  }

}


void CDpio2Signal::setCallbackFunction(FUNCPTR pSignalFunction, sem_t* argument)
{

  m_pSignalFunction = pSignalFunction;
  m_argument = argument;

}


