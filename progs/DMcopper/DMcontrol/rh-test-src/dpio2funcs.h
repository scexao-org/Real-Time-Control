/* Includes */
#include "stdio.h"      /* printf, ... */
#include "stdlib.h"     /* malloc,.. */
#include "sys/user.h"   /* PAGE_SIZE */
//#include "asm/page.h"   /* PAGE_SIZE */
#include "pthread.h"    /* pthread_attr_init ,pthread_create, .... */ 
#include "time.h"       /* nanosleep */
#include "unistd.h"     /* sleep, close, ... */
#include <malloc.h>     /* memalign */
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

#include "vmosa.h"
#include "dpio2.h"

/* Functions */

#ifdef __cplusplus
extern "C" {
#endif

void dmaDoneNotifier (sem_t*  pDmaDoneSemaphore);
STATUS scanForDevices (int* pNumDpio2Modules);
STATUS openDpio2ForOutput (int dpio2DeviceNumber);
STATUS configureDmaDoneInterrupt (int dpio2DeviceNumber, FUNCPTR pFunction, sem_t* pDmaDoneSemaphore);
STATUS enableStrobeGeneration (int dpio2DeviceNumber, unsigned int* pResultingFrequency, int Mode);
STATUS activateFpdpInterface (int dpio2DeviceNumber);
STATUS allocateAndLockDmaBuffer (size_t lengthInBytes, void** buffer, DPIO2_DMA** ppPageInfo);         //rsr
STATUS copyToKernel (void* kernelAddress, void* userAddress, int iLength);
STATUS configureDmaChain(int dpio2DeviceNumber, DPIO2_DMA* pPageInfo, int desc_num);
STATUS startDmaTransfer (int dpio2DeviceNumber, int desc_num);
void unlockAndFreeDmaBuffer (DPIO2_DMA* pPageInfo);                                                    //rsr
void closeDevice(int dpio2DeviceNumber);

#ifdef __cplusplus
}
#endif
