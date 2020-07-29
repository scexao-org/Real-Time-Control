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


Module      : vmosaCPciDevice.h

Description : Header file declaring the CPciDevice class which represent
              functionality common to all PCI devices.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01a,07jul2004,meh  Ported from CPciDevice.h

*****************************************************************************/

#ifndef CPCIDEVICE_H_INCLUDED
#define CPCIDEVICE_H_INCLUDED


#define MAX_NUMBER_OF_BARS   (6)


class CPciDevice
{
  public:

    CPciDevice( HANDLE hDpio2 );

    CPciDevice(const CPciDevice& right);

    STATUS getVendorId(UINT16& id);

    STATUS getDeviceId(UINT16& id);

    STATUS getSubsystemId(UINT16& id);

    STATUS getSubsystemVendorId(UINT16& id);

    STATUS getRevisionId(UINT8& id);

    //## Checks whether the specified BAR register maps into PCI IO Space.
    //## 
    //## This method returns TRUE if the specified BAR register is mapping IO
    //## Space and FALSE otherwise.
    STATUS checkIsBarMappingIoSpace(int barNumber, BOOL& flag);

    //## Checks whether the region mapped by the specified BAR register is
    //## prefetchable.  Returns TRUE if it is, and FALSE otherwise.
    //## 
    //## This method throws an exception if the specified BAR does not exist or
    //## is mapping PCI IO Space.
    STATUS checkIsBarRegionPrefetchable(int barNumber, BOOL& flag);

    //## Returns the size of the region that a specified BAR maps.  The size is
    //## meassured in bytes.
    //## 
    //## This method throws an exception if the specified BAR register does not
    //## exist.
    STATUS getSizeOfBarRegion(int barNumber, UINT32& size);

    //##Documentation
    //## Sets the address where the specified BAR register should map in its
    //## region.
    //## 
    //## This method throws an exception if the specified BAR register does not
    //## exist.
    STATUS setBar(int barNumber, UINT32 pciAddress);

    STATUS getBar(int barNumber, UINT32& pciAddress);

    //## Returns the current setting of the Latency Timer.
    UINT8 getLatencyTimer();

    //## Sets the value of the Latency Timer.
    void setLatencyTimer(UINT8 timerValue);

    //## Enables the PCI Device to respond as target to accesses in IO Space.
    void enableAsIoSpaceTarget();

    //## Disables the PCI Device from responding as target to accesses in IO
    //## Space.
    void disableAsIoSpaceTarget();

    //## Checks whether the PCI device is enabled to respond as target to
    //## accesses in IO Space.  Returns TRUE if it is enabled, and FALSE
    //## otherwise.
    BOOL checkIsEnabledAsIoSpaceTarget();

    //## Enables the PCI Device to respond as target to accesses in Memory
    //## Space.
    void enableAsMemorySpaceTarget();

    //## Disables the PCI Device from responding as target to accesses in
    //## Memory Space.
    void disableAsMemorySpaceTarget();

    //## Checks whether the PCI device is enabled to respond as target to
    //## accesses in Memory Space.  Returns TRUE if it is enabled, and FALSE
    //## otherwise.
    BOOL checkIsEnabledAsMemorySpaceTarget();

    //##ModelId=3A9245A102DA
    //##Documentation
    //## Enables the PCI device to act as Bus Master.
    void enableAsMaster();

    //##ModelId=3A9245A103B6
    //##Documentation
    //## Disables the PCI device from acting as Bus Master.
    void disableAsMaster();

    //##ModelId=3A9245A200DC
    //##Documentation
    //## Checks whether the PCI device is enabled to act as Bus Master.
    //## Returns TRUE if enabled, and FALSE otherwise.
    BOOL checkIsEnabledAsMaster();


    BOOL checkHasMasterDataParityErrorOccurred();
    BOOL checkIsTargetAbortSignaled();
    BOOL checkIsTargetAbortReceived();
    BOOL checkIsMasterAbortReceived();
    BOOL checkIsSystemErrorSignaled();


    //## Returns the value of the Interrupt Pin Register.
    UINT8 getInterruptPin();

    //## Returns the value of the Interrupt Line Register.
    UINT8 getInterruptLine();

    //## Sets the value of the Interrupt Line Register.
    void setInterruptLine(UINT8 interruptLineValue);

    inline UINT8 getPciDeviceNumber();
    inline UINT8 getPciBusNumber();


  private:

    HANDLE m_hDpio2;

    int   m_aBarOffset[MAX_NUMBER_OF_BARS];

};


UINT8 CPciDevice::getPciDeviceNumber()
{

  return (m_deviceNumber);

}


UINT8 CPciDevice::getPciBusNumber()
{

  return (m_busNumber);

}

#endif /* CPCIDEVICE_H_INCLUDED_C569C934 */
