/*---------------------------------------------------------------------------
Copyright Notice:
  This computer software is proprietary to VMETRO. The use of this software
  is governed by a licensing agreement. VMETRO retains all rights under
  the copyright laws of the United States of America and other countries.
  This software may not be furnished or disclosed to any third party and
  may not be copied or reproduced by any means, electronic, mechanical, or
  otherwise, in whole or in part, without specific authorization in writing
  from VMETRO.
 
    Copyright (c) 1997, 1998 by VMETRO, Inc.  All Rights Reserved.
 
Description:

    Contain err module number for DPIO driver.  See "USAGE" below.

$Id: dpioErrMod.h,v 1.1.1.1 2001/07/12 08:54:01 cvs Exp $

MODIFICATION HISTORY:
01a,24jan97 bqv written

$Log: dpioErrMod.h,v $
Revision 1.1.1.1  2001/07/12 08:54:01  cvs
Source from 1.0 Beta A

Revision 1.2  1998/02/19 22:58:43  jbaker
Changed to use standar VMETRO banner.

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
USAGE:
This file is meant to be included in $(WIND_BASE)/target/h/vwModNum.h.
You must:
    Link or copy this file to $(WIND_BASE)/target/h.
    Modify $(WIND_BASE)/target/h/vwModNum.h to include this file.

Make sure the Module number of 8000 does not conflict with any module
already in $(WIND_BASE)/target/h/vwModNum.h. If there is a conflict,
you can change the 8000 to something else.

SYSTEM DEPENDENCIES:
    VxWorks
---------------------------------------------------------------------------*/

#ifndef _DPIO_ERR_MOD_H_
#define _DPIO_ERR_MOD_H_

#define M_dpio  (8000 << 16)

#endif
