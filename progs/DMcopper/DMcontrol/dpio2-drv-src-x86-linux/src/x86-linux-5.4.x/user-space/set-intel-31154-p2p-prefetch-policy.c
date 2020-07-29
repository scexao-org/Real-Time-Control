#include "vmosa-dpio2.h"

#include <stdio.h>
#include <stdlib.h>
#include "string.h"     /* memset, strncmp,.. */



static int h2i (char* input);




int main(int argc, char **argv)
{

  int  status;  

  int  prefetch_policy;

  int  cacheline_size;


  if ( argc == 1 ) {
    
    printf ("input param: <prefetch policy> <cacheline size>\n");
    printf ("optimal prefetch policy value is 0x3ff\n");
    printf ("optimal cacheline size depend the system. Try 0x8, 0x10, 0x20\n");
    return OK;

  }
    
  if ( ( argc < 3 ) || ( argc > 3 ) ) {

    printf ("ERROR: input param: <prefetch policy> <cacheline size>\n");
    return ERROR;

  }


  prefetch_policy = h2i ( argv [1] );

  cacheline_size = h2i ( argv [2] );

  printf ("prefetch_policy 0x%x\n",prefetch_policy);
  printf ("cacheline_size 0x%x\n",cacheline_size);

  status = set_intel_31154_p2p_prefetch_policy ( prefetch_policy, 
						 cacheline_size );
     
  if ( status == ERROR ) {
    
    printf ("Non intel 31154 P2P brigde found!\n");
    
  } else {

    printf ("Found %d Intel 31154 P2P brigde\n",status);
    
  }
  

  return OK;

}





static int h2i (char* input)
{
  int   dec;
  char  c[3];
  int   val = 0;
  int   i;
  int   dig = 0;
  char* arg;


  sprintf (c, "0x");
  arg = input;

  /* hex or dec */
  dec = strncmp (c, arg, 2);
  if (!dec)
    arg = arg + 2;
  
  if (dec) {
    val = atoi (arg);
  }
  else {
    for (i=0; (((arg[i]>='0')&&(arg[i]<='9')) || 
               ((arg[i]>='a')&&(arg[i]<='f'))); i++) {
      if ((arg[i]>='0')&&(arg[i]<='9')) {
        dig = arg[i] - '0';
      }
      if ((arg[i]>='a')&&(arg[i]<='f')) {
        dig = 10 + arg[i] - 'a';
      }
      val = (val * 0x10) + dig;
    }
  }

  return val;
}

