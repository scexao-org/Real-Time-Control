/****************************************************************************
Module      : CDpio2Oscillator.h

Description : Header file declaring the CDpio2Oscillator class.
              A CDpio2Oscillator object represents the functionality
              for controlling the programmable oscillator on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
                
  01f,06jul2004,meh  Made changes necessary to compile file under VMOSA.
  01e,22jan2004,meh  Added 3state bit to the CDpio2OscillatorCY22150 class
  01d,16dec2003,meh  Added support fpr new Clock Generator CY22150.
                     Change getProgrammedFrequency to getFrequency.
                     Moved the function getFrequency to CDpio2Ocsillator.cpp
                     Changed CDpio2Oscillator to CDpio2OscillatorICD2053B.
  01c,18mar2002,nib  Updated file for use in a MCOS environment.
  01b,26oct2001,nib  Updated file for use in a Windows environment.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifndef CDPIO2OSCILLATOR_H_INCLUDED_C569C8E6
#define CDPIO2OSCILLATOR_H_INCLUDED_C569C8E6


#if defined(WIN32)

#include "CWControlBit.h"

#elif defined(MCOS)

#include "CMControlBit.h"

#elif defined(VMOSA)

#include "vmosaCControlBit.h"

#else

#include "CControlBit.h"

#endif


class CDpio2Oscillator
{
  public:
    virtual STATUS setFrequency(UINT32 requestedFrequency, 
                                UINT32& resultingFrequency) = 0;

    virtual UINT32 getFrequency() = 0;

};


class CDpio2OscillatorICD2053B : public CDpio2Oscillator
{
  public:
    CDpio2OscillatorICD2053B(const UINT32 referenceFrequency,
                             const CControlBit& programClockBit, 
                             const CControlBit& programDataBit);

    virtual STATUS setFrequency(UINT32 requestedFrequency, 
                                UINT32& resultingFrequency);

    virtual UINT32 getFrequency();

    STATUS setFrequency(UINT32 pCounter,
                        UINT32 dutyCycleAdjustUp,
                        UINT32 mux,
                        UINT32 qCounter,
                        UINT32 index);

  private:
    UINT32 m_referenceFrequency;

    UINT32 m_programmedFrequency;

    CControlBit m_bitProgramClock;

    CControlBit m_bitProgramData;


    inline void setControlWord(UINT32 word);
    inline void setProgramWord(UINT32 word);


};



/*meh*/

class CDpio2OscillatorCY22150 : public CDpio2Oscillator
{
  public:
    CDpio2OscillatorCY22150(const UINT32 referenceFrequency,
                            const CControlBit& programClockBit, 
                            const CControlBit& programDataBit,
                            const CControlBit& program3StateData);

    virtual STATUS setFrequency(UINT32 requestedFrequency, 
                                UINT32& resultingFrequency);

    virtual UINT32 getFrequency();


 private:
    UINT32 m_referenceFrequency;

    UINT32 m_programmedFrequency;

    CControlBit m_bitProgramClock;

    CControlBit m_bitProgramData;

    CControlBit m_bitProgram3StateData; /*meh*/

    /*meh*/  
    void CDpio2OscillatorCY22150::clockUpDown();

    void CDpio2OscillatorCY22150::startSignal();

    void CDpio2OscillatorCY22150::stopSignal();

    void CDpio2OscillatorCY22150::acknowlegdePulse();

    void CDpio2OscillatorCY22150::writeByte(UINT8 byte);

    void CDpio2OscillatorCY22150::writeDeviceId(UINT8 byte);

    void CDpio2OscillatorCY22150::setClockRegister(UINT8 deviceAdrs,
                                                   UINT8 registerAdrs,
                                                   UINT8 data);

    STATUS CDpio2OscillatorCY22150::setClockGeneratorRegisters
                                                      (UINT32  div,
                                                       UINT32  Qt,
                                                       UINT32  Pb,
                                                       UINT32  Po,
                                                       int     doNotUseVco);


    void CDpio2OscillatorCY22150::findBestParameters(UINT32   requestedFrequency,
                                                     UINT32&  resultingFrequency,
                                                     UINT32&  bestDiv,
                                                     UINT32&  bestQt,
                                                     UINT32&  bestPb,
                                                     UINT32&  bestPo,
                                                     int&       doNotUseVco);

#if 0 /*meh, debug*/
    void CDpio2OscillatorCY22150::findq (UINT32   requestedFrequency);

    void CDpio2OscillatorCY22150::calcf(uint32_t Ptmax,
                                        uint32_t Ptmin,
                                        uint32_t Qtmax,
                                        uint32_t showmax,
                                        uint32_t showmin);

    void CDpio2OscillatorCY22150::busmark(UINT32 arg);
#endif /*meh, debug*/

};



#endif /* CDPIO2OSCILLATOR_H_INCLUDED_C569C8E6 */
