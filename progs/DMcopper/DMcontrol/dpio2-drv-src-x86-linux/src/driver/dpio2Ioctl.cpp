/*---------------------------------------------------------------------------
Copyright Notice:
  This computer software is proprietary to VMETRO. The use of this software
  is governed by a licensing agreement. VMETRO retains all rights under
  the copyright laws of the United States of America and other countries.
  This software may not be furnished or disclosed to any third party and
  may not be copied or reproduced by any means, electronic, mechanical, or
  otherwise, in whole or in part, without specific authorization in writing
  from VMETRO.
 
    Copyright (c) 2001 by VMETRO, ASA.  All Rights Reserved.
 
Description:

     Device driver for DPIO2 offering two APIs; one similar to the device driver 
     for DPIO and one providing the new functionality of the DPIO2.


MODIFICATION HISTORY:
01j,08jul2004,meh  Made changes necessary to compile file under VMOSA.
01i,13feb2004,meh  Support new Strobe Frequency Range.
01h,01oct2003,meh  Changed ERROR_MSG to DPIO2_ERROR_MSG.
                   Changed the DEBUG_MSG with TRUE flag to
                   DPIO2_WARNING_MSG.
01g,02oct2002,nib  Renamed TTL and PECL strobe to primary and secondary strobe.
01f,17sep2002,nib  Added DPIO2_CMD_EOT_COUNT_ENABLE 
                   and DPIO2_CMD_EOT_COUNT_DISABLE.
01e,13sep2002,nib  Added DPIO2_CMD_TRANSFERRED_BYTE_COUNT_GET.
01d,21mar2002,nib  Updated to compile in an MCOS environment.
01c,15feb2002,nib  Added the command DPIO2_CMD_FIFO_FLUSH to the DPIO API.
01b,12feb2002,nib  Added the command DPIO2_CMD_SYNC_GENERATION_COUNTER_SET.
01a,23apr2001,nib  Created from dpio2.cpp to extract the ioctl functions
                   which are used in both Windows and VxWorks.

---------------------------------------------------------------------------*/

/* includes */
#include "dpio2Ioctl.h"

#include "DpioDefs.h"

#include "dpioErr.h"


/* defines */

#define  DPIO2_CMD_DVALID_ALWAYS_ASSERTED_SELECT  5000
#define  DPIO2_CMD_GET_PTR_FLASH                  5001

/*                                                                                           rsr
* Declare overloaded fuction prototype for dpio2Ioctl                                        rsr
*/                                                                                         //rsr
STATUS dpio2Ioctl(int devno, int command, unsigned long argument);                         //rsr


/* typedefs */

/* Structure for information about an interrupt actions.
 * Used by dpio2InitInterruptActions() to store the default interrupt
 * action settings.
 */
struct InterruptActionInfo {

  BOOL (CDpio2DriverController::*pConditionMethod)();
  BOOL condition;

};



/* imports */


/* globals */


/* locals */
/* Array of constant structures used by dpio2InitInterruptActions
 * initialize interrupt actions.
 */
const struct InterruptActionInfo   dpio2ApiActionInfo[DPIO2_INT_CONDITIONS] =
{
  /* DPIO2_INT_COND_DMA_DONE_CHAIN */
  {&CDpio2DriverController::checkIsDmaTransferCompleted, TRUE},

  /* DPIO2_INT_COND_DMA_DONE_BLOCK */
  {&CDpio2DriverController::checkIsDmaTransferCompleted, FALSE},

  /* DPIO2_INT_COND_FIFO_NOT_EMPTY */
  {NULL, FALSE},

  /* DPIO2_INT_COND_FIFO_EMPTY */
  {NULL, FALSE},

  /* DPIO2_INT_COND_FIFO_ALMOST_EMPTY */
  {NULL, FALSE},

  /* DPIO2_INT_COND_FIFO_HALF_FULL */
  {NULL, FALSE},

  /* DPIO2_INT_COND_FIFO_ALMOST_FULL */
  {NULL, FALSE},

  /* DPIO2_INT_COND_FIFO_FULL */
  {NULL, FALSE},

  /* DPIO2_INT_COND_FIFO_OVERFLOW */
  {NULL, FALSE},

  /* DPIO2_INT_COND_PIO1 */
  {NULL, FALSE},

  /* DPIO2_INT_COND_PIO2 */
  {NULL, FALSE},

  /* DPIO2_INT_COND_SYNC */
  {NULL, FALSE},

  /* DPIO2_INT_COND_SUSPEND */
  {NULL, FALSE},

  /* DPIO2_INT_COND_TARGET_ABORT */
  {&CDpio2DriverController::checkIsTargetAbortReceived, TRUE},

  /* DPIO2_INT_COND_MASTER_ABORT */
  {&CDpio2DriverController::checkIsMasterAbortReceived, TRUE},

};


#if defined(VXWORKS) || defined(WIN32)

const struct InterruptActionInfo   dpioApiActionInfo[DPIO_INT_CONDITIONS] =
{
  /* DPIO_INT_COND_DMA_DONE_CHAIN */
  {&CDpio2DriverController::checkIsDmaTransferCompleted, TRUE},

  /* DPIO_INT_COND_DMA_DONE_BLOCK */
  {&CDpio2DriverController::checkIsDmaTransferCompleted, FALSE},

  /* DPIO_INT_COND_FIFO_NOT_EMPTY */
  {NULL, FALSE},

  /* DPIO_INT_COND_FIFO_EMPTY */
  {NULL, FALSE},

  /* DPIO_INT_COND_FIFO_HALF_FULL */
  {NULL, FALSE},

  /* DPIO_INT_COND_FIFO_FULL */
  {NULL, FALSE},

  /* DPIO_INT_COND_PIO1 */
  {NULL, FALSE},

  /* DPIO_INT_COND_PIO2 */
  {NULL, FALSE},

  /* DPIO_INT_COND_FIFO */
  {NULL, FALSE},

  /* DPIO_INT_COND_TARGET_ABORT */
  {&CDpio2DriverController::checkIsTargetAbortReceived, TRUE},

  /* DPIO_INT_COND_MASTER_ABORT */
  {&CDpio2DriverController::checkIsMasterAbortReceived, TRUE},

};
#endif /* VXWORKS || WIN32 */


static FUNCPTR   pHaltFunction = NULL;


/* forward static declarations */
#if defined(VXWORKS) || defined(WIN32)
static STATUS dpio2DmaStart(DPIO2_DPIO_API_DEVICE* pDevice, 
                            DPIO_DMA_START_DESC* pStartDescriptor);

static UINT8 buildDpioInterruptControlRegister(CDpio2DriverController* pController);
static UINT8 buildDpioFifoStatusRegister(CDpio2DriverController* pController);
static UINT8 buildDpioPioStatusRegister(CDpio2DriverController* pController);
static UINT8 buildDpioInterruptStatusRegister(CDpio2DriverController* pController);
static UINT8 buildDpioModuleIdRegister(CDpio2DriverController* pController);
#endif /* VXWORKS || WIN32 */



#if defined(VXWORKS) || defined(WIN32)
/******************************************************************************
* dpio2DpioApiInitHardware - Initialize the DPIO2 hardware like a DPIO
*
* RETURNS: OK if successfull, ERROR if not.
* 
*/
static STATUS dpio2DpioApiInitHardware(DPIO2_DPIO_API_DEVICE* pDevice)
{

  UINT8   personalityModuleId;
  UINT8   mainBoardId;

  int   moduleType;


  /* Determine the DPIO device type.
   */ 
  personalityModuleId = pDevice->pController->getPersonalityId();
  mainBoardId = pDevice->pController->getMainBoardId();

  moduleType = (mainBoardId << 4) | personalityModuleId;


  /* Check that the module is a valid predefined device types 
   */
  if ( (moduleType != DPIO2_FB_MODULE) &&
       (moduleType != DPIO2_LB_MODULE) ) {

    DPIO2_ERROR_MSG(("DPIO2 type 0x%02x wasn't Input or Output Module.\n", moduleType));
    errno = S_dpio_DPIO_PCI_ERR_INVALID_DEVICE_TYPE;
    return (ERROR);

  }

  
  /* Change DMA read command from Mem Read Line to Mem Read Multiple 
   */  
  pDevice->pController->setDmaPciReadCommand(PCI_MEMORY_READ_MULTIPLE_COMMAND);


  /* Enable FIFO "occurred" flags 
   */
  pDevice->pController->resetFifoHistoryFlags();


  return OK;

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
DPIO2_INTERRUPT_SOURCE_ID mapDpioConditionToSource(int condition)
{

  switch (condition) {

  case DPIO_INT_COND_DMA_DONE_CHAIN:
    return (DPIO2_DMA_DONE_INTERRUPT);

  case DPIO_INT_COND_DMA_DONE_BLOCK:
    return (DPIO2_DMA_DONE_INTERRUPT);

  case DPIO_INT_COND_FIFO_NOT_EMPTY:
    return (DPIO2_FIFO_NOT_EMPTY_INTERRUPT);

  case DPIO_INT_COND_FIFO_EMPTY:
    return (DPIO2_FIFO_EMPTY_INTERRUPT);

  case DPIO_INT_COND_FIFO_HALF_FULL:
    return (DPIO2_FIFO_HALF_FULL_INTERRUPT);

  case DPIO_INT_COND_FIFO_FULL:
    return (DPIO2_FIFO_FULL_INTERRUPT);

  case DPIO_INT_COND_PIO1:
    return (DPIO2_PIO1_ASSERTED_INTERRUPT);

  case DPIO_INT_COND_PIO2:
    return (DPIO2_PIO2_ASSERTED_INTERRUPT);

  case DPIO_INT_COND_TARGET_ABORT:
    return (DPIO2_DMA_ERROR_INTERRUPT);

  case DPIO_INT_COND_MASTER_ABORT:
    return (DPIO2_DMA_ERROR_INTERRUPT);

  case DPIO_INT_COND_FIFO:
    /* Return the reserved invalid ID to signal that 
     * this interrupt is not implemented on DPIO2.
     */
    return (DPIO2_INVALID_INTERRUPT_SOURCE_ID);

  default:
    throw;
  }

}
#endif /* VXWORKS || WIN32 */


DPIO2_INTERRUPT_SOURCE_ID mapDpio2ConditionToSource(int condition)
{













  switch (condition) {

  case DPIO2_INT_COND_DMA_DONE_CHAIN:
    return (DPIO2_DMA_DONE_INTERRUPT);

  case DPIO2_INT_COND_DMA_DONE_BLOCK:
    return (DPIO2_DMA_DONE_INTERRUPT);

  case DPIO2_INT_COND_FIFO_NOT_EMPTY:
    return (DPIO2_FIFO_NOT_EMPTY_INTERRUPT);

  case DPIO2_INT_COND_FIFO_EMPTY:
    return (DPIO2_FIFO_EMPTY_INTERRUPT);

  case DPIO2_INT_COND_FIFO_ALMOST_EMPTY:
    return (DPIO2_FIFO_ALMOST_EMPTY_INTERRUPT);

  case DPIO2_INT_COND_FIFO_HALF_FULL:
    return (DPIO2_FIFO_HALF_FULL_INTERRUPT);

  case DPIO2_INT_COND_FIFO_ALMOST_FULL:
    return (DPIO2_FIFO_ALMOST_FULL_INTERRUPT);

  case DPIO2_INT_COND_FIFO_FULL:
    return (DPIO2_FIFO_FULL_INTERRUPT);

  case DPIO2_INT_COND_FIFO_OVERFLOW:
    return (DPIO2_FIFO_OVERFLOW_INTERRUPT);

  case DPIO2_INT_COND_PIO1:
    return (DPIO2_PIO1_ASSERTED_INTERRUPT);

  case DPIO2_INT_COND_PIO2:
    return (DPIO2_PIO2_ASSERTED_INTERRUPT);

  case DPIO2_INT_COND_SYNC:
    return (DPIO2_SYNC_ASSERTED_INTERRUPT);

  case DPIO2_INT_COND_SUSPEND:
    return (DPIO2_SUSPEND_ASSERTED_INTERRUPT);

  case DPIO2_INT_COND_TARGET_ABORT:
    return (DPIO2_DMA_ERROR_INTERRUPT);

  case DPIO2_INT_COND_MASTER_ABORT:
    return (DPIO2_DMA_ERROR_INTERRUPT);

  default:
    throw;
  }

}


/******************************************************************************
* dpio2DmaStart
* 
* Start the DMA controller. Description on how to start is given in the
* DPIO_DMA_START_DESC structure referenced by the second pointer argument. 
* If this pointer argument is NULL, the the controller is started without 
* setting any registers at first.
* 
* RETURNS: OK if successfull, ERROR if not.
* 
* ERRNO:
* DPIOErr_INVALID_ARG - Invalid argument
*/
#if defined(VXWORKS) || defined(WIN32)
static STATUS dpio2DmaStart(DPIO2_DPIO_API_DEVICE* pDevice, DPIO_DMA_START_DESC* pStartDescriptor)
{

  STATUS   status;


  if ( pStartDescriptor != NULL ) {

    /* Raise interrupt at End of transfer.
     */
    if ( pStartDescriptor->intEndOfChain ) {

      DEBUG_MSG(("Functionality for interrupt at end of DMA chain is not implemented\n"), FALSE);

    }

    /* Check that the given descriptor ID is valid
     * before setting it as start of the DMA chain.
     */
    if ( pStartDescriptor->startDescID == 0 ) {

      DPIO2_ERROR_MSG(("The specified DMA Block Descriptor ID (%d) is not valid.\n", 
                 pStartDescriptor->startDescID));
      errno = S_dpio_DPIO_DMA_ERR_INVALID_ARG;
      return ERROR;

    }

    status = pDevice->pController->setNextDescriptorInDmaChain(pStartDescriptor->startDescID);
    if ( status != OK ) {

      DPIO2_ERROR_MSG(("The specified DMA Block Descriptor ID (%d) is not valid.\n", 
                 pStartDescriptor->startDescID));
      errno = S_dpio_DPIO_DMA_ERR_INVALID_ARG;
      return ERROR;
    }

  }


  /* Kick off the DMA transfer and halt the processor if enabled for.
   */
  pDevice->pController->startDmaTransfer();

  if (pDevice->enableHalt) {

    if ( pHaltFunction != NULL ) {

      (*pHaltFunction)();

    }

  }


  return OK;

}
#endif /* VXWORKS || WIN32 */


/******************************************************************************
* dpio2Ioctl -  DPIO2 driver control routine
*               Eventually this version of dpio2Ioctl will handle                                              //rsr
*               only those commands requiring an integer argument                                              //rsr
*
* DPIO2 driver control routine
*
* The following commands are accepted and handled by this function:
*
* DPIO2_CMD_STROBE_GENERATION_ENABLE:
*
*   Enables Strobe Generation and consequently disables Strobe Reception.
*   This command also selects whether the fixed or the programable oscillator 
*   should be used, through the integer value given as argument:
*
*      ioctl(fd, DPIO2_CMD_STROBE_GENERATION_ENABLE, <oscillator>);
*
*   One of the following values must be used for <oscillator>:
*
*      DPIO2_PROGRAMABLE_OSCILLATOR
*      DPIO2_FIXED_OSCILLATOR
*
*   Releated commands:
*
*     DPIO2_CMD_STROBE_RECEPTION_ENABLE
*     DPIO2_CMD_STROBE_FREQUENCY_SET
*
*
* DPIO2_CMD_STROBE_RECEPTION_ENABLE:
*
*   Enables Strobe Reception and consequently disables Strobe Generation.
*   This command also selects the strobe source to be used, through 
*   the integer value given as argument:
*
*      ioctl(fd, DPIO2_CMD_STROBE_RECEPTION_ENABLE, <strobe-signal>);
*
*   One of the following values must be used for <strobe-signal>:
*
*      DPIO2_PRIMARY_STROBE
*      DPIO2_SECONDARY_STROBE
*
*   For DPIO2 modules with FPDP personality, DPIO2_PRIMARY_STROBE selects
*   the TTL strobe signal and DPIO2_SECONDARY_STROBE selects the PECL strobe.
*   For DPIO2 modules with LVDS, PECL, and RS422 personality, DPIO2_PRIMARY_STROBE
*   is the only valid option, DPIO2_SECONDARY_STROBE cannot be used.
*   
*
*   Releated commands:
*
*     DPIO2_CMD_STROBE_GENERATION_ENABLE
*     DPIO2_CMD_STROBE_FREQUENCY_RANGE_SET
*
*
* DPIO2_CMD_STROBE_FREQUENCY_SET:
*
*   Sets the frequency of the FPDP strobe.
*   Expects a pointer to a variable of type UINT32 as argument.
*   When the command is issued, this variable must be set to 
*   the wanted frequency in Hz.  Upon return the variable will contain 
*   the closest obtainable frequency which the strobe frequency is actually set to:
*
*      UINT32   frequencyInHz = <wanted-frequency>;
*
*      status = ioctl(fd, DPIO2_CMD_STROBE_FREQUENCY_SET, &frequencyInHz);
*
*   The DPIO2 module must have been set to generate strobe using
*   the programmable oscillator, for this command to have any effect.
*
*   Releated commands:
*
*     DPIO2_CMD_STROBE_GENERATION_ENABLE
*
*
* DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT:
* 
*   Selects whether to enable clocking on both strobe edges.
*   Expects a boolean value as argument and enables clocking
*   on both edges if this value is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT, TRUE);
*
*   If the boolean arguement is FALSE, clocking on both strobe edges 
*   is disabled:
*
*
*      status = ioctl(fd, DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT, FALSE);
*
*
* DPIO2_CMD_STROBE_FREQUENCY_RANGE_SET:
*
*   Informs the driver about the frequency range the FPDP strobe lies in.
*   Expects a integer value as argument, which identifies the frequency range
*   being used:
*
*      status = ioctl(fd, DPIO2_CMD_STROBE_FREQUENCY_RANGE_SET, <frequency-range>);
*
*   One of the following values must be used for <frequency-range>:
*
*      DPIO2_STROBE_FREQUENCY_BELOW_OR_EQUAL_TO_5MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_10MHZ_BELOW_OR_EQUAL_TO_15MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_15MHZ_BELOW_OR_EQUAL_TO_20MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_20MHZ_BELOW_OR_EQUAL_TO_25MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_25MHZ_BELOW_OR_EQUAL_TO_30MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_30MHZ_BELOW_OR_EQUAL_TO_35MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_35MHZ_BELOW_OR_EQUAL_TO_40MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_40MHZ_BELOW_OR_EQUAL_TO_45MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_45MHZ_BELOW_OR_EQUAL_TO_50MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_50MHZ_BELOW_OR_EQUAL_TO_55MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_55MHZ_BELOW_OR_EQUAL_TO_60MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_60MHZ_BELOW_OR_EQUAL_TO_65MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_65MHZ_BELOW_OR_EQUAL_TO_70MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_70MHZ_BELOW_OR_EQUAL_TO_75MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_75MHZ_BELOW_OR_EQUAL_TO_80MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_80MHZ_BELOW_OR_EQUAL_TO_85MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_85MHZ_BELOW_OR_EQUAL_TO_90MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_90MHZ_BELOW_OR_EQUAL_TO_95MHZ,
*      DPIO2_STROBE_FREQUENCY_ABOVE_95MHZ_BELOW_OR_EQUAL_TO_100MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_100MHZ
*
*  For dpio2 pcb B and dpio2_66 pcb A and older versions the following
*  values is til legal.
*
*      DPIO2_STROBE_FREQUENCY_BELOW_OR_EQUAL_TO_5MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_5MHZ_BELOW_OR_EQUAL_TO_10MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_10MHZ_BELOW_OR_EQUAL_TO_25MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_25MHZ_BELOW_OR_EQUAL_TO_50MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_50MHZ_BELOW_OR_EQUAL_TO_100MHZ
*      DPIO2_STROBE_FREQUENCY_ABOVE_100MHZ
*   
*   This command should be issued when the DPIO2 is receiving the FPDP strobe
*   and when the DPIO2 is generating the strobe based on a fixed oscillator.
*
*
* DPIO2_CMD_FPDP_ACTIVATION_SELECT:
*
*   Selects whether the DPIO2 module shall activate its FPDP interface or not.
*   Expects a value of type BOOL as argument, and activates the interface 
*   if the argument is TRUE:
*
*      ioctl(fd, DPIO2_CMD_FPDP_ACTIVATION_SELECT, TRUE);
*
*   If the argument is FALSE the FPDP interface is deactivated:
*
*      ioctl(fd, DPIO2_CMD_FPDP_ACTIVATION_SELECT, FALSE);
*
*
* DPIO2_CMD_SYNC_GENERATION_COUNTER_SET:
*
*   Sets the value of the counter which is used to generate SYNC if
*   the DPIO2_SYNC_GENERATION_ON_COUNT is selected by the command
*   DPIO2_CMD_SYNC_GENERATION_SELECT.
*
*   If the counter is set to N, SYNC will be asserted for every Nth word 
*   transferred.  The minimum valid counter value is 2.
*
*   This command expects an unsigned long integer as argument:
*
*      unsigned long   counterValue;
*
*      status = ioctl(fd, DPIO2_CMD_SYNC_GENERATION_COUNTER_SET, (int) counterValue);
*
*   Related Commands:
*
*      DPIO2_CMD_SYNC_GENERATION_SELECT
*
*
* DPIO2_CMD_SYNC_GENERATION_SELECT:
*
*   Selects how a DPIO2 configured as output shall generate SYNC.
*   Expects a integer as argument, and configures the SYNC generation
*   according to the value of this function:
*
*      status = ioctl(fd, DPIO2_CMD_SYNC_GENERATION_SELECT, <syncGenerationMode>);
*
*   The argument <syncGenerationMode> must have one of the following values:
*
*      DPIO2_SYNC_GENERATION_DISABLED
*      DPIO2_SYNC_GENERATION_BEFORE_DATA
*      DPIO2_SYNC_GENERATION_AT_END_OF_FRAME
*      DPIO2_SYNC_GENERATION_AT_START_OF_FRAME
*      DPIO2_SYNC_GENERATION_ON_ODD_FRAME
*      DPIO2_SYNC_GENERATION_ON_COUNT
*
*   If the DPIO2_SYNC_GENERATION_ON_COUNT mode is selected, the 
*   DPIO2_CMD_SYNC_GENERATION_COUNTER_SET command should be called to specify
*   a counter value.
*
*   In order to have effect, this command must be issued before the FPDP
*   interface is activated (see command DPIO2_CMD_FPDP_ACTIVATION_SELECT).
*
*   Related Commands:
*
*      DPIO2_CMD_SYNC_GENERATION_COUNTER_SET
*
*
* DPIO2_CMD_SYNC_RECEPTION_SELECT:
*
*   Selects how a DPIO2 configured as input shall handle received SYNC.
*   Expects an integer as argument, and configures the SYNC reception
*   according to the value of this function:
*
*      status = ioctl(fd, DPIO2_CMD_SYNC_RECEPTION_SELECT, <syncReceptionMode>);
*
*   The argument <syncReceptionMode> must have one of the following values:
*
*      DPIO2_SYNC_RECEPTION_DISABLED
*      DPIO2_SYNC_RECEPTION_STARTS_DATA_RECEPTION
*
*   In order to have effect, this command must be issued before the FPDP
*   interface is activated (see command DPIO2_CMD_FPDP_ACTIVATION_SELECT).
*
*
* DPIO2_CMD_D0_TO_BE_USED_FOR_SYNC_SELECT:
*
*   Selects whether data bit 0 (D0) shall be used to handle SYNC or not.
*   If D0 is selected to handle SYNC on a DPIO2 module configured for output,
*   D0 is routed to the SYNC signal.  The opposite will happen on modules 
*   configured for input; the SYNC signal is routed to D0.
*   The command expects a value of type BOOL as argument, and enables the routing
*   if this parameter is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_D0_TO_BE_USED_FOR_SYNC_SELECT, TRUE);
*
*   If the parameter is FALSE, D0 will not be used to handle SYNC:
*
*      status = ioctl(fd, DPIO2_CMD_D0_TO_BE_USED_FOR_SYNC_SELECT, FALSE);
*
*   Related commands:
*
*     DPIO2_CMD_SYNC_GENERATION_SELECT
*     DPIO2_CMD_SYNC_RECEPTION_SELECT
*
*
* DPIO2_CMD_VIDEO_MODE_SELECT:
*
*   Selects whether Video Mode shall be enabled or not.
*   Expects a value of type BOOL as argument, and enables Video Mode if
*   this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_VIDEO_MODE_SELECT, TRUE);
*
*   If the argument is FALSE, Video Mode is disabled:
*
*      status = ioctl(fd, DPIO2_CMD_VIDEO_MODE_SELECT, FALSE);
*
*   To have effect this command must be issued before the FPDP interface
*   is activated (see DPIO2_CMD_FPDP_ACTIVATION_SELECT).
*
*   This command is only applicable if the DPIO2 is configured as input.
*
*
* DPIO2_CMD_COUNTER_ADDRESSING_ENABLE
*
*   Enables Counter Addressing on a DPIO2 module configured as input.
*   Expects a pointer to a DPIO2_COUNTER_ADDRESSING_INFO structure as argument:
*
*      DPIO2_COUNTER_ADDRESSING_INFO   addressingInfo;
*
*      status = ioctl(fd, DPIO2_CMD_COUNTER_ADDRESSING_ENABLE, &addressingInfo);
*
*   The DPIO2_COUNTER_ADDRESSING_INFO structure has the following fields:
*
*      initialSkipCount - unsigned integer specifying how many words the DPIO2 module
*                         shall ignore immediately after the FPDP interface is activated
*
*      skipCount - unsigned integer specifying how many words the DPIO2 shall ignore
*                  after it has received a specified amount of data 
*
*      receiveCount - unsigned integer specifying how many words the DPIO2 module
*                     shall receive after skipping the number of words specified
*                     by 'skipCount' or 'initialSkipCount'
*
*   NOTE: In order to have effect this command must be called before the 
*         FPDP interface is activated (DPIO2_CMD_FPDP_ACTIVATION_SELECT). 
*
*   Related Commands:
*
*      DPIO2_CMD_COUNTER_ADDRESSING_DISABLE
*
*
* DPIO2_CMD_COUNTER_ADDRESSING_DISABLE:
*
*   Disables Counter Addressing on a DPIO2 module configured as input.
*   This command requires no arguments:
*
*      status = ioctl(fd, DPIO2_CMD_COUNTER_ADDRESSING_DISABLE, 0);
*
*   NOTE: In order to have effect this command must be called before the 
*         FPDP interface is activated (DPIO2_CMD_FPDP_ACTIVATION_SELECT). 
*
*   Related Commands:
*
*      DPIO2_CMD_COUNTER_ADDRESSING_ENABLE
*
*
* DPIO2_CMD_DATA_SWAP_MODE_SELECT:
*
*   Selects how a DPIO2 shall swap 8 bit, 16 bit, and 32 bit data.
*   Expects a integer as argument, and configures the data swap mode
*   according to the value of this integer:
*
*      status = ioctl(fd, DPIO2_CMD_DATA_SWAP_MODE_SELECT, <dataSwapMode>);
*
*   The argument <dataSwapMode> must have one of the following values:
*
*      DPIO2_NO_SWAP
*      DPIO2_8BIT_SWAP
*      DPIO2_16BIT_SWAP
*      DPIO2_8BIT_16BIT_SWAP
*      DPIO2_32BIT_SWAP
*      DPIO2_8BIT_32BIT_SWAP
*      DPIO2_16BIT_32BIT_SWAP
*      DPIO2_8BIT_16BIT_32BIT_SWAP
*
*   NOTE: 32 bit swapping will only have effect for DMA transfers using D64 accesses.
*
*
* DPIO2_CMD_DATA_PACKING_CAPABILITY_GET:
*
*   Tells which data packing modes are available on a DPIO2 module.
*   The command expects a pointer to an integer variable as argument, and returns 
*   a value in this variable which represents the available packing modes:
*
*      int   packingCapability
* 
*      status = ioctl(fd, DPIO2_CMD_DATA_PACKING_CAPABILITY_GET, (int) &packingCapability);
*
*   The returned value will have one of the following values:
*
*      DPIO2_8BIT_4BIT_PACKING_AVAILABLE
*      DPIO2_16BIT_10BIT_PACKING_AVAILABLE
*
*   Releated commands:
*
*     DPIO2_CMD_DATA_PACKING_ENABLE
*     DPIO2_CMD_DATA_PACKING_DISABLE
*
*
* DPIO2_CMD_DATA_PACKING_ENABLE:
*
*   Enables unpacking from 32 bits words on DPIO2 modules configured for output, 
*   and packing into 32 bits on modules configured for input.
*   The command expects a value which specifies how data should be  packed/unpacked:
*
*      status = ioctl(fd, DPIO2_CMD_DATA_PACKING_ENABLE, <packingMode>);
*
*   The argument <packingMode> must have one of the following values:
*
*      DPIO2_PACK_16_LSB_ON_FPDP
*      DPIO2_PACK_16_MSB_ON_FPDP
*      DPIO2_PACK_10_LSB_ON_FPDP
*      DPIO2_PACK_8_LSB_ON_FPDP
*      DPIO2_PACK_4_LSB_ON_FPDP
*
*   Related commands:
*
*     DPIO2_CMD_DATA_PACKING_DISABLE
*     DPIO2_CMD_DATA_PACKING_CAPABILITY_GET
*
*
* DPIO2_CMD_DATA_PACKING_DISABLE:
*
*   Disables unpacking from 32 bits words on DPIO2 modules configured for output, 
*   or packing into 32 bits on modules configured for input.
*   The command takes no arguments:
*
*      status = ioctl(fd, DPIO2_CMD_DATA_PACKING_DISABLE, 0);
*
*   Related commands:
*
*     DPIO2_CMD_DATA_PACKING_ENABLE
*     DPIO2_CMD_DATA_PACKING_CAPABILITY_GET
*
*
* DPIO2_CMD_DATA_PACKING_PIPELINE_CHECK:
*
*   Checks whether there are data in the packing pipeline on a DPIO2
*   module configured as input.  Expects a pointer to a BOOL as argument:
*   
*      BOOL   pipelineNotEmpty;
*
*      status = ioctl(fd, DPIO2_CMD_DATA_PACKING_PIPELINE_CHECK, &pipelineNotEmpty);
*
*   If there are data in the pipeline, the boolean variable referenced by
*   the pointer argument, is set to TRUE.  If the pipeline is empty, the boolean
*   variable is set to FALSE.
*
*   Related commands:
*
*      DPIO2_CMD_DATA_PACKING_ENABLE
*      DPIO2_CMD_DATA_PACKING_DISABLE
*      DPIO2_CMD_DATA_PACKING_PIPELINE_FLUSH
*
*
* DPIO2_CMD_DATA_PACKING_PIPELINE_FLUSH:
*
*   Flushes data in the packing pipeline on a DPIO2 module configured for input.
*   Garbage data is added for the lacking bits in the resulting 32 bits word.
*   This commands takes no arguments:
*
*      status = ioctl(fd, DPIO2_CMD_DATA_PACKING_PIPELINE_FLUSH, 0);
*
*   Related commands:
*
*      DPIO2_CMD_DATA_PACKING_ENABLE
*      DPIO2_CMD_DATA_PACKING_DISABLE
*      DPIO2_CMD_DATA_PACKING_PIPELINE_CHECK
*
*
* DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT:
*
*   Selects whether flow control by the SUSPEND signal shall be enabled
*   or not.  Expects a value of type BOOL as argument, and enables SUSPEND
*   flow control if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT, TRUE);
*
*   If the argument is FALSE, SUSPEND flow control is disabled:
*
*      status = ioctl(fd, DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT, FALSE);
*
*   By default SUSPEND flow control is enabled.
*   
*   Related commands:
*
*     DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT
*
*   
* DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT:
*
*   Selects whether flow control by the NRDY signal shall be enabled
*   or not.  Expects a value of type BOOL as argument, and enables NRDY
*   flow control if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT, TRUE);
*
*   If the argument is FALSE, NRDY flow control is disabled:
*
*      status = ioctl(fd, DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT, FALSE);
*
*   By default NRDY flow control is enabled.
*   
*   Related commands:
*
*     DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT
*
*   
* DPIO2_CMD_RES1_DIRECTION_SELECT:
*
*   Configures the direction of the RES1 signal.  Expects a value of type
*   BOOL as argument, and configures RES1 as output if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_RES1_DIRECTION_SELECT, TRUE);
*
*   If the argument is FALSE, RES1 is configured as input:
*
*      status = ioctl(fd, DPIO2_CMD_RES1_DIRECTION_SELECT, FALSE);
*
*   By default RES1 is configured as input.
*
*   Related commands:
*      DPIO2_CMD_RES1_OUTPUT_VALUE_SET
*      DPIO2_CMD_RES1_VALUE_GET
*
*
* DPIO2_CMD_RES1_OUTPUT_VALUE_SET:
*
*   Sets the output value of the RES1 signal.  Expects a value of type
*   BOOL as argument, and sets RES1 high if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_RES1_OUTPUT_VALUE_SET, TRUE);
*
*   If the argument is FALSE, RES1 is set low:
*
*      status = ioctl(fd, DPIO2_CMD_RES1_OUTPUT_VALUE_SET, FALSE);
*
*   Note: This command has no effect, unless the RES1 signal has been 
*         configured as output by the DPIO2_CMD_RES1_DIRECTION_SELECT command.
*
*   Related commands:
*      DPIO2_CMD_RES1_DIRECTION_SELECT
*      DPIO2_CMD_RES1_VALUE_GET
*
*
* DPIO2_CMD_RES1_VALUE_GET:
*
*   Returns the value of the RES1 signal.  Expects a pointer to a variable
*   of type BOOL as argument, and sets this variable to TRUE if RES1 is high
*   and FALSE if RES1 is low:
*
*      BOOL   res1IsHigh;
*
*      status = ioctl(fd, DPIO2_CMD_RES1_VALUE_GET, (int) &res1IsHigh);
*
*   Related commands:
*      DPIO2_CMD_RES1_DIRECTION_SELECT
*      DPIO2_CMD_RES1_OUTPUT_VALUE_SET
*
*
* DPIO2_CMD_RES2_DIRECTION_SELECT:
*
*   Configures the direction of the RES2 signal.  Expects a value of type
*   BOOL as argument, and configures RES2 as output if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_RES2_DIRECTION_SELECT, TRUE);
*
*   If the argument is FALSE, RES2 is configured as input:
*
*      status = ioctl(fd, DPIO2_CMD_RES2_DIRECTION_SELECT, FALSE);
*
*   By default RES2 is configured as input.
*
*   Related commands:
*      DPIO2_CMD_RES2_OUTPUT_VALUE_SET
*      DPIO2_CMD_RES2_VALUE_GET
*
*
* DPIO2_CMD_RES2_OUTPUT_VALUE_SET:
*
*   Sets the output value of the RES2 signal.  Expects a value of type
*   BOOL as argument, and sets RES2 high if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_RES2_OUTPUT_VALUE_SET, TRUE);
*
*   If the argument is FALSE, RES2 is set low:
*
*      status = ioctl(fd, DPIO2_CMD_RES2_OUTPUT_VALUE_SET, FALSE);
*
*   Note: This command has no effect, unless the RES2 signal has been 
*         configured as output by the DPIO2_CMD_RES2_DIRECTION_SELECT command.
*
*   Related commands:
*      DPIO2_CMD_RES2_DIRECTION_SELECT
*      DPIO2_CMD_RES2_VALUE_GET
*
*
* DPIO2_CMD_RES2_VALUE_GET:
*
*   Returns the value of the RES2 signal.  Expects a pointer to a variable
*   of type BOOL as argument, and sets this variable to TRUE if RES2 is high
*   and FALSE if RES2 is low:
*
*      BOOL   res2IsHigh;
*
*      status = ioctl(fd, DPIO2_CMD_RES2_VALUE_GET, (int) &res2IsHigh);
*
*   Related commands:
*      DPIO2_CMD_RES2_DIRECTION_SELECT
*      DPIO2_CMD_RES2_OUTPUT_VALUE_SET
*
*
* DPIO2_CMD_RES3_DIRECTION_SELECT:
*
*   Configures the direction of the RES3 signal.  Expects a value of type
*   BOOL as argument, and configures RES3 as output if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_RES3_DIRECTION_SELECT, TRUE);
*
*   If the argument is FALSE, RES3 is configured as input:
*
*      status = ioctl(fd, DPIO2_CMD_RES3_DIRECTION_SELECT, FALSE);
*
*   By default RES3 is configured as input.
*
*   Related commands:
*      DPIO2_CMD_RES3_OUTPUT_VALUE_SET
*      DPIO2_CMD_RES3_VALUE_GET
*
*
* DPIO2_CMD_RES3_OUTPUT_VALUE_SET:
*
*   Sets the output value of the RES3 signal.  Expects a value of type
*   BOOL as argument, and sets RES3 high if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_RES3_OUTPUT_VALUE_SET, TRUE);
*
*   If the argument is FALSE, RES3 is set low:
*
*      status = ioctl(fd, DPIO2_CMD_RES3_OUTPUT_VALUE_SET, FALSE);
*
*   Note: This command has no effect, unless the RES3 signal has been 
*         configured as output by the DPIO2_CMD_RES3_DIRECTION_SELECT command.
*
*   Related commands:
*      DPIO2_CMD_RES3_DIRECTION_SELECT
*      DPIO2_CMD_RES3_VALUE_GET
*
*
* DPIO2_CMD_RES3_VALUE_GET:
*
*   Returns the value of the RES3 signal.  Expects a pointer to a variable
*   of type BOOL as argument, and sets this variable to TRUE if RES3 is high
*   and FALSE if RES3 is low:
*
*      BOOL   res3IsHigh;
*
*      status = ioctl(fd, DPIO2_CMD_RES3_VALUE_GET, (int) &res3IsHigh);
*
*   Related commands:
*      DPIO2_CMD_RES3_DIRECTION_SELECT
*      DPIO2_CMD_RES3_OUTPUT_VALUE_SET
*
*
* DPIO2_CMD_PIO1_DIRECTION_SELECT:
*
*   Configures the direction of the PIO1 signal.  Expects a value of type
*   BOOL as argument, and configures PIO1 as output if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_PIO1_DIRECTION_SELECT, TRUE);
*
*   If the argument is FALSE, PIO1 is configured as input:

*
*      status = ioctl(fd, DPIO2_CMD_PIO1_DIRECTION_SELECT, FALSE);
*
*   By default PIO1 is configured as input.
*
*   Related commands:
*      DPIO2_CMD_PIO1_OUTPUT_VALUE_SET
*      DPIO2_CMD_PIO1_VALUE_GET
*
*
* DPIO2_CMD_PIO1_OUTPUT_VALUE_SET:
*
*   Sets the output value of the PIO1 signal.  Expects a value of type
*   BOOL as argument, and sets PIO1 high if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_PIO1_OUTPUT_VALUE_SET, TRUE);
*
*   If the argument is FALSE, PIO1 is set low:
*
*      status = ioctl(fd, DPIO2_CMD_PIO1_OUTPUT_VALUE_SET, FALSE);
*
*   Note: This command has no effect, unless the PIO1 signal has been 
*         configured as output by the DPIO2_CMD_PIO1_DIRECTION_SELECT command.
*
*   Related commands:
*      DPIO2_CMD_PIO1_DIRECTION_SELECT
*      DPIO2_CMD_PIO1_VALUE_GET
*
*
* DPIO2_CMD_PIO1_VALUE_GET:
*
*   Returns the value of the PIO1 signal.  Expects a pointer to a variable
*   of type BOOL as argument, and sets this variable to TRUE if PIO1 is high
*   and FALSE if PIO1 is low:
*
*      BOOL   pio1IsHigh;
*
*      status = ioctl(fd, DPIO2_CMD_PIO1_VALUE_GET, (int) &pio1IsHigh);
*
*   Related commands:
*      DPIO2_CMD_PIO1_DIRECTION_SELECT
*      DPIO2_CMD_PIO1_OUTPUT_VALUE_SET
*
*
* DPIO2_CMD_PIO2_DIRECTION_SELECT:
*
*   Configures the direction of the PIO2 signal.  Expects a value of type
*   BOOL as argument, and configures PIO2 as output if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_PIO2_DIRECTION_SELECT, TRUE);
*
*   If the argument is FALSE, PIO2 is configured as input:
*
*      status = ioctl(fd, DPIO2_CMD_PIO2_DIRECTION_SELECT, FALSE);
*
*   By default PIO2 is configured as input.
*
*   Related commands:
*      DPIO2_CMD_PIO2_OUTPUT_VALUE_SET
*      DPIO2_CMD_PIO2_VALUE_GET
*
*
* DPIO2_CMD_PIO2_OUTPUT_VALUE_SET:
*
*   Sets the output value of the PIO2 signal.  Expects a value of type
*   BOOL as argument, and sets PIO2 high if this argument is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_PIO2_OUTPUT_VALUE_SET, TRUE);
*
*   If the argument is FALSE, PIO2 is set low:
*
*      status = ioctl(fd, DPIO2_CMD_PIO2_OUTPUT_VALUE_SET, FALSE);
*
*   Note: This command has no effect, unless the PIO2 signal has been 
*         configured as output by the DPIO2_CMD_PIO2_DIRECTION_SELECT command.
*
*   Related commands:
*      DPIO2_CMD_PIO2_DIRECTION_SELECT
*      DPIO2_CMD_PIO2_VALUE_GET
*
*
* DPIO2_CMD_PIO2_VALUE_GET:
*
*   Returns the value of the PIO2 signal.  Expects a pointer to a variable
*   of type BOOL as argument, and sets this variable to TRUE if PIO2 is high
*   and FALSE if PIO2 is low:
*
*      BOOL   pio2IsHigh;
*
*      status = ioctl(fd, DPIO2_CMD_PIO2_VALUE_GET, (int) &pio2IsHigh);
*
*   Related commands:
*      DPIO2_CMD_PIO2_DIRECTION_SELECT
*      DPIO2_CMD_PIO2_OUTPUT_VALUE_SET
*
*
* DPIO2_CMD_TEST_PATTERN_GENERATION_ENABLE
*
*   On a DPIO2 configured for input, this command causes test patterns
*   to be written into the FIFO instead of data from the FPDP bus.
*   And on a DPIO2 configured for output, this command causes test patterns
*   to be clocked onto the FPDP bus instead of data from the FIFO.
*
*   The command expects an integer as argument:
*
*      status = ioctl(fd, DPIO2_CMD_TEST_PATTERN_GENERATION_ENABLE, <patternId>);
*
*   The integer given as argument, <patternId>, must have one of the following
*   values:
*
*      DPIO2_WALKING_ONE_PATTERN
*      DPIO2_WALKING_ZERO_PATTERN
*      DPIO2_COUNTER_PATTERN
*      DPIO2_COUNTER_PATTERN_WITH_PROGRAMMABLE_START
*
*   Note: To have effect, this command must be issued before the FPDP interface
*         is activated (DPIO2_CMD_FPDP_ACTIVATION_SELECT).
*
*   Related commands:
*      DPIO2_CMD_FPDP_ACTIVATION_SELECT
*      DPIO2_CMD_TEST_PATTERN_GENERATION_DISABLE
*
*
* DPIO2_CMD_TEST_PATTERN_GENERATION_DISABLE:
*
*   This command disables test pattern generation.
*
*   The command takes no arguments:
*
*      status = ioctl(fd, DPIO2_CMD_TEST_PATTERN_GENERATION_DISABLE, 0);
*
*   Note: To have effect, this command must be issued before the FPDP interface
*         is activated (DPIO2_CMD_FPDP_ACTIVATION_SELECT).
*
*   Related commands:
*      DPIO2_CMD_FPDP_ACTIVATION_SELECT
*      DPIO2_CMD_TEST_PATTERN_GENERATION_ENABLE
*
*
* DPIO2_CMD_TEST_PATTERN_START_VALUE_SET:
*
*   This command specifies the start value which will be used by
*   the test pattern generator when it is activated.  This start value
*   is only support for the counter pattern with programmable start
*   (DPIO2_COUNTER_PATTERN_WITH_PROGRAMMABLE_START).
*
*   The command takes a 16 bits unsigned integer as argument:
*
*      UINT16   startValue;
*
*      status = ioctl(fd, DPIO2_CMD_TEST_PATTERN_START_VALUE_SET, (int) startValue);
*
*   Related commands:
*      DPIO2_CMD_TEST_PATTERN_GENERATION_ENABLE
*      DPIO2_CMD_TEST_PATTERN_GENERATION_DISABLE
*
* DPIO2_CMD_STROBE_SKEW_SET
*
*   This command is used to specify the strobe skew when clocking data
*   on both strobe edges (the command has no effect when data is clocked on
*   only one of the strobe edges).
*
*   The amount of skew is specified as positive or negative multiples
*   of a Time Unit (tu).  The duration of one Time Unit is frequency dependent,
*   as shown in the table below:
*
*         Strobe Frequency (Fs) Range   |    Time Unit
*      -----------------------------------------------------
*                 < 25MHz               |   1 / (64 * Fs)
*               25MHz - 50MHz           |   1 / (32 * Fs)
*                 > 50MHz               |   1 / (16 * Fs)
*
*   The command takes an integer as argument:
*
*      status = ioctl(fd, DPIO2_CMD_STROBE_SKEW_SET, <skew-in-time-units>);
*
*   Valid values for <skew-in-time-units> are -6, -4, -2, 0, 2, 4, and 6.
*
*   Related commands:
*      DPIO2_CMD_DEFAULT_STROBE_SKEW_SET
*      DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT
*
*
* DPIO2_CMD_DEFAULT_STROBE_SKEW_SET
*
*   This command makes the driver use default strobe skew when clocking data
*   on both strobe edges (the command has no effect when data is clocked on
*   only one of the strobe edges).
*
*   The command takes no arguments:
*
*      status = ioctl(fd, DPIO2_CMD_DEFAULT_STROBE_SKEW_SET, 0);
*
*   Related commands:
*      DPIO2_CMD_STROBE_SKEW_SET
*      DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT
*
*
* DPIO2_CMD_DMA_SET_DESC:
*
*   Writes a DMA descriptor to the SRAM on the DPIO2 module.
*   Expects a pointer to a DPIO2_DMA_DESCRIPTOR structure as argument:
*
*      DPIO2_DMA_DESCRIPTOR   descriptor;
*
*      ioctl(fd, DPIO2_CMD_DMA_SET_DESC, &descriptor);
*
*   The DPIO2_DMA_DESCRIPTOR structure have the following fields:
*
*      descriptorId          - value that uniquely identifies 
*                              the descriptor to be written.
*                              
*      nextDescriptorId      - value that uniquely identifies 
*                              the next descriptor the DMA Controller
*                              will load when the transfer represented
*                              by this descriptor is completed.
*
*      pciAddress            - base address in PCI Memory Space
*                              where the data represented by this descriptor
*                              is written to or read from.  This address
*                              must be aligned to a 4 bytes (8 bytes) boundary
*                              when 32 bits (64 bits) accesses are to be used.
*
*      blockSizeInBytes      - number of bytes to transfer.
*                              The number of bytes must be a multiple of 4 bytes
*                              (8 bytes) when 32 bits (64 bits) accesses are
*                              to be used, and its minimum value is 16.
*
*      lastBlockInChain      - boolean value which must be set to TRUE if
*                              this descriptor shall be the last in a DMA chain.
*
*      endOfBlockInterrupt   - boolean value which must be set to TRUE if
*                              the DMA Controller shall generate an interrupt
*                              when the transfer represented by this descriptor
*                              is completed.
*
*      useD64                - boolean value which must be set to TRUE if
*                              the DMA controller shall try to use D64 accesses
*                              to transfer the data represented by this descriptor.
*                              If this value is FALSE the DMA controller will use
*                              D32 accesses to transfer.
*
*      notEndOfFrame         - boolean value which must be set to TRUE if
*                              the data represented by this descriptor shall
*                              not be handled as the last part of a frame.
*      
*
* DPIO2_CMD_DMA_SET_START_DESCRIPTOR:
*
*   Sets ID of the DMA descriptor that the DMA controller shall load first. 
*   Expects the ID of the start descriptor as argument:
*
*      UINT32   startDmaId;
*
*      status = ioctl(fd, DPIO2_CMD_DMA_SET_START_DESCRIPTOR, startDmaId);
*
*
* DPIO2_CMD_DMA_START:
*
*   Starts the DMA controller:
*
*      status = ioctl(fd, DPIO2_CMD_DMA_START, 0);
*
*   The ID of the DMA descriptor the DMA controller shall load and use for
*   the first transfer, must be set before this command is issued.
*
*   Related commands:
*
*     DPIO2_CMD_DMA_SET_START_DESCRIPTOR
*     DPIO2_CMD_DMA_ABORT
*     DPIO2_CMD_DMA_GET_DONE
*
*
* DPIO2_CMD_DMA_ABORT:
*
*   Stops the DMA controller if it is running.  In order to restart afterwards, 
*   the DMA controller needs load a new DMA descriptor.
*   This command can return the ID of the next descriptor the DMA controller
*   would have loaded, if a pointer to a UINT32 is given as argument:
*
*      UINT32   nextDescriptorId;
*
*      status = ioctl(fd, DPIO2_CMD_DMA_ABORT, &nextDescriptorId);
*
*   If the ID of the next descriptor is not required, set the argument to zero:
*
*      status = ioctl(fd, DPIO2_CMD_DMA_ABORT, 0);
*
*   Related commands:
*
*     DPIO2_CMD_DMA_SET_START_DESCRIPTOR
*     DPIO2_CMD_DMA_START
*     DPIO2_CMD_DMA_SUSPEND
*
*
* DPIO2_CMD_FLUSH_ON_DMA_ABORT_SELECT:
*
*   Selects whether data queued in DMA Controller should be flushed 
*   when a DMA transfer is aborted.
*
*   This command expects a boolean value as parameter and enables flushing
*   if this value is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_FLUSH_ON_DMA_ABORT_SELECT, TRUE);
*
*   Flushing is disabled if the boolean value is FALSE:
*
*      status = ioctl(fd, DPIO2_CMD_FLUSH_ON_DMA_ABORT_SELECT, FALSE);
*
*   Related commands:
*
*     DPIO2_CMD_DMA_STOP
*
*
* DPIO2_CMD_DMA_SUSPEND:
*
*   Pauses the DMA controller if it is running.  In order to restart, 
*   the DMA controller does not need to load a new DMA descriptor.
*   This command can return the ID of the next descriptor the DMA controller
*   would have loaded, if a pointer to a UINT32 is given as argument:
*
*      UINT32   nextDescriptorId;
*
*      status = ioctl(fd, DPIO2_CMD_DMA_SUSPEND, &nextDescriptorId);
*
*   If the ID of the next descriptor is not required, set the argument to zero:
*
*      status = ioctl(fd, DPIO2_CMD_DMA_SUSPEND, 0);
*
*   Related commands:
*
*     DPIO2_CMD_DMA_RESUME
*     DPIO2_CMD_DMA_ABORT
*
*
* DPIO2_CMD_DMA_RESUME:
*
*   Restarts the DMA controller after it has been paused by the
*   DPIO2_DMA_SUSPEND command.
*
*      status = ioctl(fd, DPIO2_CMD_DMA_RESUME, 0);
*
*   The DMA controller does not load a new DMA descriptor, but continues
*   the transfer that was paused.
*
*   Related commands:
*
*     DPIO2_CMD_DMA_SUSPEND
*     DPIO2_CMD_DMA_START
*     DPIO2_CMD_DMA_GET_DONE
*
*
* DPIO2_CMD_DMA_GET_DONE:
*
*   Checks whether the DMA controller has completed a data transfer.
*   Expects a pointer to a BOOL variable <dmaTransferIsCompleted> as argument,
*   and sets this variable to TRUE if the DMA controller has completed the transfer
*   or FALSE if it has not:
*  
*      BOOL   dmaTransferIsCompleted;
*
*      status = ioctl(fd, DPIO2_CMD_DMA_GET_DONE, &dmaTransferIsCompleted);
*
*   Related commands:
*
*     DPIO2_CMD_DMA_START
*     DPIO2_CMD_DMA_RESUME
*
*
* DPIO2_CMD_REG_GET_DEMAND_MD:
*
*   Expects a pointer to a BOOL variable <demandModeIsEnabled> as argument,
*   and sets this variable to TRUE if DMA Demand Mode is enabled
*   or FALSE if not:
*
*      BOOL   demandModeIsEnabled;
*
*      status = ioctl(fd, DPIO2_CMD_REG_GET_DEMAND_MD, &demandModeIsEnabled);
*
*
* DPIO2_CMD_REG_SET_DEMAND_MD:
*
*   Enables DMA Demand Mode:
*
*      status = ioctl(fd, DPIO2_CMD_REG_SET_DEMAND_MD, 0);
*
*
* DPIO2_CMD_REG_CLR_DEMAND_MD:
*
*   Disables DMA Demand Mode:
*
*      status = ioctl(fd, DPIO2_CMD_REG_CLR_DEMAND_MD, 0);
*
*
* DPIO2_CMD_CONTINUE_ON_EOT_SELECT:
*
*   Selects whether the DMA controller should stop the transfer on EOT, 
*   or continue on the next descriptor in the DMA chain.
*   This command expects a boolean value as parameter, and configures
*   the DMA controller to continue on EOT if this value is TRUE:
*
*      status = ioctl(fd, DPIO2_CMD_CONTINUE_ON_EOT_SELECT, TRUE);
*   
*   The DMA is configured to stop on EOT if the boolean value is FALSE:
*
*      status = ioctl(fd, DPIO2_CMD_CONTINUE_ON_EOT_SELECT, FALSE);
*
*   Related Commands:
*
*      DPIO2_CMD_EOT_ENABLE
*      DPIO2_CMD_EOT_DISABLE
*
*
* DPIO2_CMD_EOT_ENABLE:
*
*   Enables the End-Of-Transfer mechanism for a DPIO2 configured as input.
*   The EOT mode to be used is specified by <eotMode> which must be either 
*   DPIO2_SYNC_MARKS_END_OF_TRANSFER, DPIO2_PIO1_MARKS_END_OF_TRANSFER, 
*   DPIO2_PIO1_MARKS_END_OF_TRANSFER, or DPIO2_RES1_MARKS_END_OF_TRANSFER:
* 
*      status = ioctl(fd, DPIO2_CMD_EOT_ENABLE, DPIO2_SYNC_MARKS_END_OF_TRANSFER);
*
*   In order to have effect, this command must be issued before the FPDP
*   interface is activated (see command DPIO2_CMD_FPDP_ACTIVATION_SELECT).
*
*
* DPIO2_CMD_EOT_DISABLE:
*
*   Disables the End-Of-Transfer mechanism for a DPIO2 configured as input:
* 
*      status = ioctl(fd, DPIO2_CMD_EOT_DISABLE, 0);
*
*
* DPIO2_CMD_EOT_COUNT_ENABLE:
*
*   This command enables the EOT count mechanism, which makes the DPIO2 wait for
*   a specific number of EOT marks before it terminates a DMA transfer.
*   The number of EOT marks is specified by a 16 bits unsigned integer given
*   as parameter to the command:
*
*      UINT16   numEotMarks;
*
*      status = ioctl(fd, DPIO2_CMD_EOT_COUNT_ENABLE, (int) numEotMarks);
*
*   Note: The EOT count mechanism is only applicable when using the SYNC signal as EOT mark.
*
*   Related Commands:
*
*      DPIO2_CMD_EOT_ENABLE
*      DPIO2_CMD_EOT_DISABLE
*      DPIO2_CMD_EOT_COUNT_DISABLE
*
*
* DPIO2_CMD_EOT_COUNT_DISABLE:
*
*   This command disables the EOT count mechanism, making the DPIO2 terminate
*   a DMA transfer on the first EOT mark it detects.  The command takes no parameters:
*
*      status = ioctl(fd, DPIO2_CMD_EOT_COUNT_DISABLE, 0);
*
*   Note: The EOT count mechanism is only applicable when using the SYNC signal as EOT mark.
*
*   Related Commands:
*
*      DPIO2_CMD_EOT_ENABLE
*      DPIO2_CMD_EOT_DISABLE
*      DPIO2_CMD_EOT_COUNT_ENABLE
*
*
* DPIO2_CMD_DMA_HALTP_EN:
*
*   Enables driver to automatically halt the processor after starting 
*   a DMA transfer:
*
*      status = ioctl(fd, DPIO2_CMD_DMA_HALTP_EN, 0);
*
*   This command has no effect, unless a pointer to a halt function was
*   given when the driver was installed (see dpio2Drv()).
*
*
* DPIO2_CMD_DMA_HALTP_DIS:
*
*   Disables automatically halting of the processor after starting a DMA transfer:
*
*      status = ioctl(fd, DPIO2_CMD_DMA_HALTP_DIS, 0);
*
*
* DPIO2_CMD_REMAINING_BYTE_COUNT_GET:
*
*   This command returns the number of bytes that were specified for transfer 
*   by the last DMA descriptor but not transferred when the DMA controller
*   terminated the command.
*
*   The command expects a pointer to a DPIO2_REMAINING_BYTE_COUNT_INFO structure
*   as parameter:
*
*      DPIO2_REMAINING_BYTE_COUNT_INFO   byteCountInfo;
*
*      status = ioctl(fd, DPIO2_CMD_REMAINING_BYTE_COUNT_GET, (int) &byteCountInfo);
*
*   The DPIO2_REMAINING_BYTE_COUNT_INFO contains two fields:
*
*      byteCount - the number of bytes that were specified for transfer by
*                  the last DMA descriptor but not transferred.
*
*      overflowFlag - boolean value which signals whether a previous value of 
*                     the byte count was overwritten before it was read.
*
*   Related commands:
*
*      DPIO2_CMD_TRANSFERRED_BYTE_COUNT_GET
*
*
* DPIO2_CMD_TRANSFERRED_BYTE_COUNT_GET:
*
*   This command returns the number of bytes that were already transferred
*   by the last DMA descriptor, when the DMA controller terminated the transfer.
*
*   The command expects a pointer to an unsigned 32 bits integer as parameter:
*
*      UINT32   byteCount;
*
*      status = ioctl(fd, DPIO2_CMD_TRANSFERRED_BYTE_COUNT_GET, (int) &byteCount);
*
*   Related commands:
*
*      DPIO2_CMD_REMAINING_BYTE_COUNT_GET
*
*
* DPIO2_CMD_INTERRUPT_ENABLE:
*
*   Enables an interrupt.
*   Expects an integer as argument which specifies what interrupt to enable:
*
*      status = ioctl(fd, DPIO2_CMD_INTERRUPT_ENABLE, <interrupt>);
*
*   The following vales can be used for <interrupt>:
*
*      DPIO2_INT_COND_DMA_DONE_CHAIN    
*      DPIO2_INT_COND_DMA_DONE_BLOCK    
*      DPIO2_INT_COND_FIFO_NOT_EMPTY    
*      DPIO2_INT_COND_FIFO_EMPTY
*      DPIO2_INT_COND_FIFO_ALMOST_EMPTY
*      DPIO2_INT_COND_FIFO_HALF_FULL
*      DPIO2_INT_COND_FIFO_ALMOST_FULL
*      DPIO2_INT_COND_FIFO_FULL
*      DPIO2_INT_COND_FIFO_OVERFLOW
*      DPIO2_INT_COND_PIO1
*      DPIO2_INT_COND_PIO2
*      DPIO2_INT_COND_SYNC
*      DPIO2_INT_COND_SUSPEND
*      DPIO2_INT_COND_TARGET_ABORT
*      DPIO2_INT_COND_MASTER_ABORT
*
*
* DPIO2_CMD_INTERRUPT_DISABLE:
*
*   Disables an interrupt.
*   Expects an integer as argument which specifies what interrupt to disable:
*
*      status = ioctl(fd, DPIO2_CMD_INTERRUPT_DISABLE, <interrupt>);
*
*   The following vales can be used for <interrupt>:
*
*      DPIO2_INT_COND_DMA_DONE_CHAIN    
*      DPIO2_INT_COND_DMA_DONE_BLOCK    
*      DPIO2_INT_COND_FIFO_NOT_EMPTY    
*      DPIO2_INT_COND_FIFO_EMPTY
*      DPIO2_INT_COND_FIFO_ALMOST_EMPTY
*      DPIO2_INT_COND_FIFO_HALF_FULL
*      DPIO2_INT_COND_FIFO_ALMOST_FULL
*      DPIO2_INT_COND_FIFO_FULL
*      DPIO2_INT_COND_FIFO_OVERFLOW
*      DPIO2_INT_COND_PIO1
*      DPIO2_INT_COND_PIO2
*      DPIO2_INT_COND_SYNC
*      DPIO2_INT_COND_SUSPEND
*      DPIO2_INT_COND_TARGET_ABORT
*      DPIO2_INT_COND_MASTER_ABORT
*
*
* DPIO2_CMD_INTERRUPT_CALLBACK_ATTACH:
*
*   Attaches a callback function to a specified interrupt condition.
*   Expects a pointer to a DPIO2_INTERRUPT_CALLBACK structure as argument:
*
*      DPIO2_INTERRUPT_CALLBACK   callback;
*
*      status = ioctl(fd, DPIO2_CMD_INTERRUPT_CALLBACK_ATTACH, &callback);
*
*   The DPIO2_INTERRUPT_CALLBACK structure have the following fields:
*
*      condition           - value identifying the interrupt condition
*                            which the callback function shall signal
*
*      pCallbackFunction   - pointer to the callback function.
*                            The callback function must take an integer (int)
*                            as argument and return an integer (int).
*
*      argument            - argument to be passed to the callback function
*                            each time it is called.
*
*   The condition field must have one of the following values:
*
*      DPIO2_INT_COND_DMA_DONE_CHAIN    
*      DPIO2_INT_COND_DMA_DONE_BLOCK    
*      DPIO2_INT_COND_FIFO_NOT_EMPTY    
*      DPIO2_INT_COND_FIFO_EMPTY
*      DPIO2_INT_COND_FIFO_ALMOST_EMPTY
*      DPIO2_INT_COND_FIFO_HALF_FULL
*      DPIO2_INT_COND_FIFO_ALMOST_FULL
*      DPIO2_INT_COND_FIFO_FULL
*      DPIO2_INT_COND_FIFO_OVERFLOW
*      DPIO2_INT_COND_PIO1
*      DPIO2_INT_COND_PIO2
*      DPIO2_INT_COND_SYNC
*      DPIO2_INT_COND_SUSPEND
*      DPIO2_INT_COND_TARGET_ABORT
*      DPIO2_INT_COND_MASTER_ABORT
*
*
* DPIO2_CMD_INTERRUPT_CALLBACK_DETACH:
*
*   Detaches a callback function from a specified interrupt condition.
*   Expects an integer as argument which specifies the interrupt condition
*   to detach the callback function from:
*
*      status = ioctl(fd, DPIO2_CMD_INTERRUPT_CALLBACK_DETACH, <interrupt>);
*
*   The following vales can be used for <interrupt>:
*
*      DPIO2_INT_COND_DMA_DONE_CHAIN    
*      DPIO2_INT_COND_DMA_DONE_BLOCK    
*      DPIO2_INT_COND_FIFO_NOT_EMPTY    
*      DPIO2_INT_COND_FIFO_EMPTY
*      DPIO2_INT_COND_FIFO_ALMOST_EMPTY
*      DPIO2_INT_COND_FIFO_HALF_FULL
*      DPIO2_INT_COND_FIFO_ALMOST_FULL
*      DPIO2_INT_COND_FIFO_FULL
*      DPIO2_INT_COND_FIFO_OVERFLOW
*      DPIO2_INT_COND_PIO1
*      DPIO2_INT_COND_PIO2
*      DPIO2_INT_COND_SYNC
*      DPIO2_INT_COND_SUSPEND
*      DPIO2_INT_COND_TARGET_ABORT
*      DPIO2_INT_COND_MASTER_ABORT
*
*
* DPI02_CMD_AUTO_DISABLE_INTERRUPT_ENABLE:
*
*   Enables automatical disabling of interrupt source when the first interrupt
*   from it is serviced.  Expects an integer as argument which specifies the 
*   interrupt:
*
*      status = ioctl(fd, DPI02_CMD_AUTO_DISABLE_INTERRUPT_ENABLE, <interrupt>);
*
*   The following vales can be used for <interrupt>:
*
*      DPIO2_INT_COND_DMA_DONE_CHAIN    
*      DPIO2_INT_COND_DMA_DONE_BLOCK    
*      DPIO2_INT_COND_FIFO_NOT_EMPTY    
*      DPIO2_INT_COND_FIFO_EMPTY
*      DPIO2_INT_COND_FIFO_ALMOST_EMPTY
*      DPIO2_INT_COND_FIFO_HALF_FULL
*      DPIO2_INT_COND_FIFO_ALMOST_FULL
*      DPIO2_INT_COND_FIFO_FULL
*      DPIO2_INT_COND_FIFO_OVERFLOW
*      DPIO2_INT_COND_PIO1
*      DPIO2_INT_COND_PIO2
*      DPIO2_INT_COND_SYNC
*      DPIO2_INT_COND_SUSPEND
*      DPIO2_INT_COND_TARGET_ABORT
*      DPIO2_INT_COND_MASTER_ABORT
*
*
* DPIO2_CMD_AUTO_DISABLE_INTERRUPT_DISABLE:
*
*   Disables automatical disabling of interrupt source when the first interrupt
*   from it is serviced.  Expects an integer as argument which specifies the 
*   interrupt:
*
*      status = ioctl(fd, DPI02_CMD_AUTO_DISABLE_INTERRUPT_DISABLE, <interrupt>);
*
*   The following vales can be used for <interrupt>:
*
*      DPIO2_INT_COND_DMA_DONE_CHAIN    
*      DPIO2_INT_COND_DMA_DONE_BLOCK    
*      DPIO2_INT_COND_FIFO_NOT_EMPTY    
*      DPIO2_INT_COND_FIFO_EMPTY
*      DPIO2_INT_COND_FIFO_ALMOST_EMPTY
*      DPIO2_INT_COND_FIFO_HALF_FULL
*      DPIO2_INT_COND_FIFO_ALMOST_FULL
*      DPIO2_INT_COND_FIFO_FULL
*      DPIO2_INT_COND_FIFO_OVERFLOW
*      DPIO2_INT_COND_PIO1
*      DPIO2_INT_COND_PIO2
*      DPIO2_INT_COND_SYNC
*      DPIO2_INT_COND_SUSPEND
*      DPIO2_INT_COND_TARGET_ABORT
*      DPIO2_INT_COND_MASTER_ABORT
*
*
* DPIO2_CMD_FIFO_FLUSH:
*
*   Flushes any data currently stored in the FIFO:
* 
*      status = ioctl(fd, DPIO2_CMD_FIFO_FLUSH, 0);
*
*
* DPIO2_CMD_GET_CURRENT_FIFO_STATUS:
*
*   Gets the current state of the FIFO.
*   Expects a pointer to a variable of type int as argument,
*   and sets this variable to a constant representing the current state:
*
*      int   fifoState;
*
*      status = ioctl(fd, DPIO2_CMD_GET_CURRENT_FIFO_STATUS, &fifoState);
*
*   The constant returned in <fifoState> will be one of the following:
*
*      DPIO2_FIFO_EMPTY
*      DPIO2_FIFO_ALMOST_EMPTY
*      DPIO2_FIFO_LESS_THAN_HALF_FULL
*      DPIO2_FIFO_GREATER_THAN_HALF_FULL
*      DPIO2_FIFO_ALMOST_FULL
*      DPIO2_FIFO_FULL
*
*
* DPIO2_CMD_RESET_OCCURRED_FLAGS:
*
*   Resets history flags which tell what states the FIFO has been in:
*
*      status = ioctl(fd, DPIO2_CMD_RESET_OCCURRED_FLAGS, 0);
*
*
* DPIO2_CMD_GET_FIFO_OVERFLOW_OCCURRED_FLAG:
*
*   Gets the flag which tells whether the FIFO has overflowed since the
*   last time the occurred flags were reset (see DPIO2_CMD_RESET_OCCURRED_FLAGS).
*   Expects a pointer to a BOOL variable as argument, and sets this
*   variable to TRUE if the FIFO has overflowed or FALSE if not:
*
*      BOOL   fifoHasOverflowed;
*
*      status = ioctl(fd, DPIO2_CMD_GET_FIFO_OVERFLOW_OCCURRED_FLAG, &fifoHasOverflowed);
*
*
* DPIO2_CMD_GET_FIFO_FULL_OCCURRED_FLAG:
*
*   Gets the flag which tells whether the FIFO has been full since the
*   last time the occurred flags were reset (see DPIO2_CMD_RESET_OCCURRED_FLAGS).
*   Expects a pointer to a BOOL variable as argument, and sets this
*   variable to TRUE if the FIFO has been full or FALSE if not:
*
*      BOOL   fifoHasBeenFull;
*
*      status = ioctl(fd, DPIO2_CMD_GET_FIFO_FULL_OCCURRED_FLAG, &fifoHasBeenFull);
*
*
* DPIO2_CMD_GET_FIFO_ALMOST_FULL_OCCURRED_FLAG:
*
*   Gets the flag which tells whether the FIFO has been almost full since the
*   last time the occurred flags were reset (see DPIO2_CMD_RESET_OCCURRED_FLAGS).
*   Expects a pointer to a BOOL variable as argument, and sets this
*   variable to TRUE if the FIFO has been almost full or FALSE if not:
*
*      BOOL   fifoHasBeenAlmostFull;
*
*      status = ioctl(fd, DPIO2_CMD_GET_FIFO_ALMOST_FULL_OCCURRED_FLAG, &fifoHasBeenAlmostFull);
*
*
* DPIO2_CMD_GET_FIFO_HALF_FULL_OCCURRED_FLAG:
*
*   Gets the flag which tells whether the FIFO has been more than half full since the
*   last time the occurred flags were reset (see DPIO2_CMD_RESET_OCCURRED_FLAGS).
*   Expects a pointer to a BOOL variable as argument, and sets this
*   variable to TRUE if the FIFO has been half full or FALSE if not:
*
*      BOOL   fifoHasBeenHalfFull;
*
*      status = ioctl(fd, DPIO2_CMD_GET_FIFO_HALF_FULL_OCCURRED_FLAG, &fifoHasBeenHalfFull);
*
*
* DPIO2_CMD_GET_FIFO_ALMOST_EMPTY_OCCURRED_FLAG:
*
*   Gets the flag which tells whether the FIFO has been almost empty since the
*   last time the occurred flags were reset (see DPIO2_CMD_RESET_OCCURRED_FLAGS).
*   Expects a pointer to a BOOL variable as argument, and sets this
*   variable to TRUE if the FIFO has been almost empty or FALSE if not:
*
*      BOOL   fifoHasBeenAlmostEmpty;
*
*      status = ioctl(fd, DPIO2_CMD_GET_FIFO_ALMOST_EMPTY_OCCURRED_FLAG, &fifoHasBeenAlmostEmpty);
*
*
* DPIO2_CMD_GET_FIFO_EMPTY_OCCURRED_FLAG:
*
*   Gets the flag which tells whether the FIFO has been empty since the
*   last time the occurred flags were reset (see DPIO2_CMD_RESET_OCCURRED_FLAGS).
*   Expects a pointer to a BOOL variable as argument, and sets this
*   variable to TRUE if the FIFO has been empty or FALSE if not:
*
*      BOOL   fifoHasBeenEmpty;
*
*      status = ioctl(fd, DPIO2_CMD_GET_FIFO_EMPTY_OCCURRED_FLAG, &fifoHasBeenEmpty);
*
*
* DPIO2_CMD_LATENCY_TIMER_GET:
*
*   Gets the current value of the PCI Latency Timer.
*   Expects a pointer to a UINT8 variable as argument, and sets this
*   variable to the value of the PCI Latency Timer:
*   
*      UINT8   latencyTimer;
*
*      status = ioctl(fd, DPIO2_CMD_LATENCY_TIMER_GET, &latencyTimer);
*
*
* DPIO2_CMD_LATENCY_TIMER_SET:
*
*   Sets a new value for the PCI Latency Timer.
*   Expects UINT8 value as argument, and the Latency Timer equal to this:
*   
*      UINT8   latencyTimer;
*
*      status = ioctl(fd, DPIO2_CMD_LATENCY_TIMER_SET, latencyTimer);
*
*
* DPIO2_CMD_GET_PTR_FIFO:
*
*   Gets a pointer to the FIFO of the DPIO2 module.
*   Expects a pointer to a UINT32 pointer as argument, 
*   and sets this pointer variable equal to the local CPU address 
*   where the FIFO is mapped:
*
*      UINT32*   pFifo;
*
*      status = ioctl(fd, DPIO2_CMD_GET_PTR_FIFO, &pFifo);
*
*
* DPIO2_CMD_GET_DEVICE_FSIZE:
*
*   Gets the size of the FIFO in words.
*   Expects a pointer to a variable of type UINT32 as argument, and sets this
*   variable to the size of the FIFO.
*
*      UINT32   fifoSizeInWords;
*
*      status = ioctl(fd, DPIO2_CMD_GET_DEVICE_FSIZE, &fifoSizeInWords);
*
*
* DPIO2_CMD_GET_DEVICE_BUSNUM:
*
*   Gets the PCI bus number of the PMC slot where the DPIO2 module is fitted.
*   Expects a pointer to a variable of type int as argument, and stores the
*   PCI bus number in this variable:
*
*      int   pciBusNumber;
*
*      status = ioctl(fd, DPIO2_CMD_GET_DEVICE_BUSNUM, &pciBusNumber);
*
*
* DPIO2_CMD_GET_DEVICE_DEVNUM:
*
*   Gets the PCI device number of the PMC slot where the DPIO2 module is fitted.
*   Expects a pointer to a variable of type int as argument, and stores the
*   PCI device number in this variable:
*
*      int   pciDeviceNumber;
*
*      status = ioctl(fd, DPIO2_CMD_GET_DEVICE_DEVNUM, &pciDeviceNumber);
*
*
* RETURNS: OK if successfull, ERROR if not.
*
* ERRNO:
* INVALID_ARG - If invalid argument.
* DMA_DONE - If DMA controller not running.
* DMA_ENABLED - If DMA controller is already enabled.
*/
STATUS dpio2Ioctl (DPIO2_DEVICE* pDevice, int command, int argument)
{

  DPIO2_DMA_DESCRIPTOR*   pDescriptor;

  DPIO2_INTERRUPT_SOURCE_ID   sourceId;

  int   iCondition;

  STATUS   status;

//  DPIO2_INTERRUPT_CALLBACK*   pInterruptCallback;                    //rsr commented out - not used in this version of the function
    
  DPIO2_COUNTER_ADDRESSING_INFO*   pCounterInfo;
    
  DPIO2_REMAINING_BYTE_COUNT_INFO*   pByteCountInfo;

#ifdef RSR_DEBUG
  printf("In dpio2Ioctl - dpio2Ioctl (int) version\n"); //rsr
#endif

  switch (command) {


#ifndef VMOSA

  /* The following commands was defined for the DPIO but have no meaning for DPIO2.
   * They are included here to issue warnings if an application tries to use them.
   */
  case DPIO_CMD_REG_SET_MODE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_SET_MODE (%d) is not implemented\n", DPIO_CMD_REG_SET_MODE));
    return (ERROR);


  case DPIO_CMD_REG_GET_MODE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_MODE (%d) is not implemented\n", DPIO_CMD_REG_GET_MODE));
    return (ERROR);


  case DPIO_CMD_REG_SET_PIO_CTR:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_SET_PIO_CTR (%d) is not implemented\n", DPIO_CMD_REG_SET_PIO_CTR));
    return (ERROR);


  case DPIO_CMD_REG_GET_PIO_CTR:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_PIO_CTR (%d) is not implemented\n", DPIO_CMD_REG_GET_PIO_CTR));
    return (ERROR);


  case DPIO_CMD_REG_SET_INT_CTR:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_SET_INT_CTR (%d) is not implemented\n", DPIO_CMD_REG_SET_INT_CTR));
    return (ERROR);
        

  case DPIO_CMD_REG_GET_INT_CTR:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_INT_CTR (%d) is not implemented\n", DPIO_CMD_REG_GET_INT_CTR));
    return (ERROR);
  
          
  case DPIO_CMD_REG_SET_DMA_CTR:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_SET_DMA_CTR (%d) is not implemented\n", DPIO_CMD_REG_SET_DMA_CTR));
    return (ERROR);


  case DPIO_CMD_REG_GET_DMA_CTR:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_DMA_CTR (%d) is not implemented\n", DPIO_CMD_REG_GET_DMA_CTR));
    return (ERROR);


  case DPIO_CMD_EOTB3_ENABLE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_EOTB3_ENABLE (%d) is not implemented\n", DPIO_CMD_EOTB3_ENABLE));
    return (ERROR);


  case DPIO_CMD_REG_GET_FIFO_STAT:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_FIFO_STAT (%d) is not implemented\n", DPIO_CMD_REG_GET_FIFO_STAT));
    return (ERROR);

  
  case DPIO_CMD_REG_GET_PIO_STAT:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_PIO_STAT (%d) is not implemented\n", DPIO_CMD_REG_GET_PIO_STAT));
    return (ERROR);


  case DPIO_CMD_REG_GET_INT_STAT:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_INT_STAT (%d) is not implemented\n", DPIO_CMD_REG_GET_INT_STAT));
    return (ERROR);


  case DPIO_CMD_REG_GET_MOD_ID:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REG_GET_MOD_ID (%d) is not implemented\n", DPIO_CMD_REG_GET_MOD_ID));
    return (ERROR);
        

  case DPIO_CMD_LMEM_UINT32_WRITE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_LMEM_UINT32_WRITE (%d) is not implemented\n", DPIO_CMD_LMEM_UINT32_WRITE));
    return (ERROR);


  case DPIO_CMD_LMEM_UINT32_READ:
    DPIO2_ERROR_MSG(("The DPIO_CMD_LMEM_UINT32_READ (%d) is not implemented\n", DPIO_CMD_LMEM_UINT32_READ));
    return (ERROR);


  case DPIO_CMD_PLX_UINT32_WRITE: 
    DPIO2_ERROR_MSG(("The DPIO_CMD_PLX_UINT32_WRITE (%d) is not implemented\n", DPIO_CMD_PLX_UINT32_WRITE));
    return (ERROR);


  case DPIO_CMD_PLX_UINT32_READ:
    DPIO2_ERROR_MSG(("The DPIO_CMD_PLX_UINT32_READ (%d) is not implemented\n", DPIO_CMD_PLX_UINT32_READ));
    return (ERROR);


  case DPIO_CMD_REINIT:
    DPIO2_ERROR_MSG(("The DPIO_CMD_REINIT (%d) is not implemented\n", DPIO_CMD_REINIT));
    return (ERROR);


  case DPIO_CMD_GET_PTR_CTRL_REGS:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_CTRL_REGS (%d) is not implemented\n", DPIO_CMD_GET_PTR_CTRL_REGS));
    return (ERROR);


  case DPIO_CMD_GET_PTR_STAT_REGS:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_STAT_REGS (%d) is not implemented\n", DPIO_CMD_GET_PTR_STAT_REGS));
    return (ERROR);


  case DPIO_CMD_GET_PTR_PCI9060_REGS:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_PCI9060_REGS (%d) is not implemented\n", DPIO_CMD_GET_PTR_PCI9060_REGS));
    return (ERROR);

    
  case DPIO_CMD_SET_INT_ACTION:
    DPIO2_ERROR_MSG(("The DPIO_CMD_SET_INT_ACTION (%d) is not implemented\n", DPIO_CMD_SET_INT_ACTION));
    return (ERROR);


  case DPIO_CMD_DMA_INT_ENABLE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_DMA_INT_ENABLE (%d) is not implemented\n", DPIO_CMD_DMA_INT_ENABLE));
    return (ERROR);


  case DPIO_CMD_DMA_INT_DISABLE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_DMA_INT_DISABLE (%d) is not implemented\n", DPIO_CMD_DMA_INT_DISABLE));
    return (ERROR);


  case DPIO_CMD_GET_PTR_RUNTIME_BASE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_RUNTIME_BASE (%d) is not implemented\n", DPIO_CMD_GET_PTR_RUNTIME_BASE));
    return (ERROR);

#endif /* !VMOSA */


  /* The following commands are not implemented yet
   */
  case DPIO2_CMD_GET_PTR_SRAM:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_GET_PTR_SRAM (%d) is not implemented\n", DPIO2_CMD_GET_PTR_SRAM));
    return (ERROR);


  case DPIO2_CMD_DUMP_DEV:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_DUMP_DEV (%d) is not implemented\n", DPIO2_CMD_DUMP_DEV));
    return (ERROR);


  case DPIO2_CMD_DUMP:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_DUMP (%d) is not implemented\n", DPIO2_CMD_DUMP));
    return (ERROR);


  case DPIO2_CMD_DMA_GET_STATUS:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_DMA_GET_STATUS (%d) is not implemented\n", DPIO2_CMD_DMA_GET_STATUS));
    return (ERROR);

    
  case DPIO2_CMD_GET_DEVICE_TYPE:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_GET_DEVICE_TYPE (%d) is not implemented\n", DPIO2_CMD_GET_DEVICE_TYPE));
    return (ERROR);


  case DPIO2_CMD_GET_DEVICE_ECO:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_GET_DEVICE_ECO (%d) is not implemented\n", DPIO2_CMD_GET_DEVICE_ECO));
    return (ERROR);


  case DPIO2_CMD_DMA_BSWAP_SET:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_DMA_BSWAP_SET (%d) is not implemented\n", DPIO2_CMD_DMA_BSWAP_SET));
    return (ERROR);


  case DPIO2_CMD_DMA_BSWAP_GET:
    DPIO2_ERROR_MSG(("The DPIO2_CMD_DMA_BSWAP_GET (%d) is not implemented\n", DPIO2_CMD_DMA_BSWAP_GET));
    return (ERROR);


  /* Frontend related commands.
   */
  case DPIO2_CMD_STROBE_FREQUENCY_SET:
    status = pDevice->pController->setOscillatorFrequency(*((UINT32*) argument), 
                                                          *((UINT32*) argument));
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_STROBE_FREQUENCY_RANGE_SET:
    status = pDevice->pController->
      setStrobeFrequencyRange((DPIO2_STROBE_FREQUENCY_RANGE) argument);
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT:
    if ( (BOOL) argument ) {
      
      pDevice->pController->enableClockingOnBothStrobeEdges();

    } else {

      pDevice->pController->disableClockingOnBothStrobeEdges();

    }
    break;


  case DPIO2_CMD_STROBE_GENERATION_ENABLE:
    pDevice->pController->enableStrobeGeneration();
    if ( argument == DPIO2_FIXED_OSCILLATOR ) {

      pDevice->pController->activateFixedOscillator();

    } else if ( argument == DPIO2_PROGRAMMABLE_OSCILLATOR ) {

      pDevice->pController->activateProgrammableOscillator();

    } else {

      DPIO2_ERROR_MSG(("DPIO2_CMD_STROBE_GENERATION_ENABLE: Invalid identifier (%d) for oscillator\n", argument));
      return (ERROR);

    }
    break;

        
  case DPIO2_CMD_STROBE_RECEPTION_ENABLE:
    pDevice->pController->disableStrobeGeneration();
    if ( argument == DPIO2_PRIMARY_STROBE ) {

      status = pDevice->pController->selectPrimaryStrobe();

    } else if ( argument == DPIO2_SECONDARY_STROBE ) {

      status = pDevice->pController->selectSecondaryStrobe();

    } else {

      DPIO2_ERROR_MSG(("DPIO2_CMD_STROBE_RECEPTION_ENABLE: Invalid identifier (%d) for strobe signal\n", argument));
      status = ERROR;

    }

    if ( status != OK ) {

      return (ERROR);

    }
    break;

        
  case DPIO2_CMD_FPDP_ACTIVATION_SELECT:
    if ( ((BOOL) argument) ) {

      pDevice->pController->startTransfer();

    } else {

      pDevice->pController->stopTransfer();

    }
    break;


  case DPIO2_CMD_SYNC_GENERATION_COUNTER_SET:
    status = pDevice->pController->setSyncGenerationCounter((UINT32) argument);
    if ( status != OK ) {
      
      return (ERROR);

    }
    break;


  case DPIO2_CMD_SYNC_GENERATION_SELECT:
    status = pDevice->pController->
      selectSyncGenerationMode((DPIO2_SYNC_GENERATION_MODE) argument);
    if ( status != OK ) {
      
      return (ERROR);

    }
    break;

        
  case DPIO2_CMD_SYNC_RECEPTION_SELECT:
    status = pDevice->pController->
      selectSyncReceptionMode((DPIO2_SYNC_RECEPTION_MODE) argument);
    if ( status != OK ) {
      
      return (ERROR);

    }
    break;


  case DPIO2_CMD_D0_TO_BE_USED_FOR_SYNC_SELECT:
    if ( ((BOOL) argument) ) {

      pDevice->pController->enableD0ToBeUsedForSync();

    } else {

      pDevice->pController->disableD0ToBeUsedForSync();

    }    
    break;


  case DPIO2_CMD_VIDEO_MODE_SELECT:
    if ( ((BOOL) argument) ) {

      status = pDevice->pController->enableVideoMode();

    } else {

      status = pDevice->pController->disableVideoMode();

    }
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_COUNTER_ADDRESSING_ENABLE:
    pCounterInfo = (DPIO2_COUNTER_ADDRESSING_INFO*) argument;
    status = pDevice->pController->enableCounterAddressing(pCounterInfo->initialSkipCount,
                                                           pCounterInfo->skipCount,
                                                           pCounterInfo->receiveCount);
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_COUNTER_ADDRESSING_DISABLE:
    status = pDevice->pController->disableCounterAddressing();
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_DVALID_ALWAYS_ASSERTED_SELECT:
    if ( ((BOOL) argument) ) {

      status = pDevice->pController->enableDvalidAlwaysAsserted();

    } else {

      status = pDevice->pController->disableDvalidAlwaysAsserted();

    }
    if ( status != OK ) {
      
      return (ERROR);

    }
    break;


  case DPIO2_CMD_DATA_SWAP_MODE_SELECT:
    status = pDevice->pController->setDataSwapMode((DPIO2_DATA_SWAP_MODE) argument);
    if ( status != OK ) {
      
      return (ERROR);

    }
    break;


  case DPIO2_CMD_DATA_PACKING_CAPABILITY_GET:
    if ( pDevice->pController->checkIs8Bit4BitPackingAvailable() ) {
      
      *((int*) argument) = DPIO2_8BIT_4BIT_PACKING_AVAILABLE;

    } else {

      *((int*) argument) = DPIO2_16BIT_10BIT_PACKING_AVAILABLE;

    }
    break;


  case DPIO2_CMD_DATA_PACKING_ENABLE:
    switch ( argument ) {

    case DPIO2_PACK_16_LSB_ON_FPDP:
      status = pDevice->pController->setPackingMode(PACK_16_LEAST_SIGNIFICANT_FPDP_BITS);
      break;

    case DPIO2_PACK_16_MSB_ON_FPDP:
      status = pDevice->pController->setPackingMode(PACK_16_MOST_SIGNIFICANT_FPDP_BITS);
      break;

    case DPIO2_PACK_4_LSB_ON_FPDP:
      status = pDevice->pController->setPackingMode(PACK_4_LEAST_SIGNIFICANT_FPDP_BITS);
      break;

    case DPIO2_PACK_8_LSB_ON_FPDP:
      status = pDevice->pController->setPackingMode(PACK_8_LEAST_SIGNIFICANT_FPDP_BITS);
      break;

    case DPIO2_PACK_10_LSB_ON_FPDP:
      status = pDevice->pController->setPackingMode(PACK_10_LEAST_SIGNIFICANT_FPDP_BITS);
      break;

    default:
      DPIO2_ERROR_MSG(("%d is not a valid mode for data packing\n", argument));
      status = ERROR;
      break;

    }

    if ( status != OK ) {
      
      return (ERROR);

    }
    break;


  case DPIO2_CMD_DATA_PACKING_PIPELINE_CHECK:
    status = pDevice->pController->checkIsPipelineNotEmpty(*((BOOL*) argument));
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_DATA_PACKING_PIPELINE_FLUSH:
    status = pDevice->pController->flushPackingPipeline();
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_DATA_PACKING_DISABLE:
    status = pDevice->pController->setPackingMode(NO_PACKING);
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enableSuspendFlowControl();

    } else {

      pDevice->pController->disableSuspendFlowControl();

    }
    break;

        
  case DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enableNrdyFlowControl();

    } else {

      pDevice->pController->disableNrdyFlowControl();

    }
    break;


  case DPIO2_CMD_RES1_DIRECTION_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enableRes1AsOutput();

    } else {

      pDevice->pController->disableRes1AsOutput();

    }
    break;


  case DPIO2_CMD_RES1_OUTPUT_VALUE_SET:
    if ( (BOOL) argument ) {

      pDevice->pController->setRes1OutputHigh();

    } else {

      pDevice->pController->setRes1OutputLow();

    }
    break;


  case DPIO2_CMD_RES1_VALUE_GET:
    *((BOOL*) argument) = pDevice->pController->checkIsRes1InputHigh();
    break;

        
  case DPIO2_CMD_RES2_DIRECTION_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enableRes2AsOutput();

    } else {

      pDevice->pController->disableRes2AsOutput();

    }
    break;


  case DPIO2_CMD_RES2_OUTPUT_VALUE_SET:
    if ( (BOOL) argument ) {

      pDevice->pController->setRes2OutputHigh();

    } else {

      pDevice->pController->setRes2OutputLow();

    }
    break;


  case DPIO2_CMD_RES2_VALUE_GET:
    *((BOOL*) argument) = pDevice->pController->checkIsRes2InputHigh();
    break;

        
  case DPIO2_CMD_RES3_DIRECTION_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enableRes3AsOutput();

    } else {

      pDevice->pController->disableRes3AsOutput();

    }
    break;


  case DPIO2_CMD_RES3_OUTPUT_VALUE_SET:
    if ( (BOOL) argument ) {

      pDevice->pController->setRes3OutputHigh();

    } else {

      pDevice->pController->setRes3OutputLow();

    }
    break;


  case DPIO2_CMD_RES3_VALUE_GET:
    *((BOOL*) argument) = pDevice->pController->checkIsRes3InputHigh();
    break;

        
  case DPIO2_CMD_PIO1_DIRECTION_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enablePio1AsOutput();

    } else {

      pDevice->pController->disablePio1AsOutput();

    }
    break;


  case DPIO2_CMD_PIO1_OUTPUT_VALUE_SET:
    if ( (BOOL) argument ) {

      pDevice->pController->setPio1OutputHigh();

    } else {

      pDevice->pController->setPio1OutputLow();

    }
    break;


  case DPIO2_CMD_PIO1_VALUE_GET:
    *((BOOL*) argument) = pDevice->pController->checkIsPio1InputHigh();
    break;

        
  case DPIO2_CMD_PIO2_DIRECTION_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enablePio2AsOutput();

    } else {

      pDevice->pController->disablePio2AsOutput();

    }
    break;


  case DPIO2_CMD_PIO2_OUTPUT_VALUE_SET:

    if ( (BOOL) argument ) {

      pDevice->pController->setPio2OutputHigh();

    } else {

      pDevice->pController->setPio2OutputLow();

    }
    break;


  case DPIO2_CMD_PIO2_VALUE_GET:
    *((BOOL*) argument) = pDevice->pController->checkIsPio2InputHigh();
    break;


  case DPIO2_CMD_TEST_PATTERN_GENERATION_ENABLE:
    status = pDevice->pController->enableTestPatternGeneration((DPIO2_TEST_PATTERN) argument);
    if ( status != OK ) {

      return (ERROR);

    }
    break;

        
  case DPIO2_CMD_TEST_PATTERN_GENERATION_DISABLE:
    pDevice->pController->disableTestPatternGeneration();
    break;


  case DPIO2_CMD_TEST_PATTERN_START_VALUE_SET:
    pDevice->pController->setTestPatternStartValue((UINT16) argument);
    break;


  case DPIO2_CMD_SUSPEND_ASSERTION_FORCE:
    if ( (BOOL) argument ) {

      status = pDevice->pController->enableForcedSuspendAssertion();

    } else {

      status = pDevice->pController->disableForcedSuspendAssertion();

    }
    if ( status != OK ) {

      return (ERROR);

    }
    break;


  case DPIO2_CMD_STROBE_SKEW_SET:
    status = pDevice->pController->setStrobeSkew(argument);
    if ( status != OK ) {

      return (ERROR);

    }
    break;

        
  case DPIO2_CMD_DEFAULT_STROBE_SKEW_SET:
    status = pDevice->pController->useDefaultStrobeSkew();
    if ( status != OK ) {

      return (ERROR);

    }
    break;

        
  /* DMA related commands.
   */
  case DPIO2_CMD_DMA_SET_DESC:

    if ( argument == 0 ) {

      DPIO2_ERROR_MSG(("Invalid Argument %0x.\n", argument));
      return ERROR;

    } else {

      pDescriptor = (DPIO2_DMA_DESCRIPTOR*) argument;

    }

    
    if ( pDevice->pController->checkIsOutput() ) {

      pDevice->outputDescriptor.m_transferLength = pDescriptor->blockSizeInBytes;
      pDevice->outputDescriptor.m_nextDescriptorId = pDescriptor->nextDescriptorId;
      pDevice->outputDescriptor.m_flagGenerateDmaBlockEndInterrupt 
        = pDescriptor->endOfBlockInterrupt;
      pDevice->outputDescriptor.m_flagEndOfDmaChain = pDescriptor->lastBlockInChain;
      pDevice->outputDescriptor.m_flagUse64BitsDataAccess = pDescriptor->useD64;
      pDevice->outputDescriptor.m_sourceAddress = pDescriptor->pciAddress;
      pDevice->outputDescriptor.m_flagNotLastBlockInFrame = pDescriptor->notEndOfFrame;

      status = pDevice->pController->setOutputDmaDescriptor(pDescriptor->descriptorId,
                                                            &pDevice->outputDescriptor);

    } else {

      pDevice->inputDescriptor.m_transferLength = pDescriptor->blockSizeInBytes;
      pDevice->inputDescriptor.m_nextDescriptorId = pDescriptor->nextDescriptorId;
      pDevice->inputDescriptor.m_flagGenerateDmaBlockEndInterrupt 
        = pDescriptor->endOfBlockInterrupt;
      pDevice->inputDescriptor.m_flagEndOfDmaChain = pDescriptor->lastBlockInChain;
      pDevice->inputDescriptor.m_flagUse64BitsDataAccess = pDescriptor->useD64;
      pDevice->inputDescriptor.m_destinationAddress = pDescriptor->pciAddress;

      status = pDevice->pController->setInputDmaDescriptor(pDescriptor->descriptorId,
                                                           &pDevice->inputDescriptor);

    }

    
    if ( status != OK ) {

      DPIO2_ERROR_MSG(("Failed to set DMA Descriptor\n"));
      return (ERROR);

    }

    break;


  case DPIO2_CMD_DMA_SET_START_DESCRIPTOR:
    status = pDevice->pController->setNextDescriptorInDmaChain((UINT32) argument);
    if ( status != OK ) {

      return (ERROR);

    }
    break;
    

  case DPIO2_CMD_DMA_START:
    pDevice->pController->startDmaTransfer();
    if ( pDevice->enableHalt && (pHaltFunction != NULL) ) {

      (*pHaltFunction)();

    }
    break;


  case DPIO2_CMD_DMA_SUSPEND:
    if ( !pDevice->pController->checkIsDmaTransferCompleted() ) {

      DPIO2_ERROR_MSG(("DMA controller is not running - cannot suspend it.\n"));
      return (ERROR);

    }

    pDevice->pController->suspendDmaTransfer();


    /* Return the ID of the Next Descriptor in the DMA chain
     * if a pointer is supplied by the argument.
     */
    if ( argument != 0 ) {

      *((UINT32*) argument) = pDevice->pController->getNextDescriptorInDmaChain();

    }
    break;


  case DPIO2_CMD_DMA_RESUME:
    if ( pDevice->pController->checkIsDmaTransferCompleted() ) {

      DPIO2_ERROR_MSG(("DMA controller is DONE.\n"));
#ifndef VMOSA
      errno = S_dpio_DPIO_DMA_ERR_CTRL_DONE;
#endif
      return (ERROR);

    }

    if ( pDevice->pController->checkIsDmaTransferSuspended() ) {

      DPIO2_ERROR_MSG(("DMA controller is already Running.\n"));
#ifndef VMOSA
      errno = S_dpio_DPIO_DMA_ERR_CTRL_NOT_RUNNING;
#endif
      return (ERROR);

    }

    /* Resume DMA controller 
     */
    pDevice->pController->resumeDmaTransfer();
    break;


  case DPIO2_CMD_DMA_ABORT:

    if ( pDevice->pController->checkIsDmaTransferCompleted() ) {

      DPIO2_ERROR_MSG(("DMA controller is DONE.\n"));
#ifndef VMOSA
      errno = S_dpio_DPIO_DMA_ERR_CTRL_DONE;
#endif
      return (ERROR);

    }

    pDevice->pController->stopDmaTransfer();

    /* Return the ID of the Next Descriptor in the DMA chain
     * if a pointer is supplied by the argument.
     */
    if ( argument != 0 ) {

      *((UINT32*) argument) = pDevice->pController->getNextDescriptorInDmaChain();

    }

    break;


  case DPIO2_CMD_FLUSH_ON_DMA_ABORT_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enableDmaFifoFlushOnStop();

    } else {

      pDevice->pController->disableDmaFifoFlushOnStop();

    }    
    break;


  case DPIO2_CMD_DMA_GET_DONE: 
    *((BOOL*) argument) = pDevice->pController->checkIsDmaTransferCompleted();
    break;


  case DPIO2_CMD_REG_GET_DEMAND_MD:
    *((BOOL*) argument) = pDevice->pController->checkIsDmaDemandModeEnabled();
    break;


  case DPIO2_CMD_REG_SET_DEMAND_MD:
    pDevice->pController->enableDmaDemandMode();
    break;

        
  case DPIO2_CMD_REG_CLR_DEMAND_MD:
    pDevice->pController->disableDmaDemandMode();
    break;


  case DPIO2_CMD_CONTINUE_ON_EOT_SELECT:
    if ( (BOOL) argument ) {

      pDevice->pController->enableContinueOnEndOfTransfer();

    } else {

      pDevice->pController->disableContinueOnEndOfTransfer();

    }
    break;

  case DPIO2_CMD_EOT_ENABLE:
    pDevice->pController->selectEndOfTransferMode((DPIO2_END_OF_TRANSFER_MODE) argument);
    pDevice->pController->enableEndOfTransfer();
    break;


  case DPIO2_CMD_EOT_DISABLE:
    pDevice->pController->selectEndOfTransferMode(DPIO2_NO_EOT);
    pDevice->pController->disableEndOfTransfer();
    break;


  case DPIO2_CMD_EOT_COUNT_ENABLE:
    status = pDevice->pController->enableEotSyncCount((UINT16) argument);
    if ( status != OK ) {

      return (ERROR);

    }
    break;
    

  case DPIO2_CMD_EOT_COUNT_DISABLE:
    status = pDevice->pController->disableEotSyncCount();
    if ( status != OK ) {

      return (ERROR);

    }
    break;
    

  case DPIO2_CMD_DMA_HALTP_EN:
    pDevice->enableHalt = TRUE;
    break;


  case DPIO2_CMD_DMA_HALTP_DIS:
    pDevice->enableHalt = FALSE;
    break;


  case DPIO2_CMD_REMAINING_BYTE_COUNT_GET:
    if ( argument == 0 ) {

      DPIO2_ERROR_MSG(("Invalid Argument %0x.\n", argument));
      return ERROR;

    } else {

      pByteCountInfo = (DPIO2_REMAINING_BYTE_COUNT_INFO*) argument;

      pDevice->pController->getRemainingByteCount(pByteCountInfo->byteCount,
                                                  pByteCountInfo->overflowFlag);

    }
    break;


  case DPIO2_CMD_TRANSFERRED_BYTE_COUNT_GET:
    if ( argument == 0 ) {

      DPIO2_ERROR_MSG(("Invalid Argument %0x.\n", argument));
      return ERROR;

    } else {

      pDevice->pController->getTransferredByteCount(*((UINT32*) argument));

    }
    break;


  /* Interrupt related commands.
   */
  case DPIO2_CMD_INTERRUPT_ENABLE:
    sourceId = mapDpio2ConditionToSource(argument);
    status = pDevice->pController->enableInterruptSource(sourceId);
    if ( status != OK ) {

      return (ERROR);

    }
    break;

    
  case DPIO2_CMD_INTERRUPT_DISABLE:
    sourceId = mapDpio2ConditionToSource(argument);
    status = pDevice->pController->disableInterruptSource(sourceId);
    if ( status != OK ) {

      return (ERROR);

    }
    break;

/* rsr this code has been moved to the other version of dpio2Ioctl
   
  case DPIO2_CMD_INTERRUPT_CALLBACK_ATTACH:
    pInterruptCallback = (DPIO2_INTERRUPT_CALLBACK*) argument;
    iCondition = pInterruptCallback->condition;
    sourceId = mapDpio2ConditionToSource(iCondition);

     * Remove callback action if one is already attached.
//   *
    pDevice->pController->
      removeInterruptAction(sourceId, &pDevice->dpio2CallbackAction[iCondition]);

     * Register the new callback function in the action,
     * and add the action to the interrupt source.

     *     
    pDevice->dpio2CallbackAction[iCondition].
      setCallbackFunction(pInterruptCallback->pCallbackFunction, pInterruptCallback->argument);

    pDevice->pController->
      addInterruptAction(sourceId, &pDevice->dpio2CallbackAction[iCondition]);
    break;

*/
    
  case DPIO2_CMD_INTERRUPT_CALLBACK_DETACH:
    iCondition = argument;
    sourceId = mapDpio2ConditionToSource(iCondition);

    /* Remove callback action if one is already attached.
     */
    pDevice->pController->
      removeInterruptAction(sourceId, &pDevice->dpio2CallbackAction[iCondition]);
    break;


  case DPIO2_CMD_AUTO_DISABLE_INTERRUPT_ENABLE:
    iCondition = argument;
    sourceId = mapDpio2ConditionToSource(iCondition);

    /* Try to remove the disable action to make sure that it isn't registered twice.
     */
    pDevice->pController->
      removeInterruptAction(sourceId, &pDevice->dpio2DisableAction[iCondition]);

    pDevice->pController->
      addInterruptAction(sourceId, &pDevice->dpio2DisableAction[iCondition]);
    break;


  case DPIO2_CMD_AUTO_DISABLE_INTERRUPT_DISABLE:
    iCondition = argument;
    sourceId = mapDpio2ConditionToSource(iCondition);

    pDevice->pController->
      removeInterruptAction(sourceId, &pDevice->dpio2DisableAction[iCondition]);
    break;


  /* FIFO related commands.
   */
  case DPIO2_CMD_FIFO_FLUSH:
    pDevice->pController->flushFifo();
    break;
    
  case DPIO2_CMD_GET_CURRENT_FIFO_STATUS:
    *((int*) argument) = (int) pDevice->pController->getFifoState();
    break;


  case DPIO2_CMD_RESET_OCCURRED_FLAGS:
    pDevice->pController->resetFifoHistoryFlags();
    break;
    

  case DPIO2_CMD_GET_FIFO_OVERFLOW_OCCURRED_FLAG:
    *((BOOL*) argument) = pDevice->pController->checkHasFifoOverflowed();
    break;


  case DPIO2_CMD_GET_FIFO_FULL_OCCURRED_FLAG:
    *((BOOL*) argument) = pDevice->pController->checkHasFifoBeenFull();
    break;


  case DPIO2_CMD_GET_FIFO_ALMOST_FULL_OCCURRED_FLAG:
    *((BOOL*) argument) = pDevice->pController->checkHasFifoBeenAlmostFull();
    break;


  case DPIO2_CMD_GET_FIFO_HALF_FULL_OCCURRED_FLAG:
    *((BOOL*) argument) = pDevice->pController->checkHasFifoBeenMoreThanHalfFull();
    break;


  case DPIO2_CMD_GET_FIFO_ALMOST_EMPTY_OCCURRED_FLAG:
    *((BOOL*) argument) = pDevice->pController->checkHasFifoBeenAlmostEmpty();
    break;


  case DPIO2_CMD_GET_FIFO_EMPTY_OCCURRED_FLAG:
    *((BOOL*) argument) = pDevice->pController->checkHasFifoBeenEmpty();
    break;


  /* Miscellaneous commands 
   */
  case DPIO2_CMD_LATENCY_TIMER_GET:
    *((UINT8*) argument) = pDevice->pController->getLatencyTimer();
    break;


  case DPIO2_CMD_LATENCY_TIMER_SET:
    pDevice->pController->setLatencyTimer((UINT8) argument);
    break;


  case DPIO2_CMD_GET_PTR_FIFO:
    *((UINT32*) argument) = pDevice->pController->getBaseAddressOfFifo();

    break;

  case DPIO2_CMD_GET_PTR_FLASH:
    *((UINT32*) argument) = pDevice->pController->getBaseAddressOfFlash();
    break;

  case DPIO2_CMD_GET_DEVICE_FSIZE:
    *((UINT32*) argument) = pDevice->pController->getFifoSize();
    break;


  case DPIO2_CMD_GET_DEVICE_BUSNUM:
    *((int*) argument) = (int) pDevice->pController->getPciBusNumber();
    break;


  case DPIO2_CMD_GET_DEVICE_DEVNUM:
    *((int*) argument) = (int) pDevice->pController->getPciDeviceNumber();
    break;


  default:
    DPIO2_ERROR_MSG(("Invalid DPIO2 ioctl command, %d.\n", command));
    return (ERROR);

  }

  return OK;

}


/******************************************************************************
* dpio2Ioctl -  DPIO2 driver control routine
*               Eventually this version of dpio2Ioctl will handle                                              //rsr
*               all commands requiring a pointer argument.                                                     //rsr
*
* DPIO2 driver control routine
*
*/
STATUS dpio2Ioctl (DPIO2_DEVICE* pDevice, int command, void* argument)
{

  DPIO2_DMA_DESCRIPTOR*   pDescriptor;

  DPIO2_INTERRUPT_SOURCE_ID   sourceId;

  int   iCondition;

  STATUS   status;

  DPIO2_INTERRUPT_CALLBACK*   pInterruptCallback;
    
//  DPIO2_COUNTER_ADDRESSING_INFO*   pCounterInfo;                                                             //rsr not used at present
    
//  DPIO2_REMAINING_BYTE_COUNT_INFO*   pByteCountInfo;                                                         //rsr not used at present

#ifdef RSR_DEBUG
  printf("In dpio2Ioctl - dpio2Ioctl (void*) version\n");
#endif

  switch (command) {

  case DPIO2_CMD_GET_PTR_FLASH:
    *((UINT32*) argument) = pDevice->pController->getBaseAddressOfFlash();
    break;

  case DPIO2_CMD_INTERRUPT_CALLBACK_ATTACH:
    pInterruptCallback = (DPIO2_INTERRUPT_CALLBACK*) argument;
    iCondition = pInterruptCallback->condition;
    sourceId = mapDpio2ConditionToSource(iCondition);

    /* Remove callback action if one is already attached.
     */
    pDevice->pController->
      removeInterruptAction(sourceId, &pDevice->dpio2CallbackAction[iCondition]);

    /* Register the new callback function in the action,
     * and add the action to the interrupt source.
     */

    pDevice->dpio2CallbackAction[iCondition].
      setCallbackFunction(pInterruptCallback->pCallbackFunction, pInterruptCallback->argument);

    pDevice->pController->
      addInterruptAction(sourceId, &pDevice->dpio2CallbackAction[iCondition]);
    break;

  case DPIO2_CMD_STROBE_FREQUENCY_SET:
    status = pDevice->pController->setOscillatorFrequency(*((UINT32*) argument), 
                                                          *((UINT32*) argument));
    if ( status != OK ) {

      return (ERROR);

    }
    break;

  case DPIO2_CMD_DMA_SET_DESC:

  printf("\nCase DPIO2_CMD_DMA_SET_DESC; contents of the descriptor are:\n\n");

    if ( argument == 0 ) {

      DPIO2_ERROR_MSG(("Invalid Argument %0x.\n", argument));
      return ERROR;

    } else {

      pDescriptor = (DPIO2_DMA_DESCRIPTOR*) argument;

    }
    printf("Descriptor ID = %x\n", pDescriptor->descriptorId);
    printf("Next descriptor ID = %x\n", pDescriptor->nextDescriptorId);
    printf("Blocksize = %u\n", pDescriptor->blockSizeInBytes);
    printf("pci address = %x\n", pDescriptor->pciAddress);
    printf("Last block in chain flag = %x\n", pDescriptor->lastBlockInChain);
    printf("Not end of frame flag = %x\n", pDescriptor->notEndOfFrame);


    
    if ( pDevice->pController->checkIsOutput() ) {

      pDevice->outputDescriptor.m_transferLength = pDescriptor->blockSizeInBytes;
      pDevice->outputDescriptor.m_nextDescriptorId = pDescriptor->nextDescriptorId;
      pDevice->outputDescriptor.m_flagGenerateDmaBlockEndInterrupt 
        = pDescriptor->endOfBlockInterrupt;
      pDevice->outputDescriptor.m_flagEndOfDmaChain = pDescriptor->lastBlockInChain;
      pDevice->outputDescriptor.m_flagUse64BitsDataAccess = pDescriptor->useD64;
      pDevice->outputDescriptor.m_sourceAddress = pDescriptor->pciAddress;
      pDevice->outputDescriptor.m_flagNotLastBlockInFrame = pDescriptor->notEndOfFrame;

      status = pDevice->pController->setOutputDmaDescriptor(pDescriptor->descriptorId,
                                                            &pDevice->outputDescriptor);

    } else {

      pDevice->inputDescriptor.m_transferLength = pDescriptor->blockSizeInBytes;
      pDevice->inputDescriptor.m_nextDescriptorId = pDescriptor->nextDescriptorId;
      pDevice->inputDescriptor.m_flagGenerateDmaBlockEndInterrupt 
        = pDescriptor->endOfBlockInterrupt;
      pDevice->inputDescriptor.m_flagEndOfDmaChain = pDescriptor->lastBlockInChain;
      pDevice->inputDescriptor.m_flagUse64BitsDataAccess = pDescriptor->useD64;
      pDevice->inputDescriptor.m_destinationAddress = pDescriptor->pciAddress;

      status = pDevice->pController->setInputDmaDescriptor(pDescriptor->descriptorId,
                                                           &pDevice->inputDescriptor);

    }

    
    if ( status != OK ) {

      DPIO2_ERROR_MSG(("Failed to set DMA Descriptor\n"));
      return (ERROR);

    }

    break;

  default:
    DPIO2_ERROR_MSG(("Invalid DPIO2 ioctl command in the unsigned long version, %d.\n", command));
    return (ERROR);

  }

  return OK;

}

#if defined(VXWORKS) || defined(WIN32)
/******************************************************************************
* dpio2DpioApiIoctl -  DPIO2 driver control routine conforming to the DPIO API
*
* RETURNS: OK if successfull, ERROR if not.
*
* ERRNO:
* INVALID_ARG - If invalid argument.
* DMA_DONE - If DMA controller not running.
* DMA_ENABLED - If DMA controller is already enabled.
*/
STATUS dpio2DpioApiIoctl(DPIO2_DPIO_API_DEVICE* pDevice, int command, int argument)
{

  UINT8   currentRegisterValue;

  DPIO_DMA_DESC* pDpioDesc;

  DPIO_DMA_STATUS*   pDpioDmaStatus;

  DPIO_LMEM_ACCESS*   pDpioLocalMemoryAccess;
  UINT32*             pLocalMemoryLocation;

  DPIO_INT_ACTION*   pAction;
  DPIO2_INTERRUPT_SOURCE_ID   sourceId;

  int   personalityModuleId;
  int   mainBoardId;

  UINT32   fifoSize;
  int      mostSignificantBit;

  STATUS   status;


  switch (command) {


  /* Register control commands.
   */
  case DPIO_CMD_REG_SET_MODE:
    updateDpioModeRegister(pDevice->pController,
                           pDevice->dpioModeRegisterValue,
                           *((UINT8*) argument),
                           pDevice->initialOscillatorFrequency);

    pDevice->dpioModeRegisterValue = *((UINT8*) argument);
    break;


  case DPIO_CMD_REG_GET_MODE:

    *((UINT8*) argument) = pDevice->dpioModeRegisterValue;
    break;


  case DPIO_CMD_REG_SET_PIO_CTR:

    updateDpioPioControlRegister(pDevice->pController,
                                 pDevice->dpioPioControlRegisterValue,
                                 *((UINT8*) argument));

    pDevice->dpioPioControlRegisterValue = *((UINT8*) argument);
    break;


  case DPIO_CMD_REG_GET_PIO_CTR:

    *((UINT8*) argument) = pDevice->dpioPioControlRegisterValue;
    break;


  case DPIO_CMD_REG_SET_INT_CTR:

    currentRegisterValue = buildDpioInterruptControlRegister(pDevice->pController);

    updateDpioInterruptControlRegister(pDevice->pController,
                                       currentRegisterValue,
                                       *((UINT8*) argument));
    break;
        

  case DPIO_CMD_REG_GET_INT_CTR:

    *((UINT8*) argument) = buildDpioInterruptControlRegister(pDevice->pController);
    break;


  case DPIO_CMD_REG_GET_DEMAND_MD:

    *((BOOL*) argument) = pDevice->pController->checkIsDmaDemandModeEnabled();
    break;


  case DPIO_CMD_REG_SET_DEMAND_MD:

    pDevice->pController->enableDmaDemandMode();
    break;

        
  case DPIO_CMD_REG_CLR_DEMAND_MD:

    pDevice->pController->disableDmaDemandMode();
    break;


  case DPIO_CMD_LBUS_TIMEOUT_EN:
    
    DPIO2_ERROR_MSG(("Time out on Local Bus is not implemented on DPIO2.\n"));
    break;


  case DPIO_CMD_LBUS_TIMEOUT_DIS:

    DPIO2_ERROR_MSG(("Time out on Local Bus is not implemented on DPIO2.\n"));
    break;


  case DPIO_CMD_EOT_ENABLE:

    if ( argument != 0 ) {

      pDevice->pController->selectEndOfTransferMode(DPIO2_PIO1_MARKS_EOT);

      pDevice->dpioDmaControlRegisterValue |= (SEL_EOT | PIO1_EOT);

    } else {

      DPIO2_WARNING_MSG(("SYNC will always generate EOT (no jumper on DPIO2 to select RES1 for EOT)\n"), TRUE);

      pDevice->pController->selectEndOfTransferMode(DPIO2_SYNC_MARKS_EOT);

      pDevice->dpioDmaControlRegisterValue |= (SEL_EOT & ~PIO1_EOT);

    }
    pDevice->pController->enableEndOfTransfer();

    break;


  case DPIO_CMD_EOTB3_ENABLE:

    pDevice->dpioDmaControlRegisterValue &= ~(SEL_EOT0 | SEL_EOT1);

    switch (argument) {

    case 0:
      DPIO2_WARNING_MSG(("SYNC will always generate EOT (no jumper on DPIO2 to select RES1 for EOT)\n"), TRUE);

      pDevice->pController->selectEndOfTransferMode(DPIO2_SYNC_MARKS_EOT);
      pDevice->pController->enableEndOfTransfer();

      pDevice->dpioDmaControlRegisterValue |= SEL_EOT1;
      break;

    case 1:
      pDevice->pController->selectEndOfTransferMode(DPIO2_PIO1_MARKS_EOT);
      pDevice->pController->enableEndOfTransfer();

      pDevice->dpioDmaControlRegisterValue |= (SEL_EOT1 | SEL_EOT0);
      break;

    case 2:
      pDevice->pController->selectEndOfTransferMode(DPIO2_PIO2_MARKS_EOT);
      pDevice->pController->enableEndOfTransfer();

      pDevice->dpioDmaControlRegisterValue |= SEL_EOT0;
      break;

    default:
      DPIO2_ERROR_MSG(("%d is not a valid argument to DPIO_CMD_EOTB3_ENABLE\n", argument));
      break;

    }       

  case DPIO_CMD_EOT_DISABLE:

    pDevice->pController->selectEndOfTransferMode(DPIO2_NO_EOT);
    pDevice->pController->disableEndOfTransfer();

    pDevice->dpioDmaControlRegisterValue &= ~(SEL_EOT0 | SEL_EOT1);
    break;
    

  case DPIO_CMD_REG_SET_DMA_CTR:

    updateDpioDmaControlRegister(pDevice->pController,
                                 pDevice->dpioDmaControlRegisterValue,
                                 *((UINT8*) argument));

    pDevice->dpioDmaControlRegisterValue = *((UINT8*) argument);
    break;
  
          
  case DPIO_CMD_REG_GET_DMA_CTR:

    *((UINT8*) argument) = pDevice->dpioDmaControlRegisterValue;

    if ( pDevice->pController->checkIsDirSignalHigh() ) {

      *((UINT8*) argument) |= IDIR;

    } else {

      *((UINT8*) argument) &= ~IDIR;

    }
    break;


  case DPIO_CMD_REG_GET_FIFO_STAT:

    *((UINT8*) argument) = buildDpioFifoStatusRegister(pDevice->pController);
    break;

  
  case DPIO_CMD_REG_GET_PIO_STAT:
    
    *((UINT8*) argument) = buildDpioPioStatusRegister(pDevice->pController);
    break;


  case DPIO_CMD_REG_GET_INT_STAT:

    *((UINT8*) argument) = buildDpioInterruptStatusRegister(pDevice->pController);
    break;


  case DPIO_CMD_REG_GET_MOD_ID:

    *((UINT8*) argument) = buildDpioModuleIdRegister(pDevice->pController);
    break;
        

  case DPIO_CMD_RESET_OCCURRED_FLAGS:

    pDevice->pController->resetFifoHistoryFlags();

    pDevice->dpioModeRegisterValue |= DPIO_RSTOC;
    break;
    

  /* DMA Controller Commands */
#ifdef __GNUC__
  case DPIO_CMD_DMA_HALTP_EN:
    pDevice->enableHalt = TRUE;
    break;

  case DPIO_CMD_DMA_HALTP_DIS:
    pDevice->enableHalt = FALSE;
    break;
#endif


  case DPIO_CMD_DMA_SET_DESC:

    if ( argument == 0 ) {

      DPIO2_ERROR_MSG(("Invalid Argument %0x.\n", argument));
      errno = S_dpio_DPIO_DMA_ERR_INVALID_ARG;
      return ERROR;

    } else {

      pDpioDesc = (DPIO_DMA_DESC*) argument;

    }


    /* Check that the descriptor ID is valid */
    if ( pDpioDesc->descID == 0 ) {

      DPIO2_ERROR_MSG(("DMA Block Descriptor Identifier is %d.\n", (int)pDpioDesc->descID));
      errno = S_dpio_DPIO_DMA_ERR_INVALID_DESC_ID;
      return ERROR;

    }


    if ( pDevice->pController->checkIsOutput() ) {

      /* If this is not the first descriptor of a DMA chain, 
       * check the assert SYNC flag of the previous descriptor.
       * If the flag of the previous descriptor had the inverse
       * value of the flag in the current descriptor, 
       * the Not-Frame-End flag in the previous descriptor must be cleared.
       */
      if ( pDevice->previousDescriptorId != 0 ) {

	if ( pDevice->previousAssertSyncFlag != pDpioDesc->AssertSnc ) {

	  pDevice->outputDescriptor.m_flagNotLastBlockInFrame = FALSE;

	  pDevice->pController->setOutputDmaDescriptor(pDevice->previousDescriptorId,
                                                       &pDevice->outputDescriptor);

	}

      }


      /* The Not-Frame-End flag of the current descriptor must be cleared
       * if the assert SYNC flag is set and the descriptor is the
       * last in a DMA chain.  This ensures that SYNC will not be
       * generated for the data represented by the first descriptor
       * in the next DMA chain defined.
       */
      pDevice->outputDescriptor.m_flagNotLastBlockInFrame 
	= !(pDpioDesc->AssertSnc && (pDpioDesc->nextDescID == 0));

      pDevice->outputDescriptor.m_sourceAddress = pDpioDesc->pciAddr;
      pDevice->outputDescriptor.m_transferLength = pDpioDesc->blockSize;
      pDevice->outputDescriptor.m_nextDescriptorId = pDpioDesc->nextDescID;
      pDevice->outputDescriptor.m_flagGenerateDmaBlockEndInterrupt = pDpioDesc->intEndOfBlock;
      pDevice->outputDescriptor.m_flagUse64BitsDataAccess = FALSE;
      pDevice->outputDescriptor.m_flagEndOfDmaChain = (pDpioDesc->nextDescID == 0);


      pDevice->pController->setOutputDmaDescriptor(pDpioDesc->descID,
                                                   &pDevice->outputDescriptor);

      
      /* If the current descriptor is the last in a DMA chain, 
       * store zero as the previous descriptor ID to signal
       * that the next descriptor is the first in a new DMA chain.
       * If the current descriptor is not the last in a DMA chain,
       * just store the ID of the current descriptor and the value
       * of its assert SYNC flag.
       */
      if ( pDpioDesc->nextDescID == 0 ) {

	pDevice->previousDescriptorId = 0;

      } else {

	pDevice->previousDescriptorId = pDpioDesc->descID;
	pDevice->previousAssertSyncFlag = pDpioDesc->AssertSnc;

      }

    } else {

      pDevice->inputDescriptor.m_destinationAddress = pDpioDesc->pciAddr;
      pDevice->inputDescriptor.m_transferLength = pDpioDesc->blockSize;
      pDevice->inputDescriptor.m_nextDescriptorId = pDpioDesc->nextDescID;
      pDevice->inputDescriptor.m_flagGenerateDmaBlockEndInterrupt = pDpioDesc->intEndOfBlock;
      pDevice->inputDescriptor.m_flagUse64BitsDataAccess = FALSE;
      pDevice->inputDescriptor.m_flagEndOfDmaChain = (pDpioDesc->nextDescID == 0);

      pDevice->pController->setInputDmaDescriptor(pDpioDesc->descID,
                                                  &pDevice->inputDescriptor);
    }

    break;


  case DPIO_CMD_DMA_START:
    if ( dpio2DmaStart(pDevice, (DPIO_DMA_START_DESC*) argument) != OK ) {
      
      return (ERROR);

    }
    break;


  case DPIO_CMD_DMA_SUSPEND:

    if ( !pDevice->pController->checkIsDmaTransferCompleted() ) {

      DPIO2_ERROR_MSG(("DMA controller is NOT Running.\n"));
      errno = S_dpio_DPIO_DMA_ERR_CTRL_NOT_RUNNING;
      return (ERROR);

    }

    pDevice->pController->suspendDmaTransfer();

    /* Return the ID of the Next Descriptor in the DMA chain
     * if a pointer is supplied by the argument.
     */
    if ( argument != 0 ) {

      *((UINT16*) argument) = pDevice->pController->getNextDescriptorInDmaChain();

    }
    break;


  case DPIO_CMD_DMA_RESUME:

    if ( pDevice->pController->checkIsDmaTransferCompleted() ) {

      DPIO2_ERROR_MSG(("DMA controller is DONE.\n"));
      errno = S_dpio_DPIO_DMA_ERR_CTRL_DONE;
      return (ERROR);

    }

    if ( pDevice->pController->checkIsDmaTransferSuspended() ) {

      DPIO2_ERROR_MSG(("DMA controller is already Running.\n"));
      errno = S_dpio_DPIO_DMA_ERR_CTRL_NOT_RUNNING;
      return (ERROR);

    }

    /* Return the ID of the Next Descriptor in the DMA chain
     * if a pointer is supplied by the argument.
     */
    if ( argument != 0 ) {


      *((UINT16*) argument) = pDevice->pController->getNextDescriptorInDmaChain();

    }

    /* Resume DMA controller 
     */
    pDevice->pController->resumeDmaTransfer();
    break;


  case DPIO_CMD_DMA_ABORT:

    if ( pDevice->pController->checkIsDmaTransferCompleted() ) {

      DPIO2_ERROR_MSG(("DMA controller is DONE.\n"));
      errno = S_dpio_DPIO_DMA_ERR_CTRL_DONE;
      return (ERROR);

    }

    pDevice->pController->stopDmaTransfer();

    /* Return the ID of the Next Descriptor in the DMA chain
     * if a pointer is supplied by the argument.
     */
    if ( argument != 0 ) {

      *((UINT16*) argument) = pDevice->pController->getNextDescriptorInDmaChain();

    }

    break;


  case DPIO_CMD_DMA_GET_STATUS:

    /* Check that a pointer is given as argument.
     */
    if ( argument == 0 ) {

      DPIO2_ERROR_MSG(("Invalid Argument %d.\n", argument));
      errno = S_dpio_DPIO_DMA_ERR_INVALID_ARG;
      return (ERROR);

    }

    
    pDpioDmaStatus = (DPIO_DMA_STATUS*) argument;

    
    /* Read error bits from the Device Status register in Configuration Space.
     */
    pDpioDmaStatus->errorFlag = 0;

    if ( pDevice->pController->checkHasMasterDataParityErrorOccurred() ) {

      pDpioDmaStatus->errorFlag |= DPIO_DMAErr_MASTER_PARITY_ERROR;

    }

    if ( pDevice->pController->checkIsTargetAbortSignaled() ) {

      pDpioDmaStatus->errorFlag |= DPIO_DMAErr_TARGET_ABORT_TARGET;

    }

    if ( pDevice->pController->checkIsTargetAbortReceived() ) {

      pDpioDmaStatus->errorFlag |= DPIO_DMAErr_TARGET_ABORT_MASTER;

    }

    if ( pDevice->pController->checkIsMasterAbortReceived() ) {

      pDpioDmaStatus->errorFlag |= DPIO_DMAErr_MASTER_ABORT;

    }

    if ( pDevice->pController->checkIsSystemErrorSignaled() ) {

      pDpioDmaStatus->errorFlag |=  DPIO_DMAErr_SERR;

    }


    pDpioDmaStatus->currentDescID = (UINT16) pDevice->pController->getNextDescriptorInDmaChain();

    pDpioDmaStatus->done = pDevice->pController->checkIsDmaTransferCompleted();
    break;

    
  case DPIO_CMD_DMA_GET_DONE: 

    *((BOOL*) argument) = pDevice->pController->checkIsDmaTransferCompleted();
    break;


  case DPIO_CMD_DMA_INT_ENABLE:

    pDevice->pController->enableInterruptSource(DPIO2_DMA_DONE_INTERRUPT);
    break;


  case DPIO_CMD_DMA_INT_DISABLE:  /* Disable DMA interrupt */

    pDevice->pController->disableInterruptSource(DPIO2_DMA_DONE_INTERRUPT);
    break;


  case DPIO_CMD_LMEM_UINT32_WRITE:
    pDpioLocalMemoryAccess = (DPIO_LMEM_ACCESS*) argument;

    pLocalMemoryLocation = (UINT32*) (pDevice->pController->getBaseAddressOfSram()
                                      + pDpioLocalMemoryAccess->address);

    *pLocalMemoryLocation = convertToLittleEndian(*((UINT32*) pDpioLocalMemoryAccess->value));
    break;


  case DPIO_CMD_LMEM_UINT32_READ:

    pDpioLocalMemoryAccess = (DPIO_LMEM_ACCESS*) argument;

    pLocalMemoryLocation = (UINT32*) (pDevice->pController->getBaseAddressOfSram()
                                      + pDpioLocalMemoryAccess->address);

    *((UINT32*) pDpioLocalMemoryAccess->value) = convertFromLittleEndian(*pLocalMemoryLocation);
    break;


  case DPIO_CMD_PLX_UINT32_WRITE: 
    
    DPIO2_ERROR_MSG(("The command 'DPIO_CMD_PLX_UINT32_WRITE' is no supported\n"));
    return (ERROR);


  case DPIO_CMD_PLX_UINT32_READ:

    DPIO2_ERROR_MSG(("The command 'DPIO_CMD_PLX_UINT32_READ' is no supported\n"));
    return (ERROR);


  case DPIO_CMD_SET_INT_ACTION:
    
    if ( argument == 0 ) {

      DPIO2_ERROR_MSG(("Invalid Argument %0x.\n", argument));
      errno = S_dpio_DPIO_MISC_ERR_INVALID_ARG;
      return (ERROR);

    }

    pAction = (DPIO_INT_ACTION*) argument;
    sourceId = mapDpioConditionToSource(pAction->condition);

    switch ( pAction->action ) {

    case DPIO_INT_ACTION_DISABLE:
      if ( pAction->descriptor == DPIO_INT_ACTION_NO_OP ) {

        pDevice->pController->
          removeInterruptAction(sourceId, &pDevice->dpioDisableAction[pAction->condition]);

      } else if ( pAction->descriptor == DPIO_INT_ACTION_INT_DIS ) {

        pDevice->pController->
          addInterruptAction(sourceId, &pDevice->dpioDisableAction[pAction->condition]);

      } else {

        DPIO2_ERROR_MSG(("The specified action descriptor (%d) is not valid\n", 
                   pAction->descriptor));
        errno = S_dpio_DPIO_MISC_ERR_INVALID_ARG;
        return (ERROR);

      }
      break;


    case DPIO_INT_ACTION_DMA:
      /* Remove the currently registered DMA action.
       */
      if ( pDevice->pDpioCurrentDmaAction[pAction->condition] != NULL ) {

        pDevice->pController->
          removeInterruptAction(sourceId, pDevice->pDpioCurrentDmaAction[pAction->condition]);

        pDevice->pDpioCurrentDmaAction[pAction->condition] = NULL;

      }

      switch ( pAction->descriptor ) {

      case DPIO_INT_DESCRIPTOR_DMA_NOOP:
        /* No new action to be registered.
         */
        break;

      case DPIO_INT_DESCRIPTOR_DMA_START:
        pDevice->pDpioCurrentDmaAction[pAction->condition] 
          = &pDevice->dpioStartDmaAction[pAction->condition];
        break;

      case DPIO_INT_DESCRIPTOR_DMA_SUSPEND:
        pDevice->pDpioCurrentDmaAction[pAction->condition] 
          = &pDevice->dpioSuspendDmaAction[pAction->condition];
        break;

      case DPIO_INT_DESCRIPTOR_DMA_RESUME:
        pDevice->pDpioCurrentDmaAction[pAction->condition] 
          = &pDevice->dpioResumeDmaAction[pAction->condition];
        break;

      case DPIO_INT_DESCRIPTOR_DMA_ABORT:
        pDevice->pDpioCurrentDmaAction[pAction->condition] 
          = &pDevice->dpioStopDmaAction[pAction->condition];
        break;

      default:
        DPIO2_ERROR_MSG(("The specified action descriptor (%d) is not valid\n", 
                   pAction->descriptor));
        errno = S_dpio_DPIO_MISC_ERR_INVALID_ARG;
        return (ERROR);

      }

      
      /* Register the new DMA action.
       */
      if ( pDevice->pDpioCurrentDmaAction[pAction->condition] != NULL ) {

        pDevice->pController->
          addInterruptAction(sourceId, pDevice->pDpioCurrentDmaAction[pAction->condition]);

      }
      break;


    case DPIO_INT_ACTION_CALL_ACTION_HOOK:
      /* Remove the currently registered callback action.
       */
      pDevice->pController->
        removeInterruptAction(sourceId, &pDevice->dpioCallbackAction[pAction->condition]);
      
      /* If a new callback function is specified, 
       * store it and re-register the callback action.
       */
      if ( pAction->descriptor != 0 ) {

        pDevice->dpioCallbackAction[pAction->condition].
          setCallbackFunction((FUNCPTR) pAction->descriptor, pAction->call_action_hook_parameter);

        pDevice->pController->
          addInterruptAction(sourceId, &pDevice->dpioCallbackAction[pAction->condition]);
        
      }
      break;

    default:
      DPIO2_ERROR_MSG(("The specified action type (%d) is unknown\n", pAction->action));
      errno = S_dpio_DPIO_MISC_ERR_INVALID_ARG;
      return (ERROR);

    }
    break;


  /* Config space commands */
  case DPIO_CMD_LATENCY_TIMER_GET:
    *((UINT8*) argument) = pDevice->pController->getLatencyTimer();
    break;


  case DPIO_CMD_LATENCY_TIMER_SET:
    pDevice->pController->setLatencyTimer((UINT8) argument);
    break;


  case DPIO_CMD_REINIT:
    if ( dpio2DpioApiInitHardware(pDevice) != OK ) {

      return (ERROR);

    } else {

      /* Re-enable interrupts. */
      pDevice->pController->enableFrontEndInterrupts();

    }
    break;


  /* Get pointers to registers for direct access */
  case DPIO_CMD_GET_PTR_CTRL_REGS:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_CTRL_REGS command (%d) is not implemented.\n", 
               DPIO_CMD_GET_PTR_CTRL_REGS));
    return (ERROR);


  case DPIO_CMD_GET_PTR_STAT_REGS:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_STAT_REGS command (%d) is not implemented.\n", 
               DPIO_CMD_GET_PTR_STAT_REGS));
    return (ERROR);


  case DPIO_CMD_GET_PTR_PCI9060_REGS:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_PCI9060_REGS command (%d) is not implemented.\n", 
               DPIO_CMD_GET_PTR_PCI9060_REGS));
    return (ERROR);


  case DPIO_CMD_GET_PTR_FIFO:
    *((UINT32*) argument) = pDevice->pController->getBaseAddressOfFifo();
    break;


  case DPIO_CMD_GET_PTR_SRAM:
    *((UINT32*) argument) = pDevice->pController->getBaseAddressOfSram();
    break;


  case DPIO_CMD_GET_PTR_RUNTIME_BASE:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_PTR_RUNTIME_BASE command (%d) is not implemented.\n", 
               DPIO_CMD_GET_PTR_RUNTIME_BASE));
    return (ERROR);


  /* Get info about the DPIO */
  case DPIO_CMD_GET_DEVICE_TYPE:
    personalityModuleId = pDevice->pController->getPersonalityId();
    mainBoardId = pDevice->pController->getMainBoardId();

    *((UINT8*) argument) = (mainBoardId << 4) | personalityModuleId;
    if ( pDevice->pController->checkIsOutput() ) {

      *((UINT8*) argument) |= 0x10;

    }
    break;


  case DPIO_CMD_GET_DEVICE_ECO:
    DPIO2_ERROR_MSG(("The DPIO_CMD_GET_DEVICE_ECO command (%d) is not implemented.\n", 
               DPIO_CMD_GET_DEVICE_ECO));
    return (ERROR);


  case DPIO_CMD_GET_DEVICE_FSIZE:
    /* The bit number of the most significant bit in the size of the FIFO is returned.
     * This is done in order to stay compatible with the DPIO driver.
     */
    fifoSize = pDevice->pController->getFifoSize();

    mostSignificantBit = 31;
    while ( (fifoSize & (1 << mostSignificantBit)) == 0 )
    {
      mostSignificantBit--;
    }

    *(UINT8*) argument = mostSignificantBit;
    break;


  case DPIO_CMD_GET_DEVICE_BUSNUM:
    *((int*) argument) = pDevice->pController->getPciBusNumber();
    break;


  case DPIO_CMD_GET_DEVICE_DEVNUM:
    *((int*) argument) = pDevice->pController->getPciDeviceNumber();
    break;


  case DPIO_CMD_DMA_BSWAP_SET:
    if ( *((BOOL*) argument) ) {

      pDevice->pController->setDataSwapMode(SWAP_8BIT);
      pDevice->byteSwapIsEnabled = TRUE;

    } else {

      pDevice->pController->setDataSwapMode(NO_SWAP);
      pDevice->byteSwapIsEnabled = FALSE;

    }
    break;


  case DPIO_CMD_DMA_BSWAP_GET:
    *((BOOL*) argument) = pDevice->byteSwapIsEnabled;
    break;


  case DPIO2_CMD_VIDEO_MODE_SELECT:
    if ( ((BOOL) argument) ) {

      status = pDevice->pController->enableVideoMode();

    } else {

      status = pDevice->pController->disableVideoMode();

    }
    if ( status != OK ) {

      return (ERROR);

    }
    break;

        
  case DPIO2_CMD_FIFO_FLUSH:
    pDevice->pController->flushFifo();
    break;
    

  default:
    DPIO2_ERROR_MSG(("Invalid DPIO ioctl cmd, %0x.\n", command));
    errno = S_dpio_DPIO_MISC_ERR_INVALID_IOCTL_CMD;
    return ERROR;

  }


  return OK;

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
void updateDpioModeRegister(CDpio2DriverController* pController,
                            UINT8 currentValue,
                            UINT8 newValue,
                            UINT32 initialOscillatorFrequency)
{

  UINT8    changeBitMask;
  UINT32   resultingFrequency;


  changeBitMask = currentValue ^ newValue;

  /* Only process this register setting if the new value is different
   * from the previous.
   */
  if ( changeBitMask != 0 ) {

    /* First process the control bits which are common for input and output
       */
    if ( (changeBitMask & START) != 0 ) {

      if ( (newValue & START) != 0 ) {

        pController->startTransfer();

      } else {

        pController->stopTransfer();

      }

    }


    if ( (changeBitMask & DPIO_SELREG) != 0 ) {

      DPIO2_WARNING_MSG(("Bit 1 in DPIO Mode Register is not implemented for DPIO2.\n"), TRUE);

    }

      
    if ( (changeBitMask & DPIO_RSTOC) != 0 ) {

      if ( (newValue & DPIO_RSTOC) != 0 ) {

        pController->resetFifoHistoryFlags();

      }

    }


    if ( (changeBitMask & SUSPDIS) != 0 ) {

      if ( (newValue & SUSPDIS) != 0 ) {

        pController->disableSuspendFlowControl();

      } else {

        pController->enableSuspendFlowControl();

      }

    }


    if ( pController->checkIsOutput() ) {

      /* Process the control bits that are only defined for DPIO output modules.
       */
      if ( (changeBitMask & DSRC) != 0 ) {

        if ( (newValue & DSRC) != 0 ) {

          pController->enableStrobeGeneration();

        } else {

          pController->disableStrobeGeneration();
            
        }

      }


      if ( (changeBitMask & USYNC) != 0 ) {

        if ( (newValue & USYNC) != 0 ) {

          pController->selectSyncGenerationMode(DPIO2_SYNC_BEFORE_DATA);

        } else {

          pController->selectSyncGenerationMode(DPIO2_NO_SYNC_GENERATION);

        }

      }

        
      /* Check whether the clock select bits (bit 7 and 6) have changed,
       * and update the clock generation if they have changed.
       */
      if ( (changeBitMask & 0xc0) != 0 ) {

        if ( (newValue & 0xc0) == CKSEL_OSC ) {

          pController->setOscillatorFrequency(initialOscillatorFrequency, resultingFrequency);

        } else if ( (newValue & 0xc0) == CKSEL_OSC_DIV_2 ) {

          pController->setOscillatorFrequency(initialOscillatorFrequency / 2, resultingFrequency);

        } else if ( (newValue & 0xc0) == CKSEL_OSC_DIV_4 ) {

          pController->setOscillatorFrequency(initialOscillatorFrequency / 4, resultingFrequency);

        } else {

          pController->setOscillatorFrequency(initialOscillatorFrequency / 8, resultingFrequency);

        }

      }

    } else {
        
      /* Process the control bits that are only defined for DPIO input modules.
         */
      if ( (changeBitMask & ASYNCCK) != 0 ) {

        if ( (newValue & ASYNCCK) != 0 ) {

          pController->selectAsynchronousClock();

        } else {

          pController->selectFreeRunningClock();

        }

      }


      if ( (changeBitMask & WFSYNC) != 0 ) {

        if ( (newValue & WFSYNC) != 0 ) {

          pController->selectSyncReceptionMode(DPIO2_SYNC_STARTS_DATA_RECEPTION);

        } else {

          pController->selectSyncReceptionMode(DPIO2_NO_SYNC_HANDLING);

        }

      }

        
      if ( (changeBitMask & PECLSTROB) != 0 ) {

        if ( (newValue & PECLSTROB) != 0 ) {

          pController->selectSecondaryStrobe();

        } else {

          pController->selectPrimaryStrobe();

        }

      }

    }

  }

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
void updateDpioPioControlRegister(CDpio2DriverController* pController,
                                  UINT8 currentValue,
                                  UINT8 newValue)
{
  
  UINT8   changeBitMask;


  changeBitMask = currentValue ^ newValue;

  /* Only process this register setting if the new value is different
   * from the previous.
   */
  if ( changeBitMask != 0 ) {

    if ( (changeBitMask & DPIO_RES1_OE) != 0 ) {

      if ( (newValue & DPIO_RES1_OE) != 0 ) {

        pController->enableRes1AsOutput();

      } else {

        pController->disableRes1AsOutput();

      } 

    }


    if ( (changeBitMask & DPIO_RES1_REG) != 0 ) {

      if ( (newValue & DPIO_RES1_REG) != 0 ) {

        pController->setRes1OutputHigh();

      } else {

        pController->setRes1OutputLow();

      } 

    }


    if ( (changeBitMask & DPIO_RES2_OE) != 0 ) {

      if ( (newValue & DPIO_RES2_OE) != 0 ) {

        pController->enableRes2AsOutput();

      } else {

        pController->disableRes2AsOutput();

      } 

    }


    if ( (changeBitMask & DPIO_RES2_REG) != 0 ) {

      if ( (newValue & DPIO_RES2_REG) != 0 ) {

        pController->setRes2OutputHigh();

      } else {

        pController->setRes2OutputLow();

      } 

    }


    if ( (changeBitMask & DPIO_PIO1_OE) != 0 ) {

      if ( (newValue & DPIO_PIO1_OE) != 0 ) {

        pController->enablePio1AsOutput();

      } else {

        pController->disablePio1AsOutput();

      } 

    }


    if ( (changeBitMask & DPIO_PIO1_REG) != 0 ) {

      if ( (newValue & DPIO_PIO1_REG) != 0 ) {

        pController->setPio1OutputHigh();

      } else {

        pController->setPio1OutputLow();

      } 

    }


    if ( (changeBitMask & DPIO_PIO2_OE) != 0 ) {

      if ( (newValue & DPIO_PIO2_OE) != 0 ) {

        pController->enablePio2AsOutput();

      } else {

        pController->disablePio2AsOutput();

      } 

    }


    if ( (changeBitMask & DPIO_PIO2_REG) != 0 ) {

      if ( (newValue & DPIO_PIO2_REG) != 0 ) {

        pController->setPio2OutputHigh();

      } else {

        pController->setPio2OutputLow();

      } 

    }

  }

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
void updateDpioInterruptControlRegister(CDpio2DriverController* pController,
                                        UINT8 currentValue,
                                        UINT8 newValue)
{

  UINT8   changeBitMask;


  changeBitMask = currentValue ^ newValue;

  /* Only process this register setting if the new value is different
   * from the previous.
   */
  if ( changeBitMask != 0 ) {

    if ( (changeBitMask & INTEN) != 0 ) {

      if ( (newValue & INTEN) != 0 ) {

        pController->enableFrontEndInterrupts();

      } else {

        pController->disableFrontEndInterrupts();

      }

    }


    if ( (changeBitMask & ENFIFO) != 0 ) {

      DPIO2_ERROR_MSG(("The FIFO lock interrupt is not implemented on DPIO2\n"));

    }


    if ( (changeBitMask & ENPIO2) != 0 ) {

      if ( (newValue & ENPIO2) != 0 ) {

        pController->enableInterruptSource(DPIO2_PIO2_ASSERTED_INTERRUPT);

      } else {

        pController->disableInterruptSource(DPIO2_PIO2_ASSERTED_INTERRUPT);

      }

    }


    if ( (changeBitMask & ENPIO1) != 0 ) {

      if ( (newValue & ENPIO1) != 0 ) {

        pController->enableInterruptSource(DPIO2_PIO1_ASSERTED_INTERRUPT);

      } else {

        pController->disableInterruptSource(DPIO2_PIO1_ASSERTED_INTERRUPT);

      }

    }


    if ( (changeBitMask & ENNEF) != 0 ) {

      if ( (newValue & ENNEF) != 0 ) {

        pController->enableInterruptSource(DPIO2_FIFO_NOT_EMPTY_INTERRUPT);

      } else {

        pController->disableInterruptSource(DPIO2_FIFO_NOT_EMPTY_INTERRUPT);

      }

    }


    if ( (changeBitMask & ENEF) != 0 ) {

      if ( (newValue & ENEF) != 0 ) {

        pController->enableInterruptSource(DPIO2_FIFO_EMPTY_INTERRUPT);

      } else {

        pController->disableInterruptSource(DPIO2_FIFO_EMPTY_INTERRUPT);

      }

    }


    if ( (changeBitMask & ENHF) != 0 ) {

      if ( (newValue & ENHF) != 0 ) {

        pController->enableInterruptSource(DPIO2_FIFO_HALF_FULL_INTERRUPT);

      } else {

        pController->disableInterruptSource(DPIO2_FIFO_HALF_FULL_INTERRUPT);

      }

    }


    if ( (changeBitMask & ENFF) != 0 ) {

      if ( (newValue & ENFF) != 0 ) {

        pController->enableInterruptSource(DPIO2_FIFO_FULL_INTERRUPT);

      } else {

        pController->disableInterruptSource(DPIO2_FIFO_FULL_INTERRUPT);

      }

    }

  }

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
void updateDpioDmaControlRegister(CDpio2DriverController* pController,
                                  UINT8 currentValue,
                                  UINT8 newValue)
{

  UINT8   changeBitMask;


  /* Only process this register setting if the control bits in the new value is different
   * from the control bits in the previous.
   */
  changeBitMask = (currentValue ^ newValue) & (~IDIR);

  if ( changeBitMask != 0 ) {

    if ( (changeBitMask & DPIO_DMA_CONTROL_REGISTER_DEMAND_MODE_BITMASK) != 0 ) {

      DPIO2_WARNING_MSG(("Cannot select DMA Demand Mode on DPIO2 - only one available\n"), TRUE);

    }


    if ( pController->checkIsOutput() ) {

      /* Process the control bits that are only defined for DPIO output modules.
       */

      if ( (changeBitMask & DPIO_DMA_CONTROL_REGISTER_ENABLE_END_OF_FRAME_SYNC_BITMASK) != 0 ) {

        if ( (newValue & DPIO_DMA_CONTROL_REGISTER_ENABLE_END_OF_FRAME_SYNC_BITMASK) != 0 ) {

          pController->selectSyncGenerationMode(DPIO2_SYNC_ON_ODD_FRAME);

        } else {

          pController->selectSyncGenerationMode(DPIO2_NO_SYNC_GENERATION);

        }

      } 


      if ( (changeBitMask & DPIO_DMA_CONTROL_REGISTER_ENABLE_D0_AS_SYNC_BITMASK) != 0 ) {

        if ( (newValue & DPIO_DMA_CONTROL_REGISTER_ENABLE_D0_AS_SYNC_BITMASK) != 0 ) {

          pController->enableD0ToBeUsedForSync();

        } else {

          pController->disableD0ToBeUsedForSync();

        }

      } 

    } else {
    
      /* Process the control bits that are only defined for DPIO input modules.
       */
      if ( (changeBitMask & DPIO_DMA_CONTROL_REGISTER_SIGNAL_DEMAND_MODE_PAUSE_BITMASK) != 0 ) {

        DPIO2_WARNING_MSG(("SW cannot signal Demand Mode Pause on DPIO2\n"), TRUE);

      }


      if ( (changeBitMask & (SEL_EOT1 | SEL_EOT0)) != 0 ) {

        if ( (newValue & (SEL_EOT1 | SEL_EOT0)) == (SEL_EOT0) ) {

          pController->selectEndOfTransferMode(DPIO2_PIO2_MARKS_EOT); 

        } else if ( (newValue & (SEL_EOT1 | SEL_EOT0)) == (SEL_EOT1) ) {

          pController->selectEndOfTransferMode(DPIO2_SYNC_MARKS_EOT); 

        } else if ( (newValue & (SEL_EOT1 | SEL_EOT0)) == (SEL_EOT0 | SEL_EOT1) ) {

          pController->selectEndOfTransferMode(DPIO2_PIO1_MARKS_EOT); 

        } else {

          pController->selectEndOfTransferMode(DPIO2_NO_EOT); 

        }

      }

    }

  }

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
static UINT8 buildDpioInterruptControlRegister(CDpio2DriverController* pController)
{

  UINT8   interruptControlRegister;

  BOOL   isEnabled;


  interruptControlRegister = 0;


  if ( pController->checkAreFrontEndInterruptsEnabled() ) {

    interruptControlRegister |= INTEN;

  }

  
  if ( pController->checkIsInterruptSourceEnabled(DPIO2_PIO2_ASSERTED_INTERRUPT, isEnabled) == OK ) {

    if ( isEnabled ) {

      interruptControlRegister |= ENPIO2;

    }

  }
      

  if ( pController->checkIsInterruptSourceEnabled(DPIO2_PIO1_ASSERTED_INTERRUPT, isEnabled) == OK ) {

    if ( isEnabled ) {

      interruptControlRegister |= ENPIO1;

    }

  }
      

  if ( pController->checkIsInterruptSourceEnabled(DPIO2_FIFO_NOT_EMPTY_INTERRUPT, isEnabled) == OK ) {

    if ( isEnabled ) {

      interruptControlRegister |= ENNEF;

    }

  }
      

  if ( pController->checkIsInterruptSourceEnabled(DPIO2_FIFO_EMPTY_INTERRUPT, isEnabled) == OK ) {

    if ( isEnabled ) {

      interruptControlRegister |= ENEF;

    }

  }
      

  if ( pController->checkIsInterruptSourceEnabled(DPIO2_FIFO_HALF_FULL_INTERRUPT, isEnabled) == OK ) {

    if ( isEnabled ) {

      interruptControlRegister |= ENHF;

    }

  }
      

  if ( pController->checkIsInterruptSourceEnabled(DPIO2_FIFO_FULL_INTERRUPT, isEnabled) == OK ) {

    if ( isEnabled ) {

      interruptControlRegister |= ENFF;

    }

  }


  return (interruptControlRegister);

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
static UINT8 buildDpioFifoStatusRegister(CDpio2DriverController* pController)
{

  UINT8   fifoStatusValue;

  BOOL   notEmptyFlag;


  fifoStatusValue = pController->getFifoState();


  if ( !pController->checkIsOutput() ) {

    pController->checkIsPipelineNotEmpty(notEmptyFlag);

    if ( notEmptyFlag ) {

         fifoStatusValue |= PLNEMPTY;

    }

  }


  if ( pController->checkHasFifoBeenFull() ) {

    fifoStatusValue |= AEFOCC;

  }

  if ( pController->checkHasFifoBeenMoreThanHalfFull() ) {

    fifoStatusValue |= HFOCC;

  }

  if ( pController->checkHasFifoBeenEmpty() ) {

    fifoStatusValue |= EFOCC;

  }

  
  if ( pController->checkIsRes3InputHigh() ) {

    fifoStatusValue |= RES3_CUR;

  }


  return (fifoStatusValue);

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
static UINT8 buildDpioPioStatusRegister(CDpio2DriverController* pController)
{

  UINT8   pioStatusValue;


  pioStatusValue = 0;


  if ( pController->checkIsRes1InputHigh() ) {

    pioStatusValue |= DPIO_RES1_CUR;

  }

  if ( pController->checkIsRes2InputHigh() ) {

    pioStatusValue |= DPIO_RES2_CUR;

  }

  if ( pController->checkIsPio1InputHigh() ) {

    pioStatusValue |= DPIO_PIO1_CUR;

  }

  if ( pController->checkIsPio2InputHigh() ) {

    pioStatusValue |= DPIO_PIO2_CUR;

  }


  return (pioStatusValue);

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
static UINT8 buildDpioInterruptStatusRegister(CDpio2DriverController* pController)
{

  UINT8   interruptStatusValue;

  BOOL   isActive;


  interruptStatusValue = 0;


  if ( pController->checkIsInterruptSourceActive(DPIO2_FIFO_FULL_INTERRUPT, isActive) == OK ) {

    if ( isActive ) {

      interruptStatusValue |= INTFF;

    }

  }


  if ( pController->checkIsInterruptSourceActive(DPIO2_FIFO_HALF_FULL_INTERRUPT, isActive) == OK ) {

    if ( isActive ) {

      interruptStatusValue |= INTHF;

    }

  }


  if ( pController->checkIsInterruptSourceActive(DPIO2_FIFO_EMPTY_INTERRUPT, isActive) == OK ) {

    if ( isActive ) {

      interruptStatusValue |= INTEF;

    }

  }


  if ( pController->checkIsInterruptSourceActive(DPIO2_FIFO_NOT_EMPTY_INTERRUPT, isActive) == OK ) {

    if ( isActive ) {

      interruptStatusValue |= INTNEF;

    }

  }


  if ( pController->checkIsInterruptSourceActive(DPIO2_PIO1_ASSERTED_INTERRUPT, isActive) == OK ) {

    if ( isActive ) {

      interruptStatusValue |= INTPIO1;

    }

  }


  if ( pController->checkIsInterruptSourceActive(DPIO2_PIO2_ASSERTED_INTERRUPT, isActive) == OK ) {

    if ( isActive ) {

      interruptStatusValue |= INTPIO2;

    }

  }


  return (interruptStatusValue);

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
static UINT8 buildDpioModuleIdRegister(CDpio2DriverController* pController)
{

  UINT8   moduleIdValue;

 
  moduleIdValue = pController->getPersonalityId();
  moduleIdValue |= (pController->getMainBoardId() << 4);

  return (moduleIdValue);

}
#endif /* VXWORKS || WIN32 */


#if defined(VXWORKS) || defined(WIN32)
void dpio2SetHaltFunction(FUNCPTR pFunc)
{

  pHaltFunction = pFunc;

}
#endif /* VXWORKS || WIN32 */


STATUS dpio2InitInterruptActions(DPIO2_DEVICE* pDevice)
{

  int   i;


  for (i = 0; i < DPIO2_INT_CONDITIONS; i++) {

    pDevice->dpio2DisableAction[i].setDriverController(pDevice->pController);
    pDevice->dpio2DisableAction[i].setInterruptSourceId(mapDpio2ConditionToSource(i));

    pDevice->pDpio2CurrentDmaAction[i] = NULL;

    pDevice->dpio2StartDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpio2StartDmaAction[i].setCondition(dpio2ApiActionInfo[i].pConditionMethod, 
                                                 dpio2ApiActionInfo[i].condition);

    pDevice->dpio2StopDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpio2StopDmaAction[i].setCondition(dpio2ApiActionInfo[i].pConditionMethod, 
                                                dpio2ApiActionInfo[i].condition);

    pDevice->dpio2SuspendDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpio2SuspendDmaAction[i].setCondition(dpio2ApiActionInfo[i].pConditionMethod, 
                                                   dpio2ApiActionInfo[i].condition);

    pDevice->dpio2ResumeDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpio2ResumeDmaAction[i].setCondition(dpio2ApiActionInfo[i].pConditionMethod, 
                                                  dpio2ApiActionInfo[i].condition);

    pDevice->dpio2CallbackAction[i].setDriverController(pDevice->pController);
    pDevice->dpio2CallbackAction[i].setCondition(dpio2ApiActionInfo[i].pConditionMethod, 
                                                 dpio2ApiActionInfo[i].condition);

  }


  return (OK);

}


#if defined(VXWORKS) || defined(WIN32)
STATUS dpio2DpioApiInitInterruptActions(DPIO2_DPIO_API_DEVICE* pDevice)
{

  int   i;


  for (i = 0; i < DPIO_INT_CONDITIONS; i++) {

    pDevice->dpioDisableAction[i].setDriverController(pDevice->pController);
    pDevice->dpioDisableAction[i].setInterruptSourceId(mapDpioConditionToSource(i));

    pDevice->pDpioCurrentDmaAction[i] = NULL;

    pDevice->dpioStartDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpioStartDmaAction[i].setCondition(dpioApiActionInfo[i].pConditionMethod, 
                                                dpioApiActionInfo[i].condition);

    pDevice->dpioStopDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpioStopDmaAction[i].setCondition(dpioApiActionInfo[i].pConditionMethod, 
                                               dpioApiActionInfo[i].condition);

    pDevice->dpioSuspendDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpioSuspendDmaAction[i].setCondition(dpioApiActionInfo[i].pConditionMethod, 
                                                  dpioApiActionInfo[i].condition);

    pDevice->dpioResumeDmaAction[i].setDriverController(pDevice->pController);
    pDevice->dpioResumeDmaAction[i].setCondition(dpioApiActionInfo[i].pConditionMethod, 
                                                 dpioApiActionInfo[i].condition);

    pDevice->dpioCallbackAction[i].setDriverController(pDevice->pController);
    pDevice->dpioCallbackAction[i].setCondition(dpioApiActionInfo[i].pConditionMethod, 
                                                dpioApiActionInfo[i].condition);

  }


  return (OK);

}
#endif /* VXWORKS || WIN32 */
