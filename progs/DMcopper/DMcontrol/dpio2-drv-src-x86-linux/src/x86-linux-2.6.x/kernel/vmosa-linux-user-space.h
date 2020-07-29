/****************************************************************************
Copyright Notice:
  This computer software is proprietary to VMETRO. The use of this software
  is governed by a licensing agreement. VMETRO retains all rights under
  the copyright laws of the United States of America and other countries.
  This software may not be furnished or disclosed to any third party and
  may not be copied or reproduced by any means, electronic, mechanical, or
  otherwise, in whole or in part, without specific authorization in writing
  from VMETRO.
 
    Copyright (c) 1996-2006 by VMETRO, ASA.  All Rights Reserved.


Module      : vmosa-linux-user-space.h

Description : Header file declaring linux user space in vmosa

Revision History:
  01a,26jul2004,meh  Created.

*****************************************************************************/

#ifndef VMOSA_LINUX_USER_SPACE_INCLUDED
#define VMOSA_LINUX_USER_SPACE_INCLUDED


#include "vmosa-linux.h"
#include "dpio2-linux-user-space.h"



#if 1
#ifdef __cplusplus
extern "C" {
#endif
#endif



/* Functions
 */

STATUS vmosa_reg_32_read ( HANDLE, UINT32, UINT32* );
STATUS vmosa_reg_32_write ( HANDLE, UINT32, UINT32 );
STATUS vmosa_reg_32_read_modify_write_atomic ( HANDLE, UINT32, UINT32, UINT32 );
STATUS vmosa_reg_32_bit_toggle_atomic ( HANDLE, UINT32, int );

STATUS vmosa_pci_config_8_read ( HANDLE, UINT8, UINT8* );
STATUS vmosa_pci_config_8_write ( HANDLE, UINT8, UINT8 );
STATUS vmosa_pci_config_16_read ( HANDLE, UINT8, UINT16* );

void   vmosa_mdelay ( UINT );
void   vmosa_udelay ( UINT );

STATUS vmosa_dpio2_dma_lock_memory ( dpio2_user_dma_page_t* );
STATUS vmosa_dpio2_dma_unlock_memory ( dpio2_user_dma_page_t* );


#if 1
#ifdef __cplusplus
}
#endif
#endif


#endif /* VMOSA_LINUX_USER_SPACE_INCLUDED */
