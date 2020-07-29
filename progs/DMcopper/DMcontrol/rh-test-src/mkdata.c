/*
 * DPIO2 RH-Linux Data Generation Utility
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


int main(int argc,char *argv[])
{
	int    i = 0;
	FILE *fp = NULL;

	fp = fopen("data1024.txt", "w");
	for (i=0; i<1024; i++)
		fprintf(fp, "%04X\n", 30583);
	fclose(fp);

	fp = fopen("data4096.txt", "w");
        for (i=0; i<4096; i++)
        {
			fprintf(fp, "%04X\n", 30583);
        }
        fclose(fp);

    	exit(0);
}
                                                                                                              
