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
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dpio2funcs.h"
#include "shared.h"
#include "mapdata.h"

#define	STATE_RUN	0
#define STATE_STOP	1
#define MAX_COMMAND_ARGS 32             // command line processing
#define COMMAND_LENGTH 128
#define iMode 4096                      // mode: efeects what translation table is used

static int   		end     = 0;		           // termination flag	
static int   		iLL     = 0;		           // Low Latency mode for 1K off
static int   		iState  = STATE_STOP;	       // default value for start/stop is stopped
static int          Mode    = iMode;
static long  		iRate   = 1000;		           // default frame rate frequency is 1000 Hz
static short 		iStrobe = 50;		           // default strobe rate frequency is 50 MHz
static short 		iFixed  = 0;		           // default strobe is programmable
static pthread_t    thread  = (pthread_t) NULL;
short  sActValue[iMode];

// termination function for SIGINT callback
static void endme(int dummy) { end = 1; }
int FindActuatorIndex(int *Map, int iCount, int iActuator);
void *thread_code(void *t);
void PrintMenu(int Mode, int iState, int iStrobe, int iRate, int iFixed, int iLL);

// main entry (no args used in command line)
int main(int argc,char *argv[])
{
    int iRet = 0;
    char SendByte;
    char Commandline[COMMAND_LENGTH], LastCommand[COMMAND_LENGTH];
    int cmd = -1, i = 0, /*j = 0, */ArgCount = 0, Length = 0;
    char* Arg[MAX_COMMAND_ARGS];
    int numberOfDpio2Modules = 0;
    int* pnumberOfDpio2Modules = &numberOfDpio2Modules;


    if (scanForDevices (pnumberOfDpio2Modules) != OK)
    	return -1;
    #ifdef MDL_DEBUG
        printf("Found %d DPIO2 modules in system\n", numberOfDpio2Modules);
    #endif
   
    // clear out command lines
    memset(Commandline, ' ', COMMAND_LENGTH);
    memset(LastCommand, ' ', COMMAND_LENGTH);

    // initialize last command to default command
    strcpy(LastCommand, "stop");

    // register interrupt signal to terminate the main loop
    signal(SIGINT, endme);

    //#ifdef MDL_DEBUG
    //        printf("RHL Main start\n");
    //#endif

    // remove the FIFO (in case previous program crash left it)
    unlink( "/tmp/mdlfifo" );

    // make a the FIFO that is available to Linux applications
    iRet = mkfifo( "/tmp/mdlfifo", 0777);
    if (iRet) {
            printf("mkfifo failed %d\n", errno);
            exit (1);
    }
    // create the framing thread
#if 0 // exp 1
    pthread_attr_t Attr;
    pthread_attr_init (&Attr);
    //pthread_attr_setschedpolicy(&Attr, SCHED_FIFO);
    pthread_attr_setscope(&Attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setinheritsched(&Attr, PTHREAD_EXPLICIT_SCHED);
#endif

#if 0
    pthread_attr_t tattr;
    int ret;
    int newprio = -20;
    struct sched_param param;

    /* initialized with default attributes */
    ret = pthread_attr_init (&tattr);
    ret = pthread_attr_setschedpolicy(&tattr, SCHED_FIFO);

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);
    printf ("P = %d\n", param.sched_priority );

    /* set the priority; others are unchanged */
    param.sched_priority = newprio;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    /* with new priority specified */
    pthread_create( &thread, &tattr, thread_code, (void *)0 );
#endif

    pthread_create( &thread, NULL, thread_code, (void *)0 );

#ifdef RSR_DEBUG
    printf("Line 124 in rhcli.c; pthread_create has started.\n");
#endif

    // open FIFO to communicate to the real-time task
    if ((cmd = open("/tmp/mdlfifo", O_WRONLY)) < 0)
    {
        printf("Error opening /tmp/mdlfifo\n");       
        exit(1);
    }

#ifdef RSR_DEBUG
    printf("Line 134 in rhcli.c; the FIFO file has opened.\n");
#endif

    // send stop
    iState = STATE_STOP;
    SendByte = STOP_CMD;
    write(cmd, &SendByte, 1);

    // send error mode = 1
    SendByte = HALTERR_CMD;
    write(cmd, &SendByte, 1);
    SendByte = 1;
    write(cmd, &SendByte, 1);

    // send frequency = 1000 Hz
    iRate = (long) 1000;
    SendByte = FRAMERATE_CMD;
    write(cmd, &SendByte, 1);
    write(cmd, &iRate, sizeof(long));

    // send strobe = 50MHz
    iStrobe = (short) 50;
    SendByte = STROBE_CMD;
    write(cmd, &SendByte, 1);
    write(cmd, &iStrobe, sizeof(short));
    iFixed = 0; // programmable
    write(cmd, &iFixed, sizeof(short));

    // wait for the above commands to execute
    sleep(1);

    // print the help menu on startup
    PrintMenu(Mode, iState, iStrobe, iRate, iFixed, iLL);

    // start main loop
    while(!end)
    {
	// prompt
	printf("\r>>>");

        fflush(stdout);

	// get the user command
	fgets(Commandline, COMMAND_LENGTH-4, stdin);
	Length = strlen(Commandline);

	// convert command line to lower case
	for (i=0; i<Length; i++)
	{
		Commandline[i] = tolower(Commandline[i]);
		if (Commandline[i] == '\n')
			Commandline[i] = '\0';
	}

	// get length of the command line
	Length = strlen(Commandline);

	if (Length == 0)
	{
		// empty command <CR>: just repeat the last command
		strcpy(Commandline, LastCommand);
		printf(">>>%s\n\r", Commandline);
	}
	else
		// save the new command
		strcpy(LastCommand, Commandline);

	#ifdef MDL_DEBUG_CMD
		for (i=0; i<strlen(Commandline); i++)
		{
			printf("Commandline[%d] = %c (%d)\n\r", i, Commandline[i], Commandline[i]);
			fflush(stdout);
		}
		printf("Commandline = %s\n\r", Commandline);fflush(stdout);
	#endif

	// get the args
	ArgCount = 0;
	Arg[ArgCount] = strtok( Commandline, " " );
    	while( Arg[ArgCount++] != NULL )
    	{
		if (ArgCount > MAX_COMMAND_ARGS)
			break;

		Arg[ArgCount] = strtok( NULL, " " );
    	}

	ArgCount--;
	Length = strlen(Arg[0]);

	#ifdef MDL_DEBUG_CMD
		printf("Arg Count =  %d\n\r", ArgCount);
		printf("Arg0 Length =  %d\n\r", Length);
		for (i=0; i<ArgCount; i++)
		{
			printf("Arg %d = %s\n\r", i, Arg[i]);
		}
	#endif

	// Arg 0 is the command
	Length = strlen(Arg[0]);

        //////////////////////////////////////////////////////
        // HELP command: print menu
        //
        if (strncmp(Arg[0], "help", Length) == 0)
        {
		PrintMenu(Mode, iState, iStrobe, iRate, iFixed, iLL);
        }
        else
	//////////////////////////////////////////////////////
	// QUIT command: just quit this program
	//mkfifo errors
	if (strncmp(Arg[0], "quit", Length) == 0)
	{
		end = 1;
	}
	else
	//////////////////////////////////////////////////////
	// RUN command: run the frame loop
	//
	if (strncmp(Arg[0], "run", Length) == 0)
	{
		iState = STATE_RUN;
		SendByte = RUN_CMD;
		write(cmd, &SendByte, 1);
	}
	else
	//////////////////////////////////////////////////////
	// STOP command: stop the frame loop
	//
	if (strncmp(Arg[0], "stop", Length) == 0)
	{
		iState = STATE_STOP;
		SendByte = STOP_CMD;
		write(cmd, &SendByte, 1);
	}
	else
        //////////////////////////////////////////////////////
        // STEP command: step the frame loop
        //
        if (strncmp(Arg[0], "step", Length) == 0)
        {
		if (iState == STATE_STOP)
		{
                	SendByte = STEP_CMD;
                	write(cmd, &SendByte, 1);
		}
		else
			printf("Can not step while running - issue stop command first...\n\r");
		
        }
        else
	//////////////////////////////////////////////////////
	// FREQUENCY command: change the frequency of the main loop
	//
	if (strncmp(Arg[0], "framerate", Length) == 0)
	{
		int rate = 0;
                if (iState == STATE_RUN)
                        printf("Can not change frame rate while running - issue stop command first...\n\r");
		else
		if (ArgCount != 2)
		{
			printf((const char *) "Wrong number of arguments - try again...\n\r");
			fflush(stdout);
		}
		else
		{
		rate = atoi(Arg[1]);

		if (rate < 10 || rate > 1000)
		{
			printf((const char *) "Bad parameter %s or out of range - try again...\n\r",
				Arg[1]);
			fflush(stdout);
		}
		else
		{	
			iRate = (long) rate;
			SendByte = FRAMERATE_CMD;
			write(cmd, &SendByte, 1);
			write(cmd, &iRate, sizeof(long));
		}
		}
	}
	else
        //////////////////////////////////////////////////////
        // PSTROBE command: change the frequency of the strobe (programmable)
        //
        if (strncmp(Arg[0], "pstrobe", Length) == 0)
        {
                int strobe = 0;
                if (iState == STATE_RUN)
                        printf("Can not change strobe while running - issue stop command first...\n\r");
                else
                if (ArgCount != 2)
                {
                        printf((const char *) "Wrong number of arguments - try again...\n\r");
                        fflush(stdout);
                }
                else
		{
                strobe = atoi(Arg[1]);

                if (strobe < 10 || strobe > 50)
                {
                        printf((const char *) "Bad parameter %s or out of range - try again...\n\r",
                                Arg[1]);
                        fflush(stdout);
                }
                else
                {
                        iStrobe = (short) strobe;
                        SendByte = STROBE_CMD;
                        write(cmd, &SendByte, 1);
                        write(cmd, &iStrobe, sizeof(short));
    			iFixed = 0; // programmable
    			write(cmd, &iFixed, sizeof(short));
                }
		}
        }
        else
        //////////////////////////////////////////////////////
        // FSTROBE command: change the frequency of the strobe (fixed)
        //
        if (strncmp(Arg[0], "fstrobe", Length) == 0)
        {
                if (iState == STATE_RUN)
                        printf("Can not change strobe while running - issue stop command first...\n\r");
                else
		{
                int strobe = 50; // use highest
                iStrobe = (short) strobe;
                SendByte = STROBE_CMD;
                write(cmd, &SendByte, 1);
                write(cmd, &iStrobe, sizeof(short));
                iFixed = 1; // programmable
                write(cmd, &iFixed, sizeof(short));
		}
        }
        else
	//////////////////////////////////////////////////////
	// CLEAR command: clear all actuator values
	//
	if (strncmp(Arg[0], "clear", Length) == 0)
	{
		SendByte = CLEAR_CMD;
		write(cmd, &SendByte, 1);
	}
	else
	//////////////////////////////////////////////////////
	// TEST command: setup test pattern
	//
	if (strncmp(Arg[0], "test", Length) == 0)
	{
		SendByte = TEST_CMD;
		write(cmd, &SendByte, 1);
	}
	else
        //////////////////////////////////////////////////////
        // HALTERR command: specify halt on error behavior
        //
        if (strncmp(Arg[0], "halterr", Length) == 0)
        {
                if (ArgCount != 2)
                {
                        printf((const char *) "Wrong number of arguments - try again...\n\r");
                        fflush(stdout);
                }
                else
                if (strcmp(Arg[1], "0") == 0)
                {
                        SendByte = HALTERR_CMD;
                        write(cmd, &SendByte, 1);
                        SendByte = 0;
                        write(cmd, &SendByte, 1);
                }
                else
                if (strcmp(Arg[1], "1") == 0)
                {
                        SendByte = HALTERR_CMD;
                        write(cmd, &SendByte, 1);
                        SendByte = 1;
                        write(cmd, &SendByte, 1);
                }
                else
                {
                        printf((const char *) "Bad parameter %s - try again...\n\r", Arg[1]);
                        fflush(stdout);
                }

        }
        else
	//////////////////////////////////////////////////////
	// LOAD command: load actuators from file
	//
	if (strncmp(Arg[0], "load", Length) == 0)
	{
		int i;
		short s;
		int Ret, iActValue;
		char ScanLine[32];
		FILE *fp = NULL;
		int Len = 0;
                if (ArgCount != 2)
                {
                        printf((const char *) "Wrong number of arguments - try again...\n\r");
                        fflush(stdout);
                }
                else
		{
		fp = fopen(Arg[1], "rt");
		if (fp == NULL)
		{
			printf((const char *) "Error opening file %s\n\r", Arg[1]);
		}
		else
		{
		int *pMap = NULL;

                if (iMode == 1024)
                {
                	if (iLL == 1)
                                pMap = g_i1kDMTBLLMap;
                        else
                                pMap = g_i1kDMTBMap;
                }
                else
                        pMap = g_i4kMap;

		s = 0;
		while(fgets(ScanLine, 12, fp) && s < iMode)
		{
			int Index = 0;
			Len = strlen(ScanLine);
			for (i=0; i<Len; i++)
			{
				if (ScanLine[i] == '\n')
					ScanLine[i] = '\0';
			}
			Ret = sscanf(ScanLine, "%x", &iActValue);

			Index = FindActuatorIndex(pMap, iMode, s);
			sActValue[Index] = (short) iActValue;
            s++;
		}
		fclose(fp);
		printf((const char *) "Loading %d actuator values...\n\r", s);
		fflush(stdout);

		SendByte = LOAD_CMD;
		write(cmd, &SendByte, 1);
		write(cmd, &s, 2);
		write(cmd, sActValue, s*sizeof(short));
		}
		}
	}
	else
	//////////////////////////////////////////////////////
	// POKE command: poke a single actuator with a value
	//
	if (strncmp(Arg[0], "poke", Length) == 0)
	{
                if (ArgCount != 3)
                {
                        printf((const char *) "Wrong number of arguments - try again...\n\r");
                        fflush(stdout);
                }
                else
                {
		unsigned short Index = 0;
		long Actuator = (long) strtol(Arg[1], (char **) NULL, 10);
		unsigned short Value    = (unsigned short) strtol(Arg[2], (char **) NULL, 10);
		long iMax  = (long) iMode;
		unsigned short maxValue = 300;    //Maximum voltage that can be sent to the DM
		
		if (Actuator < 1 || Actuator > iMax)
		{
			printf((const char *) "Actuator %d out of range - try again...\n\r", Actuator);
			fflush(stdout);
		}
		else if (Value < 0 || Value > maxValue)
		{
			printf((const char *) "Voltage %u out of range - must be between 0 and 300\n\r", Value);
		}
		else
		{
			Actuator--;
			if (iMode == 1024)
			{
				if (iLL == 1)
					Index = FindActuatorIndex(g_i1kDMTBLLMap, iMax, Actuator);
				else
					Index = FindActuatorIndex(g_i1kDMTBMap, iMax, Actuator);
			}
			else
				Index = FindActuatorIndex(g_i4kMap, iMax, Actuator);

			printf((const char *) "Poke actuator %d (index %d) with %u volts\n\r", Actuator, Index, Value);

            Value = (Value * 65535)/300;  //Convert the voltage to a value suitable for loading into the buffer

			fflush(stdout);
			SendByte = POKE_CMD;
			write(cmd, &SendByte, 1);
			write(cmd, &Index, sizeof(short));
			write(cmd, &Value, sizeof(short));
		}
		}
	}
	else
	{
		printf((const char *) "Unknown command: %s - try again...\n\r", Arg[0]);
		fflush(stdout);
	}
    }

    SendByte = TERM_CMD;	// send the TERM command to the thread	
    write(cmd, &SendByte, 1);	// write it to the FIFO

    // wait for thread to exit
    pthread_join( thread, NULL );
    //#ifdef MDL_DEBUG
    //   printf("RHL Main Cleanup\n");
    //#endif

    // close the FIFO
    close(cmd);

    // remove the FIFO
    unlink( "/tmp/mdlfifo" );

    exit(0);
}

