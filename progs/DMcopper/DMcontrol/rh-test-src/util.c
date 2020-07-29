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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dpio2funcs.h"
#include "shared.h"

// help function prints this menu
void PrintMenu(int iMode, int iState, int iStrobe, int iRate, int iFixed, int iLL)
{
	printf("RH-Linux DPIO2 Command line Interface\n");
        printf("Version 2.2, %s\n", __DATE__ ", " __TIME__); 
	if (iFixed)
           printf("Mode: %d%s (%s), Rate: %d Hz, Fixed Strobe: 13.8 MHz\n\r", 
			iMode, iLL?"LL":"", iState?"Stopped":"Running", iRate);
	else
           printf("Mode: %d%s (%s), Rate: %d Hz, Programmable Strobe: %d MHz\n\r",
                        iMode, iLL?"LL":"", iState?"Stopped":"Running", iRate, iStrobe);

        printf("---------------------------------------------------\n\r");
        printf("Command           Meaning\n\r");
        printf("---------------------------------------------------\n\r");
        printf("run               Run framing loop\n\r");
        printf("stop              Stop framing loop\n\r");
	    printf("step              Step framing loop\n\r");
        printf("framerate <Hz>    Select frame rate (10 - 1000) Hz\n\r");
	    printf("pstrobe <MHz>     Select programmable strobe rate (10 - 50) MHz\n\r");
	    printf("fstrobe           Select fixed strobe rate (13.8 MHz)\n\r");
        printf("poke <N> <V>      Poke actuator <N> with voltage <V>\n\r");
        printf("clear             Clear all actuators to default values\n\r");
        printf("load <file>       Load all actuators from an ASCII file\n\r");
        printf("test              Load all actuators with a test pattern\n\r");
        printf("halterr <0|1>     Enable/disable halt on loop error\n\r");
        printf("quit              Quit the program\n\r");
        printf("help              Print this menu\n\r");
}

// helper function to look-up the alpha-numeric actuator in the provided list
// and return the index to support the POKE function
int FindActuatorIndex(int* Map, int iCount, int iActuator)
{
        int i, Index = -1;
	#ifdef MDL_DEBUG
	printf("look for %d in array of size %d\n", iActuator, iCount);
	#endif

        for (i=0; i<iCount; i++)
        {
                // compare the search string with the current actuator
                if (Map[i] == iActuator)
                {
                        // we found it, so record the index for return and break
			#ifdef MDL_DEBUG
	                printf((const char *) "Found actuator %d at index %d\n\r", iActuator, i);
                        fflush(stdout);
			#endif
                        Index = i;
                        break;
                }
        }
        return Index;
}



