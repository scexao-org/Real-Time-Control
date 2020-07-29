#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    int i = 0;
    int istring = 0;
    int length = 30;
    char readline[length];
    char* result = NULL;
    char delim[] = " ";

    FILE* fpin = NULL;
    FILE* fpout = NULL;

    fpin = fopen("dmv_vmap.txt", "r");
    fpout = fopen("vmap.txt", "w");

    for (i = 1; i<=1024; i++)
    {
        fgets(readline, length - 4, fpin);
        result = strtok(readline, delim);
        result = strtok(NULL, delim);
        result = strtok(NULL, delim);
        istring = atoi(result)*65535/300;
        fprintf(fpout, "%04X\n", istring);
    }
    for (i = 1025; i<=4096; i++)
    {
		fprintf(fpout, "%04X\n", 0);
	}
    fclose(fpin);
    fclose(fpout);

    return 0;
}
