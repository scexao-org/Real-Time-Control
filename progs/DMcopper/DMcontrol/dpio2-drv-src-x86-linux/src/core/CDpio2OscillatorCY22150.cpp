/****************************************************************************
Module      : CDpio2Oscillator.cpp

Description : Source file defining the CDpio2Oscillator class.
              A CDpio2Oscillator object represents the functionality
              for controlling the programmable oscillator on a DPIO2 module.

Copyright(c): 2001 VMETRO asa. All Rights Reserved.

Revision History:
  01c,06jul2004.meh  Made changes necessary to compile file under VMOSA.
  01b,22jan2004,meh  Supporting Oscillator CY22150.
  01a,17des2003,meh  Created. Based on the old CDpio2Oscillator(ICD2053B).cpp

*****************************************************************************/

#ifdef VXWORKS
#include "taskLib.h"
#endif

#ifdef VMOSA
#include "vmosa.h"
#endif

#include "CDpio2Oscillator.h"
#include "DpioCommon.h"




#if 0 /*meh*/
#define MARKM printf ("Was Here %s,%d\n", __FILE__, __LINE__);
#endif





const int   NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD = 6;


int showbm = 1; /*meh*/ /*debug*/



CDpio2OscillatorCY22150::CDpio2OscillatorCY22150(const UINT32 referenceFrequency,
                                                 const CControlBit& programClockBit, 
                                                 const CControlBit& programDataBit,
                                                 const CControlBit& program3StateData)
  : m_bitProgramClock(programClockBit),
    m_bitProgramData(programDataBit),
    m_bitProgram3StateData(program3StateData)
{
  
  m_referenceFrequency = referenceFrequency;
  
  m_programmedFrequency = 0;
  
}




UINT32 CDpio2OscillatorCY22150::getFrequency()
{

  return (m_programmedFrequency);

}




STATUS CDpio2OscillatorCY22150::setFrequency(UINT32 requestedFrequency, 
                                             UINT32& resultingFrequency)
{

  UINT32  Qt = 0;
  UINT32  Pb = 0;
  UINT32  Po = 0;
  UINT32  div = 0;
  int     doNotUseVco = 0;



  /* find nearest/absolute frequency with most optimal/stable VCO parameters */

  findBestParameters(requestedFrequency,
                     resultingFrequency,
                     div,Qt,Pb,Po,doNotUseVco);


  setClockGeneratorRegisters (div,Qt,Pb,Po,doNotUseVco);

  m_programmedFrequency = resultingFrequency;


  return OK;

}




STATUS CDpio2OscillatorCY22150::setClockGeneratorRegisters(UINT32  div,
                                                           UINT32  Qt,
                                                           UINT32  Pb,
                                                           UINT32  Po,
                                                           int     doNotUseVco)
{

  const UINT8  DEVICE_ID_CY22150 = (0x69);
  
  const UINT8  REGISTER_ADRS_OUPUT_ENABLE = (0x09);
    const UINT8  ENABLE_OUTPUT_PIN_5 = (0x10);
    const UINT8  DISABLE_OUTPUT_PIN_5 = (0x0);
  const UINT8  REGISTER_ADRS_MUX_AND_DIVIDER_1 = (0x0c);
    const UINT8  USE_VCO = (0x00);
    const UINT8  USE_REF = (0x80);
  const UINT8  REGISTER_ADRS_EXTERNAL_CLOCK_REF = (0x12);
    const UINT8  USE_20MHZ_INPUT_CLOCK_REFERENCE = (0x20);
  const UINT8  REGISTER_ADRS_INPUT_LOAD_CAPACITOR = (0x13);
    const UINT8  STANDAR_SETTINGS_WITH_EXTERNAL_CLOCK = (0x00);
  const UINT8  REGISTER_ADRS_CHARGE_PUMP_AND_PB_HIGH = (0x40);
    const UINT8  STANDARD_SETTINGS_CHARGE_PUMP_AND_PB = (0xc0);
    const UINT8  CHARGE_PUMP_IF_PT_16_44 = (0x00);
    const UINT8  CHARGE_PUMP_IF_PT_44_479 = (0x04);
    const UINT8  CHARGE_PUMP_IF_PT_480_639 = (0x08);
    const UINT8  CHARGE_PUMP_IF_PT_640_799 = (0x0c);
    const UINT8  CHARGE_PUMP_IF_PT_800_1023 = (0x10);
  const UINT8  REGISTER_ADRS_PB_LOW = (0x41);
  const UINT8  REGISTER_ADRS_PO_AND_Q = (0x42);
    const UINT8  PO_1 = (0x80);
  const UINT8  REGISTER_ADRS_SWITCH_MATRIX_1 = (0x44);
    const UINT8  STANDARD_SETTINGS_SWITCH_MATRIX_1 = (0x0);
  const UINT8  REGISTER_ADRS_SWITCH_MATRIX_2 = (0x45);
    const UINT8  USE_EXTERNAL_CLOCK_REF = (0x0);
    const UINT8  USE_DIV1_N = (0x02); 
    const UINT8  USE_DIV1_2 = (0x04); /* set DIV_N to 4 */
    const UINT8  USE_DIV1_3 = (0x0c); /* set DIV_N to 6 */
  const UINT8  REGISTER_ADRS_SWITCH_MATRIX_3 = (0x46);
    const UINT8  STANDARD_SETTINGS_SWITCH_MATRIX_3 = (0x3f);
  const UINT8  REGISTER_ADRS_MUX_AND_DIVIDER_2 = (0x47);
    const UINT8  STANDARD_SETTINGS_MUX_AND_DIVIDER_2 = (0x08);

  
  UINT32    Pt;
  UINT8     buf8;



#if 0
  printf ("div = %d, Qt = %d, Pb = %d, Po = %d, doNotUseVco = %d\n",
          (int)div,(int)Qt,(int)Pb,(int)Po,(int)doNotUseVco);
#endif


  /* Set registers with parameters value 
   */



  /* Disable output pin # 5 */

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_OUPUT_ENABLE,
                    DISABLE_OUTPUT_PIN_5);
 

  /* Set 20MHz input clock referanc */

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_EXTERNAL_CLOCK_REF,
                    USE_20MHZ_INPUT_CLOCK_REFERENCE);


  /* Set Pb high and Charge Pump */

  buf8 = (Pb>>8);

  buf8 |= STANDARD_SETTINGS_CHARGE_PUMP_AND_PB;

  Pt = (2*(Pb+4))+Po;

  if (Pt < 45) {
    
    buf8 |= CHARGE_PUMP_IF_PT_16_44;

  } else if (Pt < 480) {

    buf8 |= CHARGE_PUMP_IF_PT_44_479;
    
  } else if (Pt < 640) {
    
    buf8 |= CHARGE_PUMP_IF_PT_480_639;

  } else if (Pt < 800) {

    buf8 |= CHARGE_PUMP_IF_PT_640_799;

  } else {

    buf8 |= CHARGE_PUMP_IF_PT_800_1023;

  }

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_CHARGE_PUMP_AND_PB_HIGH,
                    buf8);


  /* Set Pb low */

  buf8 = 0x000000FF & Pb;

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_PB_LOW,
                    buf8);


  /* Set Q and Po */

  buf8 = Qt - 2;

  if (Po)

    buf8 |= PO_1;

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_PO_AND_Q,
                    buf8);


  /* Set CapLoad register to delfault 0 */

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_INPUT_LOAD_CAPACITOR,
                    STANDAR_SETTINGS_WITH_EXTERNAL_CLOCK);



  /* Set switch matrix */

  if (div == 1) {

    buf8 = USE_EXTERNAL_CLOCK_REF;

  } else if (div == 2) {

    buf8 = USE_DIV1_2;

  } else if (div == 3) {

    buf8 = USE_DIV1_3;

  } else {
    
    buf8 = USE_DIV1_N;

  }

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_SWITCH_MATRIX_2,
                    buf8);


  /* Set DIVIDER 1 and mux 1 */

  if (div <= 2) {

    buf8 = 0x04;

  } else if (div == 3) {

    buf8 = 0x06;

  } else {
    
    buf8 = 0x7F & div;

  }

  if (doNotUseVco) 

    buf8 |= USE_REF;

  else

    buf8 |= USE_VCO;


  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_MUX_AND_DIVIDER_1,
                    buf8);


  /* Set the other switch register to "off" */

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_SWITCH_MATRIX_1,
                    STANDARD_SETTINGS_SWITCH_MATRIX_1);

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_SWITCH_MATRIX_3,
                    STANDARD_SETTINGS_SWITCH_MATRIX_3);

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_MUX_AND_DIVIDER_2,
                    STANDARD_SETTINGS_MUX_AND_DIVIDER_2);


  /* Enable output pin # 5 */

  setClockRegister (DEVICE_ID_CY22150,
                    REGISTER_ADRS_OUPUT_ENABLE,
                    ENABLE_OUTPUT_PIN_5);



  return OK;


}










/* Give one clock puls to the Programmable Oscillator CY22150
   with half periode greater then 1,25us = 400kHz 
*/
void CDpio2OscillatorCY22150::clockUpDown()
{

  int i;


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD/2; i++)
    
    m_bitProgramClock.clear();


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++)

    m_bitProgramClock.set();


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD/2; i++)

    m_bitProgramClock.clear();


}



/* Give start signal to the Programmable Oscillator CY22150 
 */
void CDpio2OscillatorCY22150::startSignal()
{

  int i;


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD/2; i++) {

    m_bitProgramData.set();
    m_bitProgramClock.set();

  }
 

  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {
  
    m_bitProgramData.clear();
    m_bitProgramClock.set();
  
  }
  

  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD/2; i++) {
   
    m_bitProgramData.clear();
    m_bitProgramClock.clear();
  
  }

}



/* Give stop signal to the Programmable Oscillator CY22150 
 */
void CDpio2OscillatorCY22150::stopSignal()
{
  int i;


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD/2; i++) {

    m_bitProgramData.clear();
    m_bitProgramClock.clear();

  }


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD; i++) {

    m_bitProgramData.clear();
    m_bitProgramClock.set();

  }  


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD*4; i++) {

    m_bitProgramData.set();
    m_bitProgramClock.set();

  }

}




/* Set 3 state bit on the output while waiting for acknowledge puls from
   the Programmable Oscillator CY22150 
*/
void CDpio2OscillatorCY22150::acknowlegdePulse()
{
  int  i;

  /* CSR(19) = 1 ==> data output in 3state mode */   

  m_bitProgram3StateData.set();

  
  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD*2; i++)

    m_bitProgramData.clear();
  

  clockUpDown();


  /* CSR(19) = 0 ==> data outout in normal mode */  

  m_bitProgram3StateData.clear();


  for (i=0; i<NUM_REPETIONS_TO_ENSURE_MINIMUM_CLOCK_PERIOD*2; i++)

    m_bitProgramData.clear();


  
}



/* Write a byte
 */
void CDpio2OscillatorCY22150::writeByte(UINT8 byte)
{
  UINT8   bitMask = 0x80;

  int     i;



  for (i=0; i<8; i++) {
    

    if (bitMask & byte) {
      
      m_bitProgramData.set();

    } else {

      m_bitProgramData.clear();

    }


    clockUpDown();

    bitMask>>=1;


  }


  /* Output low before acknowledge puls */

  acknowlegdePulse();


}




/* Write a Device ID
 */
void CDpio2OscillatorCY22150::writeDeviceId(UINT8 byte)
{

  UINT8   bitMask = 0x40;
  
  int     i;



  /* Write device ID */

  for (i=0; i<7; i++) {
    
    if (bitMask & byte) {
      
      m_bitProgramData.set();

    } else {

      m_bitProgramData.clear();

    }


    clockUpDown();

    bitMask>>=1;


  }


  /* Write Read/Write command Bit */

  if (0x80 & byte) {
    
    m_bitProgramData.set();
    
  } else {

    m_bitProgramData.clear();

  }
  
  
  clockUpDown();



  /* Output low before acknowledge puls */

  acknowlegdePulse();


}





/* Program one of the registes in the Programmable Clock CY22150
 */
void CDpio2OscillatorCY22150::setClockRegister(UINT8 deviceAdrs,
                                               UINT8 registerAdrs,
                                               UINT8 data)
{
  
  const UINT8  I2C_WRITE = (0);


#if 0 /*win*/
  taskDelay (1);
#endif

  /* Give start signal/sequnece */

  startSignal();


  /* Write device adrs */

  writeDeviceId (deviceAdrs | I2C_WRITE);


  /* Write register adrs */

  writeByte (registerAdrs);


  /* Write data to register */

  writeByte (data);


  /* Give stop signal/sequnece */

  stopSignal();


}





void CDpio2OscillatorCY22150::findBestParameters(UINT32   requestedFrequency,
                                                 UINT32&  resultingFrequency,
                                                 UINT32&  bestDiv,
                                                 UINT32&  bestQt,
                                                 UINT32&  bestPb,
                                                 UINT32&  bestPo,
                                                 int&     doNotUseVco)

{


  const UINT32 MEGA                      = (1000*1000);
  const UINT32 KILO                      = 1000;
  const UINT32 DIVIVER_MIN               = 2;
  const UINT32 DIVIVER_MAX               = 127;
  const UINT32 Qt_MIN                    = 2;
  const UINT32 Qt_MAX                    = 80;
  const UINT32 Pt_MIN                    = 16;
  const UINT32 Pt_MAX                    = 1023;
  const UINT32 REFERANCE_CLOCK_FREQUENCY = (20*MEGA);
  const UINT32 MIN_GENERATET_FREQUENCY   = (500*KILO);
  const UINT32 MAX_GENERATET_FREQUENCY   = (150*MEGA);
  const UINT32 MIN_VCO_FREQUENCY         = (100*MEGA);
  const UINT32 MAX_VCO_FREQUENCY         = (400*MEGA);


  UINT32  Qt;
  UINT32  Pt = 0;
  UINT32  vco;
  UINT32  Fgen = 0;
  UINT32  Fref;
  UINT32  Freq;
  UINT32  div = 0;

  int     bestDiff = 99999999; /* high num */
  int     tmpDiff;
  UINT32  bestPt = 0;

  UINT32  lowerLimit;
  UINT32  higherLimit;

  UINT32  Qt_start;
  UINT32  div_start;



  /* Check for decided frequency limit */

  Fref = REFERANCE_CLOCK_FREQUENCY;

  Freq = requestedFrequency;

  if (Freq < MIN_GENERATET_FREQUENCY) {
    
    Freq = MIN_GENERATET_FREQUENCY;

  }

  
  if (Freq > MAX_GENERATET_FREQUENCY) {

    Freq = MAX_GENERATET_FREQUENCY;

  }



#if 0 
  printf ("\nRequested Frequency = %d MHZ, %d kHz, %d Hz\n",
          (int)(Freq/MEGA),(int)(Freq/KILO),(int)Freq);
#endif


  doNotUseVco = 1; /* init to 1 */
  bestDiv = 0;
  bestQt = 999999;
  bestPb = 0;
  bestPo = 0;

  
  lowerLimit = Freq - 10000;
  higherLimit = Freq + 20000;
  
  
  if (lowerLimit < MIN_GENERATET_FREQUENCY)
    
    lowerLimit = MIN_GENERATET_FREQUENCY;
  
  
  if (higherLimit > MAX_GENERATET_FREQUENCY)
    
    higherLimit = MAX_GENERATET_FREQUENCY;
  



  /* Check if use of VCO is possible */

  if (Freq < (MIN_VCO_FREQUENCY / DIVIVER_MAX))

    goto not_using_vco;


  /* Calculate all intresting result freqency and store the best result */

  for (Pt = Pt_MIN; Pt <= Pt_MAX; Pt++) {

    
    /* find smallest Qt which give VCO <= MAX_VCO_FREQUENCY */

    if (Pt >= ((UINT32)0xffffffff / Fref)) /* uint32_t overflow */
    
      Qt_start = (UINT32)0xffffffff / MAX_VCO_FREQUENCY;  
    
    else
    
      Qt_start = (Fref * Pt) / MAX_VCO_FREQUENCY;
    
    if (Qt_start < Qt_MIN)
      
      Qt_start = Qt_MIN;

    if (Qt_start > Qt_MAX)
      
      Qt_start = Qt_MAX;


    for (Qt = Qt_start; Qt <= Qt_MAX; Qt++) {

      
      /* Calculate VCO frequency */
      
      if (Pt >= ((UINT32)0xffffffff / Fref))

        vco = (Fref / Qt) * Pt;

      else

        vco = (Fref * Pt) / Qt;


      /* If VCO frequency < 100MHz then the vco calculated in the next iteration 
         will be even lower ==> break*/
      
      if (vco < MIN_VCO_FREQUENCY)
        
        break;      

      
      /* If the VCO frequency is <= to 400MHz 
         then check for best result frequency */
      
      if (vco <= MAX_VCO_FREQUENCY) {

        
        /* Calc best divider start value */

        div_start = vco / higherLimit;

        if (div_start < DIVIVER_MIN)

          div_start = DIVIVER_MIN;          


        for (div = div_start; div <= DIVIVER_MAX; div++) {
          

          Fgen = vco / div;


          /* If calulated result frequency is lower then decided then the next 
             calculater resulting frequency will be even lower ==> break */

          if (Fgen < lowerLimit)

            break;


          /* If calulated result frequency is below decided limit,
             check for best result frequency */
          
          if (Fgen <= higherLimit) {

 
            tmpDiff = (int)(Fgen - Freq);
            

            if (tmpDiff < 0)

              tmpDiff *= -1;


            /* If smaller frequency differance 
               or smaller Qt with the same frequency differance
               ==> new best candidate */

            if ( (bestDiff > tmpDiff) ||
                 ((bestDiff == tmpDiff) && (bestQt > Qt)) ) {
              

              bestDiff = tmpDiff;
              
              bestDiv = div;

              bestQt = Qt;

              bestPt = Pt;

              doNotUseVco = 0;


            } /* (bestDiff > tmpDiff) ||
                 ((bestDiff == tmpDiff) && (bestQt > Qt)) */

          } /* Fgen <= higherLimit */


        } /* (div = DIVIVER_MIN; div <= DIVIVER_MAX; div++) */


      } /* (vco <= MAX_VCO_FREQUENCY) */


    } /* (Qt = Qt_MIN; Qt <= Qt_MAX; Qt++) */
    
    
  } /* (Pt = Pt_MIN; Pt <= Pt_MAX; Pt++) */
  

  
  /* Calculate Po and Pb */
  
  if (doNotUseVco == 0) {
    

    if (bestPt % 2) {
      
       bestPo = 1;
       
       bestPb = ((bestPt - 1) / 2) - 4;
       
    } else {
      
      bestPo = 0;
      
      bestPb = (bestPt / 2) - 4;
      
    }

    
  } /* (doNotUseVco == 0) */
      
      
      
      
 not_using_vco: /* Label */


  /* Check if not using VCO gives a better resulting frequency */
  
  if (bestDiff &&  (Freq < 10*MEGA)) {


    for (div = DIVIVER_MIN; div <= DIVIVER_MAX; div++) {


      Fgen = Fref / div;


      /* check if diff is better then for VCO */
      
      tmpDiff = (int)(Fgen - Freq);
      
      if ( tmpDiff < 0)
        
        tmpDiff *= -1;
      
      
      
      if (bestDiff > tmpDiff) {


        bestDiff = tmpDiff;

        bestDiv = div;

        bestQt = 0;

        bestPb = 0;

        bestPo = 0;

        doNotUseVco = 1;
        
        
      } /* (bestDiff > tmpDiff) */

      
    } /* (i=DIVIVER_MIN; i<=DIVIVER_MAX; i++) */
    

  } /* (Fdiff &&  (Freq < 10*MEGA)) */



  if (doNotUseVco) {

    Fgen = Fref / bestDiv;


  } else {

    if (bestPt >= ((UINT32)0xffffffff / Fref)) /* uint32_t overflow */
      
      Fgen = ( (Fref / bestQt) * ((2*(bestPb+4)) + bestPo) ) / bestDiv;
       
    else
      
      Fgen = ((Fref * ((2*(bestPb+4)) + bestPo) ) / bestQt) / bestDiv;
    
  } /* (doNotUseVco) */


  resultingFrequency = Fgen;


  tmpDiff = (int)(Fgen - Freq);
      
  if (tmpDiff < 0)
        
    tmpDiff *= -1;
  

#if 0
  printf ("Resulting Frequency = %d Hz, Difference %d Hz (%6.4f prosent)\n",
          (int)resultingFrequency, tmpDiff,
          (double)( ((double)tmpDiff * 100.0) / (double)Freq ) );
#endif 

}












#if 0 /* debug function */


void CDpio2OscillatorCY22150::busmark (UINT32 arg)
{
  if (showbm)
    pciConfigOutLong (1,5,0,0,arg);
}  




void CDpio2OscillatorCY22150::findq (UINT32   requestedFrequency)
{
  UINT32  resultingFrequency = 0;
  UINT32  bestDiv;
  UINT32  bestQt;
  UINT32  bestPb;
  UINT32  bestPo;
  int     doNotUseVco;


  printf ("\nRequested Frequency = %d MHZ, %d kHz, %d Hz\n",
          (int)(Freq/MEGA),(int)(Freq/KILO),(int)Freq);


  findBestParameters(requestedFrequency,
                     resultingFrequency,
                     bestDiv,
                     bestQt,
                     bestPb,
                     bestPo,
                     doNotUseVco);
}



#define MEGA (1000*1000)
#define KILO 1000


void CDpio2OscillatorCY22150::calcf(UINT32 Ptmax,
                                    UINT32 Ptmin,
                                    UINT32 Qtmax,
                                    UINT32 showmax,
                                    UINT32 showmin)
{
  UINT32  Qt;
  UINT32  Pt;
  UINT32  vco;
  UINT32  Fref;
  UINT32* buf;
  UINT32* sort; 
  UINT32  div;

  UINT32  biggest;

  UINT32 i=0;
  UINT32 n=0;
  UINT32 f=0;
  UINT32 g=0;
  UINT32 pos=0;

  double VCO;



  if (Qtmax < 2)
    Qtmax = 2;

  if (Qtmax>80)
    Qtmax=80;

  if (Ptmax>1023)
    Ptmax=1023;

  if (Ptmin<16)
    Ptmin=16;


  printf ("Ptmin %d,Ptmax %d,Qtmax %d, showmax %d showmin %d\n",
          (int)Ptmin,(int)Ptmax,(int)Qtmax,(int)showmax, (int) showmin);


  buf = (UINT32*) malloc (0x00100000);
  sort =(UINT32*) malloc (0x00100000);

  bzero ((char*) buf,0x00100000);
  bzero ((char*) sort,0x00100000);

  Fref = 20 * MEGA;


  printf ("Calculating...\n");

  for (Pt=Ptmax; Pt>=Ptmin; Pt--) {
    
    for (Qt=2; Qt<=Qtmax; Qt++) {
      
      if (Pt>212)
        vco = (Fref/Qt)*Pt;
      else
        vco = (Fref * Pt) / Qt;
#if 0       
      printf ("vco = %d    div=%d Pb=%d Po=%d Qt=%d\n",
              (int)vco,(int)div,(int)Pb,(int)Po,(int)Qt);
#endif   
      if ( vco<(100*MEGA))
        break;      

      if (vco <= (400*MEGA)) {
        
        for (div=2; div<=127; div++) {
          
          if ((vco/div) < showmin)
            break;
          
          if ( ((vco/div) >= showmin) && ((vco/div) <= showmax) ) {
            i++;
            buf[i] = vco / div;
#if 0    
            printf ("F = %d\n", (int)buf[i]);
#endif
          }
        }
      }
    }
  }
  
  
  

  printf ("\nSorting (#%d)...\n",i);

  /* sortere */
  for (n=0,g=0; n<=i; n++) {
    

    biggest = 0;
    for (f=0;f<=i; f++) {
      if (buf[f] > biggest) {
        biggest = buf[f];
        pos = f;
      }
    }
    
    if (n==0) {
      sort[g] = biggest;
      g++;
    }

    if ((n>1) && (biggest < sort[g-1])) {
      sort[g] = biggest;
      g++;
    }
    buf[pos] = 0;

  }

  /* print */

  printf ("Printing (#%d)..\n",g);
  for (n=0; n<g; n++) {

    if ((sort[n]>showmin) && (sort[n]<showmax)) {

      printf ("frekvens   %3d MHz  %6d kiloHz  %d\n",
              (int) (sort[n]/1000000), (int)(sort[n]/1000),(int) sort[n]);
    }

  }


  free (buf);
  free (sort);

}

#endif

