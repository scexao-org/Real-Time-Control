/****************************************************************************
Module      : CDpio2DmaDescriptor.h

Description : Header file declaring the CDpio2DmaDescriptor, 
              CDpio2InputDmaDescriptor, and CDpio2OutputDmaDescriptor classes.

              CDpio2DmaDescriptor is the super class which contains
              the attributes which are common to all DMA descriptors on DPIO2.
              Both CDpio2InputDmaDescriptor and CDpio2OutputDmaDescriptor
              has CDpio2DmaDescriptor as base class.

              In addition to the attributes inherited from CDpio2DmaDescriptor,
              CDpio2InputDmaDescriptor contains the extra attibutes which are
              required when the DPIO2 is configured for input.

              In the same way, CDpio2OutputDmaDescriptor contains the extra
              attributes required when the DPIO2 is configured for output.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01b,06jul2004.meh  Made changes necessary to compile file under VMOSA.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CDPIO2DMADESCRIPTOR_H
#define CDPIO2DMADESCRIPTOR_H

#ifdef VXWORKS
#include "vxWorks.h"
#endif

#ifdef VMOSA
#include "vmosa.h"
#endif


class CDpio2DmaDescriptor
{

public:

  UINT32 m_transferLength;
  UINT32 m_nextDescriptorId;
  BOOL m_flagGenerateDmaBlockEndInterrupt;
  BOOL m_flagEndOfDmaChain;
  BOOL m_flagUse64BitsDataAccess;

};


class CDpio2InputDmaDescriptor : public CDpio2DmaDescriptor
{

public:

  UINT32 m_destinationAddress;

};


class CDpio2OutputDmaDescriptor : public CDpio2DmaDescriptor
{

public:

  UINT32 m_sourceAddress;

  BOOL m_flagNotLastBlockInFrame;

};


#endif /* CDPIO2DMADESCRIPTOR_H */


