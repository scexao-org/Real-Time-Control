/*
 * DPIO2 RH-Linux Command Line Interface (RHCLI)
 *
 * Author: Michael A. DeLuca
 * MDL Enterprises
 * m.deluca@comcast.net
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "dpio2funcs.h"
#include "timespecutil.h"
#include "shared.h"


extern int clock_nanosleep (clockid_t __clock_id, int __flags,
                            __const struct timespec *__req,
                            struct timespec *__rem);



#define USE_CLOCK CLOCK_REALTIME
//#define USE_CLOCK CLOCK_PROCESS_CPUTIME_ID
//#define USE_CLOCK CLOCK_THREAD_CPUTIME_ID

#define DBL_BUF
#define USE_SEMA

#define NSTICK  1000000000

#define USER_BUFFER_ELEMENTS 4096
/*
Don't change this define. The user buffer and DMA
buffer sizes must be consistent!
*/
#define BYTES_IN_DMA_BUFFER 2 * USER_BUFFER_ELEMENTS

FILE*                   fpout           = NULL;
static long             Rate            = 1200;
static long             iTick           = NSTICK / 1200;
static int		fd1		= -1;
static unsigned long    FrameNum        = 0;
static int              Run             = 1;
static int              Step            = 0;
static int              Term            = 0;
static short            Strobe          = 10;
static short            Fixed           = 0;
static unsigned int     dwStrobe        = 10*1000000;
static int              iDataMode       = 0;
static short            PokeIndex       = 0;
static unsigned short   PokeValue       = 0;
static unsigned int     dwResultStrobe  = 0;
static unsigned int*    pdwResultStrobe = &dwResultStrobe;
static unsigned int     dwDmaSkipCount  = 0;
static int              iErrorMode      = 0;

static STATUS       status;
static int          devno_out = 1;

#ifdef DBL_BUF
static int             idx              = 0;             // double buffer index
static DPIO2_DMA       dmaInfo_out[2];
static DPIO2_DMA*      pPageInfo_out[2] = {&dmaInfo_out[0], &dmaInfo_out[1]};
static void*           pDmaBuffer[2]    = {NULL, NULL};  // these are the kernel space DMA buffer pointers
static unsigned short* pUserBuffer[2]   = {NULL, NULL};  // these are the user space buffer pointers passed to the driver
#else
static DPIO2_DMA       dmaInfo_out;
static DPIO2_DMA*      pPageInfo_out    = &dmaInfo_out;
static void*           pDmaBuffer       = NULL;          // this is the kernel space DMA buffer pointer
static unsigned short* pUserBuffer      = NULL;          // this is the user space buffer pointer passed to the driver
#endif

#ifdef USE_SEMA
static sem_t        dmaDoneSemaphore_out;
#endif

STATUS AllocDmaBuffers()
{
    const size_t dma_buffer_size = BYTES_IN_DMA_BUFFER;

#ifdef DBL_BUF

	printf("Double buffering is on\n");

        // Free buffers
        if (pDmaBuffer[0])
        {
                unlockAndFreeDmaBuffer (pPageInfo_out[0]);
                pDmaBuffer[0] = NULL;
        }
        if (pDmaBuffer[1])
        {
                unlockAndFreeDmaBuffer (pPageInfo_out[1]);
                pDmaBuffer[1]= NULL;
        }


        // Allocate buffers
        status  = allocateAndLockDmaBuffer (dma_buffer_size, &pDmaBuffer[0], &pPageInfo_out[0]);
        status |= allocateAndLockDmaBuffer (dma_buffer_size, &pDmaBuffer[1], &pPageInfo_out[1]);
        if (status != OK)
        {
                printf("Failed to allocate DMA buffers\n");
                return 1;
        }

	printf("Finished allocating buffers; pDmaBuffer[0] = %p, pDmaBuffer[1] =%p\n\n", pDmaBuffer[0], pDmaBuffer[1]);

        // Configure DMA chain
#ifdef RSR_DEBUG
        printf("In AllocBuffers; calling configureDmaChain.\n");
#endif

        status = configureDmaChain (devno_out, pPageInfo_out[0], 0);

#ifdef RSR_DEBUG
	printf("Finished first configureDmaChain\n");
#endif

        if (status != OK)
        {
                printf ("Failed to configure DMA chain\n");
                unlockAndFreeDmaBuffer (pPageInfo_out[0]);
                return  1;
        }
        status = configureDmaChain (devno_out, pPageInfo_out[1], 8);

#ifdef RSR_DEBUG
	printf("Finished second configureDmaChain\n");
#endif

        if (status != OK)
        {
                printf ("Failed to configure DMA chain\n");
                unlockAndFreeDmaBuffer (pPageInfo_out[1]);
                return  1;
        }

#ifdef RSR_DEBUG
	printf("Finished allocating DMA buffers.\n");
#endif

#else

	printf("Single buffering is on\n");        

        // Free buffers
        if (pDmaBuffer)
        {
                unlockAndFreeDmaBuffer (pPageInfo_out);
                pDmaBuffer = NULL;
        }

        // Allocate buffer
        status  = allocateAndLockDmaBuffer (dma_buffer_size, &pDmaBuffer, &pPageInfo_out);
        if (status != OK)
        {
                printf("Failed to allocate buffer\n");
                return 1;
        }

	printf("Finished allocating buffers; pDmaBuffer = %p\n", pDmaBuffer);

        // Configure DMA chain
        status = configureDmaChain (devno_out, pPageInfo_out, 0);
        if (status != OK)
        {
                printf ("Failed to configure DMA chain\n");
                unlockAndFreeDmaBuffer (pPageInfo_out);
                return  1;
        }

#endif

        return OK;
}


/*
    The following code allocates the user buffers and locks them
	into memory so that they are always available to be copied to
	the kernel.
*/
STATUS AllocateUserBuffers()
{
	STATUS status;
	void* address           = NULL;
	int memalign_return     = 0;
    size_t user_buffer_size = 2 * USER_BUFFER_ELEMENTS;

#ifdef DBL_BUF
	memalign_return  = posix_memalign(&address, PAGE_SIZE, user_buffer_size);
	pUserBuffer[0]   = (unsigned short*) address;
	memalign_return |= posix_memalign(&address, PAGE_SIZE, user_buffer_size);
	pUserBuffer[1]   = (unsigned short*) address;

	if (memalign_return == OK)
	{
		printf("pUserBuffer[0] = %p, pUserBuffer[1] = %p\n", pUserBuffer[0], pUserBuffer[1]);
	}
	else
    {
        printf ("Failed to allocate user buffers!\n");
        return  1;
    }
#else
	memalign_return  = posix_memalign(&address, PAGE_SIZE, user_buffer_size);
    pUserBuffer      = (unsigned short*) address;

	if (memalign_return == OK)
	{
		printf("pUserBuffer = %p\n", pUserBuffer);
	}
	else
    {
        printf ("Failed to allocate user buffer!\n");
        return  1;
    }
#endif

#ifdef DBL_BUF
	status  = mlock((void*) pUserBuffer[0], user_buffer_size);
    status |= mlock((void*) pUserBuffer[1], user_buffer_size);

	if (status != OK)
    {
        printf ("Failed to lock user buffers!\n");
        return  1;
    }
#else
	status = mlock((void*) pUserBuffer, user_buffer_size);

	if (status != OK)
    {
        printf ("Failed to lock user buffer!\n");
        return  1;
    }
#endif
	return OK;
}


/*
    The following code unlocks and frees the user buffers.
*/
STATUS UnlockUserBuffers()
{
	STATUS status;
    size_t user_buffer_size = 2 * USER_BUFFER_ELEMENTS;

#ifdef DBL_BUF
	status  = munlock((void*) pUserBuffer[0], user_buffer_size);
    status |= munlock((void*) pUserBuffer[1], user_buffer_size);

	if (status != OK)
    {
		printf ("Failed to unlock user buffers!\n");
		return  1;
    }
#else
	status = munlock((void*) pUserBuffer, user_buffer_size);

	if (status != OK)
	{
		printf ("Failed to unlock user buffer!\n");
		return  1;
	}
#endif

#ifdef DBL_BUF
	free((void*) pUserBuffer[0]);
	free((void*) pUserBuffer[1]);
#else
	free((void*) pUserBuffer);
#endif
	
	return OK;
}


// The following is called from the real time thread loop.
// 1) Control Strobes
// 2) Receive commands from RHCLI via FIFO
// 3) Generate Data
// 4) Send Data
// 5) Toggle LPT strobes for diagnostics
//
static void rh_timer_callback(void)
{
	int i =0, j = 0;
	const int user_buffer_elements = USER_BUFFER_ELEMENTS;
/*
	bytes_to_copy is set to the DMA buffer size, since writing past
	the end of the DMA buffer could lead to a system crash!
*/
	const int bytes_to_copy = BYTES_IN_DMA_BUFFER;

	unsigned char CmdByte;
	STATUS status = OK;

	// raise PIO strobe(s)
	if (dpio2Ioctl (devno_out, DPIO2_CMD_PIO2_OUTPUT_VALUE_SET, TRUE) != OK)
        {
        	Run = 0;
        }
        // get the command code
        if (read(fd1, &CmdByte, sizeof(CmdByte)) == sizeof(CmdByte))
        {
                switch (CmdByte)
                {
                // stop main loop
                case STOP_CMD:
                        #ifdef MDL_DEBUG
                        printf("STOP\n");
                        #endif
                        Run = 0;
                        if (dwDmaSkipCount)
                           printf("DMA overrun: %d frames\n", dwDmaSkipCount);
                        dwDmaSkipCount = 0;
                        break;

                // start main loop
                case RUN_CMD:
                        #ifdef MDL_DEBUG
                        printf("RUN\n");
                        #endif
                        Run = 1;
                        dwDmaSkipCount = 0;
                        break;

                // step main loop
                case STEP_CMD:
                        #ifdef MDL_DEBUG
                        printf("STEP\n");
                        #endif
                        Step = 1;
                        break;

                // clear all actuator values to 0x0000
                case CLEAR_CMD:
                        #ifdef MDL_DEBUG
                        printf("CLEAR\n");
                        #endif
                        iDataMode = 0;
                        for( i=0; i<user_buffer_elements; i++ )
                        {
#ifdef DBL_BUF
                                pUserBuffer[0][i] = pUserBuffer[1][i] = (unsigned short int) 0x0000;
#else
                                pUserBuffer[i] = (unsigned short int) 0x0000;
#endif
                        }
                        //Copy user buffer to DMA buffer
#ifdef DBL_BUF
                        status  = copyToKernel (pDmaBuffer[0], (void*) pUserBuffer[0], bytes_to_copy);
                        status |= copyToKernel (pDmaBuffer[1], (void*) pUserBuffer[1], bytes_to_copy);
#else
                        status  = copyToKernel (pDmaBuffer, (void*) pUserBuffer, bytes_to_copy);
#endif


                        break;

                // load all actuators from a file


                case LOAD_CMD:
                        #ifdef MDL_DEBUG
                        printf("LOAD\n");
                        #endif
                        if (iDataMode != 1)
                        {
                                for( i=0; i<user_buffer_elements; i++ )
#ifdef DBL_BUF
                                        pUserBuffer[0][i] = pUserBuffer[1][i] = (unsigned short int) 0x0000;
#else
                                        pUserBuffer[i] = (unsigned short int) 0x0000;
#endif
                        }
                        iDataMode = 1;
                        // PokeIndex used to get count of words to come
                        // fpout = fopen("userbuffer.txt", "w");
                        while (read(fd1, &PokeIndex, sizeof(PokeIndex)) != sizeof(PokeIndex));
                        for (i=0; i<PokeIndex; i++)
                        {
                                while (read(fd1, &PokeValue, sizeof(PokeValue)) != sizeof(PokeValue));
#ifdef DBL_BUF
                                pUserBuffer[0][i] = pUserBuffer[1][i] = PokeValue;
#else
                                pUserBuffer[i] = PokeValue;
#endif
                                // fprintf(fpout, "i = %d, PokeValue = %04X\n", i, PokeValue);
                        }
                        //fclose(fpout);
                        // Copy user buffer to DMA buffer
#ifdef DBL_BUF
                        status  = copyToKernel (pDmaBuffer[0], (void*) pUserBuffer[0], bytes_to_copy);
                        status |= copyToKernel (pDmaBuffer[1], (void*) pUserBuffer[1], bytes_to_copy);
#else
                        status  = copyToKernel (pDmaBuffer, (void*) pUserBuffer, bytes_to_copy);
#endif
                        break;

                // setup framing test pattern
                case TEST_CMD:
						iDataMode = 3;
                        #ifdef MDL_DEBUG
                        printf("TEST: iDataMode = %d\n", iDataMode);
                        #endif
                        break;

               // specify halt on error
                case HALTERR_CMD:
                        #ifdef MDL_DEBUG
                        printf("HALTERR ");
                        #endif
                        while (read(fd1, &CmdByte, sizeof(CmdByte)) != sizeof(CmdByte));
                        iErrorMode = CmdByte ? 1 : 0;
                        #ifdef MDL_DEBUG
                        printf("%d\n", iErrorMode);
                        #endif
                        break;

                // poke a single actuator
                case POKE_CMD:
                        #ifdef MDL_DEBUG
                        printf("POKE\n");
                        #endif
                        if (iDataMode != 2)
                        {
                                for( i=0; i<user_buffer_elements; i++ )
#ifdef DBL_BUF
                                        pUserBuffer[0][i] = pUserBuffer[1][i] = (unsigned short int) 0x0000;
#else
                                        pUserBuffer[i] = (unsigned short int) 0x0000;
#endif
                        }
                        iDataMode = 2;
                        while (read(fd1, &PokeIndex, sizeof(PokeIndex)) != sizeof(PokeIndex));
                        while (read(fd1, &PokeValue, sizeof(PokeValue)) != sizeof(PokeValue));
                        #ifdef MDL_DEBUG
                        printf("POKE act %d with %04x\n", PokeIndex, PokeValue);
                        #endif
                        if (PokeIndex >=0 && PokeIndex < user_buffer_elements)
                        {
#ifdef DBL_BUF
                                pUserBuffer[0][PokeIndex] = pUserBuffer[1][PokeIndex] = PokeValue;
#else
                                pUserBuffer[PokeIndex] = PokeValue;
#endif
                        }
                        //Copy user buffer to DMA buffer
#ifdef DBL_BUF
                        status  = copyToKernel (pDmaBuffer[0], (void*) pUserBuffer[0], bytes_to_copy);
                        status |= copyToKernel (pDmaBuffer[1], (void*) pUserBuffer[1], bytes_to_copy);
#else
                        status  = copyToKernel (pDmaBuffer, (void*) pUserBuffer, bytes_to_copy);
#endif
                        break;

                // set frame rate via reprogramming the timer
                case FRAMERATE_CMD:
                        while (read(fd1, &Rate, sizeof(Rate)) != sizeof(Rate));
                        iTick = NSTICK / Rate;
                        #ifdef MDL_DEBUG
                        printf("FRAMERATE, iTick = %d\n", iTick);
                        #endif
                        break;

                // set strobe
                case STROBE_CMD:
                        #ifdef MDL_DEBUG
                        printf("STROBE\n");
                        #endif
                        while (read(fd1, &Strobe, sizeof(Strobe)) != sizeof(Strobe));
                        while (read(fd1, &Fixed,  sizeof(Fixed))  != sizeof(Fixed));
                        dwStrobe = (unsigned int) (Strobe*1000000);
                        dwResultStrobe = dwStrobe;
                        printf("The address of dwResultStrobe is %p\n", pdwResultStrobe);
                        {
                        // use DPIO2_FIXED_OSCILLATOR or DPIO2_PROGRAMMABLE_OSCILLATOR
                        status = enableStrobeGeneration(devno_out, pdwResultStrobe,
                                 Fixed ? DPIO2_FIXED_OSCILLATOR : DPIO2_PROGRAMMABLE_OSCILLATOR);

                        if (status != OK)
                                printf("dpio2_EnableStrobeGeneration failed\n");
                        #ifdef MDL_DEBUG
                        else
                                printf("%s dpio2 strobe set to %d\n", Fixed ? "FIXED" : "PROG", dwResultStrobe);
                        #endif
                        }
                        break;

                // rtcli termination, just stop framing
                case TERM_CMD:
                        #ifdef MDL_DEBUG
                        printf("TERM\n");
                        #endif
                        Term = 1;
                        break;
                }
        }

        // if we are framing
        if ((Run||Step) && Term==0)

        {
                // the only dynamic pattern is the test pattern (iDataMode = 3)
                if (iDataMode==3)
                { 
                     unsigned short int iTopNibble;
                     unsigned short int iShiftPat = 1;
                     FrameNum++;
                     iTopNibble = (FrameNum%16)<<12;
                     for( j=0; j<user_buffer_elements; j++ )
                     {
#ifdef DBL_BUF
                          pUserBuffer[0][j] = pUserBuffer[1][j] = (unsigned short int) (iTopNibble | iShiftPat);
#else
                          pUserBuffer[j] = (unsigned short int) (iTopNibble | iShiftPat);
#endif
                          iShiftPat = iShiftPat<<1;
                          if (iShiftPat & 0x1000)
                                  iShiftPat = 1;
                     }
                }

        	// lower PIO2 strobe
        	if (dpio2Ioctl (devno_out, DPIO2_CMD_PIO2_OUTPUT_VALUE_SET, FALSE) != OK)
                	Run = 0;

  		// Start DMA transfer. 
#ifdef DBL_BUF
  		status = startDmaTransfer (devno_out, idx?8:0);
#else
                status = startDmaTransfer (devno_out, 0);
#endif
        	if (status != OK)
        	{
            	    if (iErrorMode)
            	    {
               		printf("DMA overrun: loop halted.\n");
               		printf("Lower the loop frequency or use \"halterr\" command to override.\n");
               		Run = 0;

            	    }
            	    else
               		// just count skipped frames id iErrorMode=0
               		dwDmaSkipCount++;
        	}
        	else
		{
            	   // toggle the ping/pong buffer if the DMA was succesful
#ifdef DBL_BUF
            	   idx = idx?0:1;
#endif

#ifdef USE_SEMA
  		   // Wait for DMA transfer to complete 
  		   sem_wait (&dmaDoneSemaphore_out);
#endif
		}

                Step = 0;
        }
}

// RH-Linux Thread
// Execute worker function at regular intervals (set by iTick through "freq" command)
// Protect against loop hang by checking current now+5us time and avoid calling clock_nanosleep()
// if we are past the time slot; run while Term!=0
void *thread_code(void* t)
{
        struct timespec next;
        //struct timespec now;
        #ifdef MDL_DEBUG
                printf("RHL Thread start\n");
        #endif

    	// Initialize device for output
    	if (openDpio2ForOutput(devno_out) != OK)
	{
      		printf("Failed to initialize device number %d for output\n", devno_out);
      		return NULL;
    	}
        else
        {
                printf("\nDevice number %d has been opened for output\n", devno_out);
        }

#ifdef USE_SEMA
  	// Init semaphore
  	sem_init (&dmaDoneSemaphore_out, 0, 0);

  	// Configure interrupts
  	status  = configureDmaDoneInterrupt(devno_out,
                                     (FUNCPTR) dmaDoneNotifier,
                                     &dmaDoneSemaphore_out);

        printf("\nSemaphore is on.\n\n");
#else
        printf("\nSemaphore is off.\n\n");
#endif

  	// Flush FIFO 
  	status = dpio2Ioctl (devno_out, DPIO2_CMD_FIFO_FLUSH, 0 );

#if 0
  	// Disable flow control (NRDY and SUSPEND).
  	dpio2Ioctl ( devno_out, DPIO2_CMD_NRDY_FLOW_CONTROL_SELECT, FALSE);
  	dpio2Ioctl ( devno_out, DPIO2_CMD_SUSPEND_FLOW_CONTROL_SELECT, FALSE);
  	// Configure the DPIO2 to clock in data on single edge of the strobe.
  	status |= dpio2Ioctl (devno_out, DPIO2_CMD_CLOCKING_ON_BOTH_STROBE_EDGES_SELECT, FALSE);
#endif

  	// set PIO1 & PIO2 as outputs
  	status |= dpio2Ioctl (devno_out, DPIO2_CMD_PIO1_DIRECTION_SELECT, TRUE);
  	status |= dpio2Ioctl (devno_out, DPIO2_CMD_PIO2_DIRECTION_SELECT, TRUE);

  	// raise PIO strobe(s)
  	status |= dpio2Ioctl (devno_out, DPIO2_CMD_PIO1_OUTPUT_VALUE_SET, TRUE);
  	status |= dpio2Ioctl (devno_out, DPIO2_CMD_PIO2_OUTPUT_VALUE_SET, TRUE);

  	// Configure FPDP interface
  	status |= activateFpdpInterface (devno_out);
  	if (status != OK)
  	{
    		printf("Failed to configure DPIO2 device\n");
    		return NULL;
  	}

#ifdef RSR_DEBUG
	printf("DPIO2 device configured in thread_code\n");
#endif

	status = AllocDmaBuffers();
	if (status != OK)
	{
		printf("Failed to allocate DMA buffers!\n");
		return NULL;
	}

	status = AllocateUserBuffers();
	if (status != OK)
	{
		printf("Failed to allocate user buffers!\n");
		return NULL;
	}

        #ifdef MDL_DEBUG
                printf("Opening FIFO for reading...\n");
        #endif
        // open the FIFOs
        fd1 = open( "/tmp/mdlfifo", O_RDONLY | O_NONBLOCK );
        if (fd1 == -1)
	{
                printf("open failed %d\n", errno);
                return NULL;
	}

#ifdef MDL_DEBUG
	clock_getres(USE_CLOCK, &next);
	printf("Clock resolution %d %d\n", next.tv_sec, next.tv_nsec);
	//{
        //clockid_t clkid = 0;
        //int iret = clock_getcpuclockid(0, &clkid);
        //printf("%d %d\n", iret, clkid);
	//}
#endif

        // get the current time and setup for the first tick
	clock_gettime( USE_CLOCK, &next);

        while (Term == 0)
        {
                // set the period for the loop 
                timespec_add_ns( &next, iTick);

#if 0
                // sleep 
                clock_gettime( USE_CLOCK, &now);

                timespec_add_ns( &now, 5000);

                if (timespec_gt(&next, &now))
#endif
           	   clock_nanosleep( USE_CLOCK, TIMER_ABSTIME, &next, NULL);

                rh_timer_callback();
        }

  	// Free DMA buffer
#ifdef DBL_BUF
  	unlockAndFreeDmaBuffer (pPageInfo_out[0]);
	unlockAndFreeDmaBuffer (pPageInfo_out[1]);
#else
	unlockAndFreeDmaBuffer (pPageInfo_out);
#endif
	//Unlock user buffer
	status = UnlockUserBuffers();

        // raise PIO strobe(s)
        dpio2Ioctl (devno_out, DPIO2_CMD_PIO1_OUTPUT_VALUE_SET, FALSE);
        dpio2Ioctl (devno_out, DPIO2_CMD_PIO2_OUTPUT_VALUE_SET, FALSE);
        // set strobes low, turn off
        dpio2Ioctl (devno_out, DPIO2_CMD_PIO1_DIRECTION_SELECT, FALSE);
        dpio2Ioctl (devno_out, DPIO2_CMD_PIO2_DIRECTION_SELECT, FALSE);

  	// Close device 
	closeDevice (devno_out);

        #ifdef MDL_DEBUG
                printf("RHL Thread exit\n");
        #endif
        return NULL;
}
