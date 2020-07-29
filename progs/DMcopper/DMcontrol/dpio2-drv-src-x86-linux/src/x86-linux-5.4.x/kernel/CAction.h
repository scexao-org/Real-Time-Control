/****************************************************************************
Module      : CAction.h

Description : Header file declaring the CAction class.
              CAction is an abstract class which defines the interface of
              all interrupt actions used in the DPIO2 Driver Core.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CACTION_H_INCLUDED_C569F07E
#define CACTION_H_INCLUDED_C569F07E

#ifdef VXWORKS
#include "vxWorks.h"
#endif


//##ModelId=3A6C2470008C
//##Documentation
//## Abstract class which defines the interface of action objects that the
//## InterruptServer uses.
//## 
//## NOTE: All classes derived from this one, must implement a virtual copy
//## constructor.
class CAction
{
  public:
    virtual ~CAction();

    //##ModelId=3A6C24CC02B2
    virtual void execute() = 0;

};



#endif /* CACTION_H_INCLUDED_C569F07E */
