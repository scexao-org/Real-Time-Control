/****************************************************************************
Module      : vmosaCInterruptManager.cpp

Description : Source file defining the CInterruptManager class.
              A CInterruptManager object is used to manage interrupts from
              a set of interrupt sources represented by CInterruptSource objects.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01b,18okt2004,meh  In notifyAboutInterrupt.
                     Moved enable (m_pInterruptSource[sourceId]->clear())
                     of new interrupt before execute of user define action.
                     This is possible as long as the global interrupt mask
                     is set as the last of operation done during interrupt
                     handling.
  01a,28jun2004,meh  Ported from CInterruptManager.cpp

*****************************************************************************/

#include "vmosa.h"

#include "vmosaCInterruptManager.h"
#include "CAction.h"


#include "DpioCommon.h"
#ifndef DB
#define DB(args) \
       DEBUG_MSG (("DB: %s %d:\n", __FILE__, __LINE__),TRUE); \
       DEBUG_MSG ( args , TRUE ); \
       vmosa_mdelay (100);
#endif


#if 1
#define INTR_ENABLE_BEFORE_EXE
#endif



STATUS CInterruptManager::addInterruptAction(int sourceId, CAction* pAction)
{

  int   iAction;


  /* Check that the parameters have valid values.
   */
  if ( (sourceId < 0) || (sourceId >= m_maxNumSources) ) {

    return (ERROR);

  }

  /* Check that the pointer to the new action can be stored.
   */
  else if ( m_numActions[sourceId] == m_maxNumActionsPerSource ) {

    return (ERROR);

  }

  else {

    /* Insert pointer to the new action into the appropriate list.
     * This is safe to do without disabling the interrupt because
     * the interrrupt handler will not try to execute the new action
     * before the counter is incremented.
     */
    iAction = m_numActions[sourceId];

    m_pAction[sourceId][iAction] = pAction;

    m_numActions[sourceId]++;

  }


  return (OK);

}


STATUS CInterruptManager::removeInterruptAction(int sourceId, CAction* pAction)
{

  int   iAction;

  BOOL   matchIsFound;

  int   dpio2_intr_route_status = 0;



  /* Check that the parameters have valid values.
   */
  if ( (sourceId < 0) || (sourceId >= m_maxNumSources) ) {

    return (ERROR);

  } else {

    /* Search for an action object which is equal to the one specified.
     * If a match is found, remove it from the list.
     */
    iAction = 0;
    matchIsFound = FALSE;
    while ( !matchIsFound && (iAction < m_numActions[sourceId]) ) {

      if ( m_pAction[sourceId][iAction] == pAction ) {

        matchIsFound = TRUE;

      } else {

        iAction++;

      }

    }


    if ( !matchIsFound ) {

      return (ERROR);

    } else {

      /* The interrupt must be disabled while the action is removed.
       */
      dpio2_disable_route_intr_pci_save_atomic ( m_hDpio2,
						 &dpio2_intr_route_status );

      m_pAction[sourceId][iAction] = NULL;

      while ( (iAction + 1) < m_numActions[sourceId] ) {

        m_pAction[sourceId][iAction] = m_pAction[sourceId][iAction + 1];
        iAction++;

      }

      m_numActions[sourceId]--;


      dpio2_enable_route_intr_pci_save ( m_hDpio2,
					 dpio2_intr_route_status );
   
    }

  }


  return (OK);

}


void CInterruptManager::removeAllInterruptActions()
{

  int   iAction;

  int   iSource;

  int   dpio2_intr_route_status = 0;


  for (iSource = 0; iSource < m_maxNumSources; iSource++) {

    if ( m_pInterruptSource[iSource] != NULL ) {


      /* The interrupt must be disabled while the action is removed.
       */
      dpio2_disable_route_intr_pci_save_atomic ( m_hDpio2,
						 &dpio2_intr_route_status );


      for (iAction = 0; iAction < m_numActions[iSource]; iAction++) {

        m_pAction[iSource][iAction] = NULL;

      }

      m_numActions[iSource] = 0;

      dpio2_enable_route_intr_pci_save ( m_hDpio2,
					 dpio2_intr_route_status );
      
    }

  }

}



STATUS CInterruptManager::enableInterruptSource(int sourceId)
{

  STATUS   returnValue;


  /* Check that the specified interrupt source exists
   */
  if ( m_pInterruptSource[sourceId] == NULL ) {

    returnValue = ERROR;

  } else {

    m_pInterruptSource[sourceId]->enable();

    returnValue = OK;

  }

  return (returnValue);

}



STATUS CInterruptManager::disableInterruptSource(int sourceId)
{

  STATUS   returnValue;


  /* Check that the specified interrupt source exists
   */
  if ( m_pInterruptSource[sourceId] == NULL ) {

    returnValue = ERROR;

  } else {

    m_pInterruptSource[sourceId]->disable();

    returnValue = OK;

  }

  return (returnValue);

}


void CInterruptManager::disableAllInterruptSources()
{

  int   iSource;


  for (iSource = 0; iSource < m_maxNumSources; iSource++) {

    if ( m_pInterruptSource[iSource] != NULL ) {

      m_pInterruptSource[iSource]->disable();

    }

  }

}


STATUS CInterruptManager::checkIsInterruptSourceEnabled(int sourceId, BOOL& enabledFlag)
{

  STATUS   returnValue;


  /* Check that the specified interrupt source exists
   */
  if ( m_pInterruptSource[sourceId] == NULL ) {

    returnValue = ERROR;

  } else {

    enabledFlag = m_pInterruptSource[sourceId]->checkIsEnabled();

    returnValue = OK;

  }


  return (returnValue);

}


STATUS CInterruptManager::checkIsInterruptSourceActive(int sourceId, BOOL& activeFlag)
{

  STATUS   returnValue;


  /* Check that the specified interrupt source exists
   */
  if ( m_pInterruptSource[sourceId] == NULL ) {

    returnValue = ERROR;

  } else {

    activeFlag = m_pInterruptSource[sourceId]->checkIsActive();

    returnValue = OK;

  }


  return (returnValue);

}



CInterruptManager::CInterruptManager(HANDLE  hDpio2,
                                     int     maxNumSources,
                                     int     maxNumActionsPerSource)
{

  int   iSource;
  int   iAction;


  m_hDpio2 = hDpio2;

  m_maxNumSources = maxNumSources;
  m_maxNumActionsPerSource = maxNumActionsPerSource;

  m_numStatusRegisters = 0;
  m_pInterruptStatusRegister = NULL;

  /* Allocate and initialize array of pointers to interrupt sources.
   */
  m_pInterruptSource = new CInterruptSource*[maxNumSources];
  for (iSource = 0; iSource < maxNumSources; iSource++) {

    m_pInterruptSource[iSource] = NULL;

  }


  /* Allocate array of pointers to pointers to actions.
   */
  m_pAction = new CAction**[maxNumSources];

  /* Allocate and initialize an array of pointers to actions for each interrupt source.
   */
  for (iSource = 0; iSource < maxNumSources; iSource++) {

    m_pAction[iSource] = new CAction*[maxNumActionsPerSource];
    for (iAction = 0; iAction < maxNumActionsPerSource; iAction++) {

      m_pAction[iSource][iAction] = NULL;

    }

  }
  

  /* Allocate and initialize array of integers that counts 
   * the number of attached actions for each source.
   */
  m_numActions = new int[maxNumSources];
  for (iSource = 0; iSource < maxNumSources; iSource++) {

    m_numActions[iSource] = 0;

  }  

}



CInterruptManager::~CInterruptManager()
{
}



STATUS CInterruptManager::notifyAboutInterrupt()
{

  STATUS   returnValue;
  int      sourceId;
  int      iRegister;
  int      iAction;


#if 0
  DB (("notifyAboutInterrupt\n"));
#endif

  /* Search all interrupt status registers for a pending interrupt.
   */
  iRegister = 0;
  sourceId = (-1);

  while ( (iRegister < m_numStatusRegisters) && (sourceId == (-1)) ) {

    sourceId = m_pInterruptStatusRegister[iRegister]->getPendingInterruptSource();
    iRegister++;

  }


  /* If no pending interrupt was found,
   * return ERROR to signal that interrupt is not handled.
   */
  if ( sourceId == (-1) ) {

    returnValue = ERROR;

  }  else {


    /* Moved enable (m_pInterruptSource[sourceId]->clear())
       of new interrupt before execute of user define action.
       This is possible as long as the global interrupt mask
       is set as the last of operation done during interrupt
       handling.
    */
#ifdef INTR_ENABLE_BEFORE_EXE
    m_pInterruptSource[sourceId]->clear();
#endif

    /* Execute all its attached actions,
     * clear the interrupt, and return OK to signal that the interrupt is handled.
     */
    for (iAction = 0; iAction < m_numActions[sourceId]; iAction++) {

      m_pAction[sourceId][iAction]->execute();

    }

#ifndef INTR_ENABLE_BEFORE_EXE
    m_pInterruptSource[sourceId]->clear();
#endif

    returnValue = OK;

  }

  return (returnValue);

}



STATUS CInterruptManager::addInterruptSource(int sourceId,
					     CInterruptSource* pSource,
					     UINT32 statusRegisterOffset,
					     int statusBitNumber)
{

  STATUS   returnValue;


  /* Check that the parameters have valid values.
   */
  if ( (sourceId < 0) || (sourceId >= m_maxNumSources) ) {

    returnValue = ERROR;

  }

  /* Check that an interrupt source with the same ID isn't  previously added.
   */
  else if ( m_pInterruptSource[sourceId] != NULL ) {

    returnValue = ERROR;

  }


  /* Check that the specified status bit isn't attached to another interrupt source.
   */
  else if ( checkIsStatusBitAttached(statusRegisterOffset, statusBitNumber) ) {

    returnValue = ERROR;

  }

  else {

    m_pInterruptSource[sourceId] = pSource;

    attachStatusBit(sourceId, statusRegisterOffset, statusBitNumber);

    returnValue = OK;

  }


  return (returnValue);

}



BOOL CInterruptManager::checkIsStatusBitAttached(UINT32 statusRegisterOffset,
						 int bitNumber)
{

  BOOL     registerIsNotFound;
  BOOL     bitIsFound;
  int      iRegister;
  UINT32   bitMask;


  /* Search for the specified address in the list of existing status registers.
   */
  registerIsNotFound = TRUE;
  iRegister = 0;

  while ( registerIsNotFound && (iRegister < m_numStatusRegisters) ) {

    if ( m_pInterruptStatusRegister[iRegister]->getOffset() == statusRegisterOffset ) {

      registerIsNotFound = FALSE;

    } else {

      iRegister++;

    }
  }


  if ( registerIsNotFound ) {

    bitIsFound = FALSE;

  } else {

    /* Get the mask of attached status bits in the status register
     * to check if the specified status bit is already attached.
     */
    bitMask = m_pInterruptStatusRegister[iRegister]->getBitMask();

    if ( (bitMask & (0x00000001 << bitNumber)) == 0 ) {

      bitIsFound = FALSE;

    } else {

      bitIsFound = TRUE;

    }
  }


  return (bitIsFound);

}



void CInterruptManager::attachStatusBit(int sourceId,
					UINT32 statusRegisterOffset,
					int bitNumber)
{

  BOOL     registerIsNotFound;
  int      iRegister;


  CInterruptStatusRegister**   pNewRegisterPointerArray;


  /* Search for the specified address in the list of existing status registers.
   */
  registerIsNotFound = TRUE;
  iRegister = 0;

  while ( registerIsNotFound && (iRegister < m_numStatusRegisters) ) {

    if ( m_pInterruptStatusRegister[iRegister]->getOffset() == statusRegisterOffset ) {

      registerIsNotFound = FALSE;

    } else {

      iRegister++;

    }
  }


  /* If the register specified by the address parameter isn't found
   * in the status register list, expand the list by creating a new status register.
   */
  if ( registerIsNotFound ) {

    /* Allocate a new pointer array with room for an extra startus register.
     * If there is an existing pointer array, copy the pointers into the new pointer array
     * before deleting the old array.
     */
    pNewRegisterPointerArray = new CInterruptStatusRegister*[m_numStatusRegisters + 1];

    if ( m_numStatusRegisters > 0 ) {

      for (iRegister = 0; iRegister < m_numStatusRegisters; iRegister++) {

	pNewRegisterPointerArray[iRegister] = m_pInterruptStatusRegister[iRegister];

      }

      delete [] m_pInterruptStatusRegister;

    }

    m_pInterruptStatusRegister = pNewRegisterPointerArray;


    /* Create a new status register and put a pointer to it at the end of
     * the expanded pointer array.
     */
    m_pInterruptStatusRegister[iRegister] = 
      new CInterruptStatusRegister ( m_hDpio2, statusRegisterOffset );

    m_numStatusRegisters++; 

  }


  /* Attach the status bit to the specified interrupt source ID.
   */
  m_pInterruptStatusRegister[iRegister]->attachSourceToStatusBit(sourceId, bitNumber);

}



