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

int msleep(unsigned long milisec) {
  struct timespec req={0};
  time_t sec=(int)(milisec/1000);
  milisec=milisec-(sec*1000);
  req.tv_sec=sec;
  req.tv_nsec=milisec*1000000L;
  while(nanosleep(&req,&req)==-1)
    continue;
  return 1;
}

// main entry (no args used in command line)
int main(int argc,char *argv[]) {
  // -----------------------------------
  struct stat finfo; // file information
  char dm_updt[50], dm_stop[50], mycmd[50], dmv_fname[50];
  FILE *dmv;
  // -----------------------------------

  int iRet = 0;
  char SendByte;
  char Commandline[COMMAND_LENGTH], LastCommand[COMMAND_LENGTH];
  int cmd = -1, i = 0;//, /*j = 0, */ArgCount = 0, Length = 0;
  //char* Arg[MAX_COMMAND_ARGS];
  int numberOfDpio2Modules = 0;
  int* pnumberOfDpio2Modules = &numberOfDpio2Modules;
  
  
  if (scanForDevices (pnumberOfDpio2Modules) != OK) return -1;
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
  if ((cmd = open("/tmp/mdlfifo", O_WRONLY)) < 0) {
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
  usleep(1000); // 10 millisec
  
  // print the help menu on startup
  //PrintMenu(Mode, iState, iStrobe, iRate, iFixed, iLL);


  // ---------------------------------------------------
  //            define the trigger files
  // ---------------------------------------------------
  sprintf(dm_updt,   "%s", "dm_update");
  sprintf(dm_stop,   "%s", "dm_stop");
  sprintf(dmv_fname, "%s", "dmv0.txt");

  // unless a file called dm_stop is created, in which
  // case everything stops, the program keeps waiting 
  // for update signals, loads a file called dmv.txt 
  // and sends its content to the DM driver...

  int Index = 0, Ret, iActValue, Len = 0;
  char ScanLine[32];
  short s;
  int *pMap;

  printf("Welcome!\n");
  fflush(stdout);

  while (stat(dm_stop, &finfo)) {

    // a DM update was requested!
    if (!stat(dm_updt, &finfo)) {
      // -----------------------------------
      // (1) delete the update trigger file
      // -----------------------------------
      sprintf(mycmd, "rm -f %s", dm_updt);
      system(mycmd);
      
      // ----------------------------
      // (2) proceed with the update
      // ----------------------------
      dmv = NULL;
      if ((dmv = fopen(dmv_fname, "rt")) == NULL) {
	printf("Error: %s file cannot be found!", dmv_fname);
	fflush(stdout);
      } else {
	// unnecessary verifications... I think
	pMap = NULL;
	if (iMode == 1024) {
	  if (iLL == 1) pMap = g_i1kDMTBLLMap;
	  else          pMap = g_i1kDMTBMap;
	}
	else pMap = g_i4kMap;
	
	// read the dmv file
	s = 0;
	while(fgets(ScanLine, 12, dmv) && s < iMode) {
	  Index = 0;
	  Len = strlen(ScanLine);
	  for (i=0; i<Len; i++)
	    if (ScanLine[i] == '\n') ScanLine[i] = '\0';
	  Ret = sscanf(ScanLine, "%x", &iActValue);
	  
	  Index = FindActuatorIndex(pMap, iMode, s);
	  sActValue[Index] = (short) iActValue;
	  s++;
	}
	fclose(dmv);
	printf("Loading %d actuator values\n", s);
	fflush(stdout);

	SendByte = LOAD_CMD;
	write(cmd, &SendByte, 1);
	write(cmd, &s, 2);
	write(cmd, sActValue, s*sizeof(short));
	
      }
      // ----------------------------
      // (3) "step" the command
      // ----------------------------
      SendByte = STEP_CMD;
      write(cmd, &SendByte, 1);
      
    } else {
      usleep(1000);
    }
  }
  // ----------------------------------
  // the stop - trigger has appeared,
  // the program is going to stop
  // ----------------------------------
  sprintf(mycmd, "rm -f %s", dm_stop);
  system(mycmd);

  SendByte = TERM_CMD;	        // send TERM command to thread	
  write(cmd, &SendByte, 1);	// write it to the FIFO
  pthread_join( thread, NULL ); // wait for thread to exit
  close(cmd);                   // close the FIFO
  unlink( "/tmp/mdlfifo" );     // remove the FIFO
  exit(0);
}
