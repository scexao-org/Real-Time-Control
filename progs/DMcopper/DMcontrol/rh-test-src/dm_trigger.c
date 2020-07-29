/*
 * DPIO2 RH-Linux Command Line Interface (RHCLI)
 *
 * Author: Michael A. DeLuca / O. Guyon
 * MDL Enterprises / Subaru Telescope
 * m.deluca@comcast.net / guyon@naoj.org
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
#include <sys/inotify.h>
#include <limits.h>
#include <sys/file.h>
#include <errno.h>


#include "dpio2funcs.h"
#include "shared.h"
#include "mapdata.h"



#define DMVfile "/milk/shm/dmv.bin"


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define NBact 1024



#define	STATE_RUN	0
#define STATE_STOP	1
#define MAX_COMMAND_ARGS 32             // command line processing
#define COMMAND_LENGTH 128
#define iMode 4096                      // mode: affects what translation table is used

static int   		end     = 0;		           // termination flag	
static int   		iLL     = 0;		           // Low Latency mode for 1K off
static int   		iState  = STATE_STOP;	       // default value for start/stop is stopped
static int              Mode    = iMode;
static long  		iRate   = 1000;		           // default frame rate frequency is 1000 Hz
static short 		iStrobe = 50;		           // default strobe rate frequency is 50 MHz
static short 		iFixed  = 0;		           // default strobe is programmable
static pthread_t    thread  = (pthread_t) NULL;
short  sActValue[iMode];

unsigned short *valarray;

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
  
  int inotifyFd, wd, j;
  long ii;
  int fd;
  char buf[BUF_LEN];
  ssize_t numRead;



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


  // WATCH DMV file
  inotifyFd = inotify_init(); /* Create inotify instance */
  if (inotifyFd == -1)
    {
      printf("ERROR: inotify_init\n");
      exit(0);
    }

  wd = inotify_add_watch(inotifyFd, DMVfile, IN_CLOSE_WRITE); //IN_ALL_EVENTS);
  if (wd == -1)
    {
      printf("ERROR: inotify_add_watch on file \"%s\"\n", DMVfile);
      exit(0);
    }
  printf("Watching %s using wd %d\n", DMVfile, wd);
  
  valarray = (unsigned short int*) malloc(sizeof(unsigned short int)*NBact);

   

  // unnecessary verifications... I think
  pMap = NULL;
  if (iMode == 1024) {
    if (iLL == 1) pMap = g_i1kDMTBLLMap;
    else          pMap = g_i1kDMTBMap;
  }
  else pMap = g_i4kMap;
  

  for(s=0;s<iMode;s++)
    {
      Index = FindActuatorIndex(pMap, iMode, s);
      sActValue[Index] = 0;
    }



  while (stat(dm_stop, &finfo)) 
    {
      numRead = read(inotifyFd, buf, BUF_LEN);
      if (numRead == 0)
	{
	  printf("ERROR: read() from inotify fd returned 0!");
	  exit(0);
	}
      if (numRead == -1)
	{
	  printf("ERROR: read\n");
	  exit(0);
	}
      
      fd = open(DMVfile, O_RDONLY);
      flock(fd, LOCK_EX);
      if( fd < 0 )
	{
	  perror( "Error opening file" );
	  printf( "Error opening file: %s\n", strerror( errno ) );
	}
      read(fd, valarray, sizeof(short int)*NBact);     
      
      flock(fd, LOCK_UN);
      close(fd);
      
      for(s=0;s<NBact;s++)
	{
	  Index = FindActuatorIndex(pMap, iMode, s);
	  sActValue[Index] = valarray[s];
	}
      
      
      printf("Loading actuator values [%d]\n", iMode);
      fflush(stdout);
      s = iMode;

      //   for(ii=0;ii<NBact;ii++)
      //	printf("[%ld,%d]",ii,valarray[ii]);

      
      SendByte = LOAD_CMD;
      write(cmd, &SendByte, 1);
      write(cmd, &s, 2);
      write(cmd, sActValue, s*sizeof(short));
      
      
      SendByte = STEP_CMD;
      write(cmd, &SendByte, 1);
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

  free(valarray);

  exit(0);
}
