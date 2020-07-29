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


Module      : vmosaCInterruptManager.h

Description : Header file declaring the CInterruptManager class.
              A CInterruptManager object is used to manage interrupts from
              a set of interrupt sources represented by CInterruptSource objects.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,28jun2004,meh  Ported from CInterruptManager.h

*****************************************************************************/

#ifndef CINTERRUPTMANAGER_H_INCLUDED
#define CINTERRUPTMANAGER_H_INCLUDED

#include "vmosa.h"

#include "CInterruptSource.h"
#include "vmosaCInterruptStatusRegister.h"


class CAction;


class CInterruptManager
{
  public:

    CInterruptManager(HANDLE  hDpio2,
                      int     maxNumSources, 
		      int     maxNumActionsPerSource);

    //## Deletes the objects in the interrupt source list and the action lists.
    virtual ~CInterruptManager();

    STATUS enableInterruptSource(int sourceId);
    STATUS disableInterruptSource(int sourceId);
    void disableAllInterruptSources();

    STATUS checkIsInterruptSourceEnabled(int sourceId, BOOL& enabledFlag);

    STATUS checkIsInterruptSourceActive(int sourceId, BOOL& activeFlag);

    //## Adds a copy of an action to the actionlist of the given interrupt
    //## source.
    //## 
    //## Returns ERROR if the 
    //## 
    //## NOTE:  Because the server copies the action, the caller is responsible
    //## for deleting the action object given as parameter to this method.
    STATUS addInterruptAction(int sourceId, CAction* pAction);
    STATUS removeInterruptAction(int sourceId, CAction* pAction);
    void removeAllInterruptActions();

    STATUS addInterruptSource(int               sourceId, 
			      CInterruptSource  *pSource,
			      UINT32            statusRegisterOffset, 
			      int               statusBitNumber);

    //## Notifies the interrupt manager that an interrupt that may have been
    //## caused by DPIO has occurred.
    STATUS notifyAboutInterrupt();


  protected:

    BOOL checkIsStatusBitAttached(UINT32 statusRegisterOffset, int bitNumber);

    void attachStatusBit(int sourceId, UINT32 statusRegisterOffset, int bitNumber);


  private:

    HANDLE  m_hDpio2;

    int m_maxNumSources;

    int m_maxNumActionsPerSource;

    int* m_numActions;

    int m_numStatusRegisters;

    int m_interruptLine;

    CAction** *m_pAction;

    CInterruptSource* *m_pInterruptSource;

    CInterruptStatusRegister* *m_pInterruptStatusRegister;

};



#endif /* CINTERRUPTMANAGER_H_INCLUDED */
