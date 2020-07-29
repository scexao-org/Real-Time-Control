/****************************************************************************
Module      : CDpio2Oscillator.cpp

Description : Source file defining the CDpio2Oscillator class.
              A CDpio2Oscillator object represents the functionality
              for controlling the programmable oscillator on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01f,06jul2004.meh  Made changes necessary to compile file under VMOSA.
  01e,16dec2003,meh  Change getProgrammedFrequency to getFrequency.
                     Moved the function getFrequency from CDpio2Ocsillator.h
                     Changed all CDpio2Oscillator to CDpio2OscillatorICD2053B.
  01d,01oct2003,meh  Changed ERROR_MSG to DPIO2_ERROR_MSG.
  01c,15may2003,nib  Added repeated clearing and setting of clock bit
                     in setControlWord() and setProgramWord() to ensure
                     minimum clock period.
  01b,18mar2002,nib  Updated file for use in a MCOS environment.
  01a,22jun2001,nib  Created.

*****************************************************************************/

#ifdef VXWORKS
#include "taskLib.h"
#endif

#ifdef VMOSA
#include "vmosa.h"
#endif

#include "CDpio2Oscillator.h"
#include "DpioCommon.h"


const int   NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD = 10;


//##ModelId=3A912764035C
CDpio2OscillatorICD2053B::CDpio2OscillatorICD2053B(const UINT32 referenceFrequency,
                                                   const CControlBit& programClockBit, 
                                                   const CControlBit& programDataBit)
  : m_bitProgramClock(programClockBit),
    m_bitProgramData(programDataBit)
{

  m_referenceFrequency = referenceFrequency;

  m_programmedFrequency = 0;

}



#if 1 /*meh*/ /* Moeved from .h file because of virituel function */
UINT32 CDpio2OscillatorICD2053B::getFrequency()
{

  return (m_programmedFrequency);

}
#endif /*meh*/



//##ModelId=3A8B89D80384
STATUS CDpio2OscillatorICD2053B::setFrequency(UINT32 requestedFrequency, 
                                              UINT32& resultingFrequency)
{

  const UINT32   MIN_VCO_FREQUENCY = (50*1000*1000);
  const UINT32   INDEX_LIMIT_VCO_FREQUENCY = (80*1000*1000);
  const UINT32   MAX_VCO_FREQUENCY = (150*1000*1000);

  const UINT32   MIN_REFERENCE_FREQUENCY_SCALED_BY_Q = (200 * 1000);
  const UINT32   MAX_REFERENCE_FREQUENCY_SCALED_BY_Q = (1 * 1000 * 1000);

  const UINT32   MIN_OUT_FREQUENCY = 3125;
  const UINT32   MAX_OUT_FREQUENCY = (100*1000*1000);

  const UINT32   MIN_Q_COUNTER_VALUE = 1;
  const UINT32   MAX_Q_COUNTER_VALUE = 127;

  const UINT32   MIN_P_COUNTER_VALUE = 1;
  const UINT32   MAX_P_COUNTER_VALUE = 127;

  const UINT32   MAX_MUX_VALUE = 7;


  STATUS   status;

  UINT32   counterP;
  UINT32   counterQ;
  UINT32   mux;

  UINT32   maxCounterQ;
  UINT32   minCounterQ;

  UINT32   bestCounterP = 0;
  UINT32   bestCounterQ = 0;
  UINT32   bestMux = 0;
  UINT32   bestIndex = 0;

  UINT32   vcoFrequency;
  UINT32   generatedFrequency;
  UINT32   deviation;

  UINT32   bestDeviation;
  UINT32   bestResultingFrequency = 0;


  status = OK;

  /* Check that the parameters are valid
   */
  if ( requestedFrequency < MIN_OUT_FREQUENCY ) {

    DPIO2_ERROR_MSG(("Requested frequency (%d Hz) is lower than the minimum (%d Hz).\n",
               (int) requestedFrequency, (int) MIN_OUT_FREQUENCY));

    status = ERROR;

  }

  if ( requestedFrequency > MAX_OUT_FREQUENCY ) {

    DPIO2_ERROR_MSG(("Requested frequency (%d Hz) is higher than the maximum (%d Hz).\n",
               (int) requestedFrequency, (int) MAX_OUT_FREQUENCY));

    status = ERROR;

  }


  if ( status == OK ) {

    /* Find the lowest valid VCO frequency (in the range between 50MHz and 150MHz).
     */
    mux = 0;
    vcoFrequency = requestedFrequency;

    while ( (vcoFrequency < MIN_VCO_FREQUENCY) && (mux <= MAX_MUX_VALUE) ) {
      
      vcoFrequency *= 2;
      mux++;

    }

    DEBUG_MSG(("Min. VCO frequency = %d MHz\n", (int) vcoFrequency), FALSE);


    /* Set best deviation to the requested frequency to be sure that
     * all computed values will be better.
     */
    bestDeviation = requestedFrequency;


    /* Try to compute parameters for all applicable VCO frequencies.
     */
    while ( vcoFrequency <= MAX_VCO_FREQUENCY ) {

      /* Compute the maximum and minimum values that can be used for Q counter.
       */
      maxCounterQ = m_referenceFrequency / MIN_REFERENCE_FREQUENCY_SCALED_BY_Q;
      if ( maxCounterQ > MAX_Q_COUNTER_VALUE ) {
        
        maxCounterQ = MAX_Q_COUNTER_VALUE;

      }

      minCounterQ = ( (m_referenceFrequency + (MAX_REFERENCE_FREQUENCY_SCALED_BY_Q - 1)) 
                      / MAX_REFERENCE_FREQUENCY_SCALED_BY_Q);
      if ( minCounterQ < MIN_Q_COUNTER_VALUE ) {
        
        minCounterQ = MIN_Q_COUNTER_VALUE;

      }


      /* Go through all valid values for Q counter and compute the value for P counter
       * that results in the VCO frequency closest to the desired frequency.
       */
      for (counterQ = minCounterQ; counterQ <= maxCounterQ; counterQ++) {

        /* When computing the value for P counter, the reference frequency is added
         * in the nominator in order to round the result.
         * To avoid overflow the follwing formula is used:
         *
         *   ((A * B) + C) / D = A * (B / D) + ((A * (B % D) + C) / D)
         */
        counterP = 
          ((counterQ + 2) * (vcoFrequency / (2 * m_referenceFrequency)))
          + (((counterQ + 2) * (vcoFrequency % (2 * m_referenceFrequency)) + m_referenceFrequency)
             / (2 * m_referenceFrequency)) - 3;

        if ( counterP < MIN_P_COUNTER_VALUE ) {

          counterP = MIN_P_COUNTER_VALUE;

        } else if ( counterP > MAX_P_COUNTER_VALUE ) {

          counterP = MAX_P_COUNTER_VALUE;

        }
      
        /* Compute the frequency that will be generated and the deviation
         * from the requested frequency.
         * To avoid overflow the follwing formula is used:
         *
         *   (A * B) / C = A * (B / C) + (A * (B % C)) / C
         */
        generatedFrequency = 
          ((counterP + 3) * ((2 * m_referenceFrequency) / ((counterQ + 2) * (1 << mux))))
          + (((counterP + 3) * ((2 * m_referenceFrequency) % ((counterQ + 2) * (1 << mux))))
             / ((counterQ + 2) * (1 << mux)));

        if ( generatedFrequency < requestedFrequency ) {

          deviation = requestedFrequency - generatedFrequency;

        } else {

          deviation = generatedFrequency - requestedFrequency;

        }

        if ( deviation < bestDeviation ) {

          bestMux = mux;
          bestCounterQ = counterQ;
          bestCounterP = counterP;
          bestIndex = (vcoFrequency <= INDEX_LIMIT_VCO_FREQUENCY) ? 0 : 8;

          bestDeviation = deviation;
          bestResultingFrequency = generatedFrequency;

          DEBUG_MSG(("Q=%3d, P=%3d, M=%d --> %8dHz\n", 
                     (int) (counterQ + 2), 
                     (int) (counterP + 3), 
                     (int) mux, 
                     (int) generatedFrequency), FALSE);

        }

      }

      vcoFrequency *= 2;
      mux++;
  
    }

    DEBUG_MSG(("Best P Counter Value: %d\n", (int) (bestCounterP + 3)), FALSE);
    DEBUG_MSG(("Best Q Counter Value: %d\n", (int) (bestCounterQ + 2)), FALSE);
    DEBUG_MSG(("Best Mux value      : %d\n", (int) bestMux), FALSE);
    DEBUG_MSG(("Best Index value    : %d\n", (int) bestIndex), FALSE);

    DEBUG_MSG(("Deviation           : %dHz\n", (int) bestDeviation), FALSE);

    setFrequency(bestCounterP, 1, bestMux, bestCounterQ, bestIndex);


    /* Return and store the resulting frequency.
     */
    m_programmedFrequency = bestResultingFrequency;
    resultingFrequency = bestResultingFrequency;

  }


  return (status);

}



//##ModelId=3ADFF1C003DF
STATUS CDpio2OscillatorICD2053B::setFrequency(UINT32 counterP,
                                              UINT32 dutyCycleAdjustUp,
                                              UINT32 mux,
                                              UINT32 counterQ,
                                              UINT32 index)
{

  STATUS   status;

  UINT32   programmingWord;


  /* Check that the parameters are valid.
   */
  status = OK;

  if ( (counterP < 1) || (counterP > 127) ) {

    DPIO2_ERROR_MSG(("Invalid P Counter value (%d)\n", (int) counterP));
    status = ERROR;

  }

  if ( (dutyCycleAdjustUp != 0) && (dutyCycleAdjustUp != 1) ) {

    DPIO2_ERROR_MSG(("Invalid Duty Cycle Adjust Up value (%d)\n", (int) dutyCycleAdjustUp));
    status = ERROR;

  }
              
  if ( mux > 7 ) {

    DPIO2_ERROR_MSG(("Invalid Mux value (%d)\n", (int) mux));
    status = ERROR;

  }

  if ( (counterQ < 1) || (counterQ > 127) ) {

    DPIO2_ERROR_MSG(("Invalid Q Counter value (%d)\n", (int) counterQ));
    status = ERROR;

  }

  if ( (index != 0) && (index != 8) ) {

    DPIO2_ERROR_MSG(("Invalid Index value (%d)\n", (int) index));
    status = ERROR;

  }


  /* Program frequency if all parameters specifying it are valid.
   */
  if ( status == OK ) {

    /* Load the Control Register to enable MUXREF and enable loading
     * of the Program Register. This will set the output to the reference
     * frequency. The transition is guaranteed to be glitch-free.
     */
    setControlWord(0x00000005);


    /* Shift in the desired output frequency value computed 
     * via a 22-bit data word, plus any bit-stuffs.
     */
    programmingWord 
      = (counterP << 15) | (dutyCycleAdjustUp << 14) | (mux << 11) | (counterQ << 4) | index;

    setProgramWord(programmingWord);


    /* Load the Control Register to enable MUXREF and disable
     * loading of the Program register. This loads the Program
     * word bits into the Program register and keeps the output
     * set to the reference frequency while the new frequency
     * settles.
     */
    setControlWord(0x00000004);

    
    /* Wait for VCO to settle in the new state.
     */
#ifdef VXWORKS

    taskDelay(7);

#elif defined(WIN32)

    Sleep(100);

#elif defined(MCOS)

    {

      struct timespec   delay = {0, 100*1000*1000};

      nanosleep(&delay, NULL);

    }

#elif (defined VMOSA)

  vmosa_mdelay (1);

#else
#error "Delay is not implemented"
#endif    

    
    /* Load the Control register to enable new frequency output.
     * The transition is guaranteed to be glitch-free.
     */
    setControlWord(0x00000000);

  }


  return (status);

}


//##ModelId=3ADFF1C10065
void CDpio2OscillatorICD2053B::setControlWord(UINT32 word)
{

  int   cBit;

  int   i;


  /* Add the Protocol Word.
   */
  word &= 0x000000FF;
  word |= 0x00001E00;


  /* Shift out the word, starting with LSB.
   */
  DEBUG_MSG(("Shifting out control word: "), FALSE);
  for (cBit = 0; cBit < 14; cBit++) {

    /* Set the serial data bit.
     */
    if ( (word & 0x00000001) == 0x00000001 ) {

      m_bitProgramData.set();

      DEBUG_MSG(("1"), FALSE);

    } else {

      m_bitProgramData.clear();

      DEBUG_MSG(("0"), FALSE);

    }

    /* Make a low-to-high transition on the serial clock bit.
     * Repeat both clearing and setting to ensure that the clock
     * signal is low for at least 450ns and high for at least 450ns.
     */
    for (i = 0; i < NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {

      m_bitProgramClock.clear();

    }
    for (i = 0; i < NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {

      m_bitProgramClock.set();

    }

    word >>= 1;

  }

  DEBUG_MSG(("\n"), FALSE);

}


//##ModelId=3ADFF1C100A0
void CDpio2OscillatorICD2053B::setProgramWord(UINT32 word)
{

  int   cBit;
  int   numSequentialOnes;

  int   i;


  numSequentialOnes = 0;

  /* Shift out the word, starting with LSB.
   */
  DEBUG_MSG(("Shifting out program word: "), FALSE);
  for (cBit = 0; cBit < 22; cBit++) {

    /* Set the serial data bit.
     */
    if ( (word & 0x00000001) == 0x00000001 ) {

      m_bitProgramData.set();
      numSequentialOnes++;

      DEBUG_MSG(("1"), FALSE);

    } else {

      m_bitProgramData.clear();
      numSequentialOnes = 0;

      DEBUG_MSG(("0"), FALSE);
    }


    /* Make a low-to-high transition on the serial clock bit.
     * Repeat both clearing and setting to ensure that the clock
     * signal is low for at least 450ns and high for at least 450ns.
     */
    for (i = 0; i < NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {

      m_bitProgramClock.clear();

    }
    for (i = 0; i < NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {

      m_bitProgramClock.set();

    }


    /* If three sequential 1s have been sent, add a stuff-bit.
     */
    if ( numSequentialOnes == 3 ) {

      m_bitProgramData.clear();
      
      DEBUG_MSG(("0"), FALSE);

      /* Make a low-to-high transition on the serial clock bit.
       * Repeat both clearing and setting to ensure that the clock
       * signal is low for at least 450ns and high for at least 450ns.
       */
      for (i = 0; i < NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {

        m_bitProgramClock.clear();

      }
      for (i = 0; i < NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {

        m_bitProgramClock.set();

      }

      numSequentialOnes = 0;

    }

    word >>= 1;

  }

  DEBUG_MSG(("\n"), FALSE);

}























