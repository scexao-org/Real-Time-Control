#include "vmosa-dpio2.h"

#include <stdio.h>
#include <stdlib.h>

#include "dpio2.h"
/*#include "dpioLib.h"*/

#include "CDpio2Flash.h"
#include "SrecToBin.h"


#define   DPIO2_CMD_GET_FLASH_PTR   DPIO2_CMD_RESERVED_2

#define  DPIO2_PCI33_PCBA_FLASH_MANUFACTURER  (0x01)
#define  DPIO2_PCI33_PCBA_FLASH_DEVICE        (0xDA)
#define  DPIO2_PCI33_PCBB_FLASH_MANUFACTURER  (0x01)
#define  DPIO2_PCI33_PCBB_FLASH_DEVICE        (0xDA)

#define  DPIO2_PCI66_FLASH_MANUFACTURER  (0x01)
#define  DPIO2_PCI66_FLASH_DEVICE        (0xC4)

/*                                                                                           rsr
* Declare overloaded fuction prototype for dpio2Ioctl                                        rsr
*/                                                                                         //rsr
STATUS dpio2Ioctl(int devno, int command, void* argument);                                 //rsr


CDpio2Flash* createFlash(UINT32 baseAddress)
{

  CDpio2Flash*   pFlash;

  UINT8   manufacturerId;
  UINT8   deviceId;


  /* Create device object apprioriate for FLASH on PCB A,
   * and try to read the manufacturer ID and device ID.
   * If these IDs could be read, return the object.
   */
  pFlash = new CDpio2Pci33PcbAFlash(baseAddress);

  manufacturerId = pFlash->getManufacturerId(0);
  deviceId = pFlash->getDeviceId(0);

  printf("Manufacturer: %02X\n", manufacturerId);
  printf("Device      : %02X\n", deviceId);

  if ( (manufacturerId == DPIO2_PCI33_PCBA_FLASH_MANUFACTURER)
       && (deviceId == DPIO2_PCI33_PCBA_FLASH_DEVICE) ) {

    printf("DPIO2 PCB A\n");
    return (pFlash);

  } else {

    delete pFlash;

  }


  /* Create device object apprioriate for FLASH on PCB B,
   * and try to read the manufacturer ID and device ID.
   * If these IDs could be read, return the object.
   */
  pFlash = new CDpio2Pci33PcbBFlash(baseAddress);

  manufacturerId = pFlash->getManufacturerId(0);
  deviceId = pFlash->getDeviceId(0);

  printf("Manufacturer: %02X\n", manufacturerId);
  printf("Device      : %02X\n", deviceId);

  if ( (manufacturerId == DPIO2_PCI33_PCBB_FLASH_MANUFACTURER)
       && (deviceId == DPIO2_PCI33_PCBB_FLASH_DEVICE) ) {

    printf("DPIO2 PCB B\n");
    return (pFlash);

  } else {

    delete pFlash;

  }


  /* Create device object apprioriate for FLASH on DPIO2 66MHz,
   * and try to read the manufacturer ID and device ID.
   * If these IDs could be read, return the object.
   */
  pFlash = new CDpio2Pci66Flash(baseAddress);

  manufacturerId = pFlash->getManufacturerId(0);
  deviceId = pFlash->getDeviceId(0);

  printf("Manufacturer: %02X\n", manufacturerId);
  printf("Device      : %02X\n", deviceId);

  if ( (manufacturerId == DPIO2_PCI66_FLASH_MANUFACTURER)
       && (deviceId == DPIO2_PCI66_FLASH_DEVICE) ) {

    printf("DPIO2 66MHz\n");
    return (pFlash);

  } else {

    delete pFlash;

  }


  return (NULL);

}



int main(int argc, char* argv[])
{

  const UINT32   SIZE_OF_FLASH = (1024*1024);

  int   deviceNumber;

  STATUS   status;

  int   numberOfDpio2Modules;
  DPIO2_INFO_T   dpio2Info;

  UINT32   localBaseAddressOfFlash;
  
  CDpio2Flash*   pFlash;

  char   exoFileName[80];
  FILE*   exoFileDescriptor;

  UINT32   lengthOfExoFile;

  UINT32   numBytesRead;
  UINT32   numTotalBytesRead;  

  char*   pExoFileData;

  UINT8*   pBinaryData;
  UINT32   lengthOfBinaryData;

  UINT32   iByte;
  UINT32   iFirstImage;
  UINT32   iSecondImage; 

  UINT8   sectorFirstImage;
  UINT8   sectorSecondImage;

  UINT8   numberOfSectors;
  UINT8   iSector;

  UINT32   iData;

  UINT32   numberOfBytesInSector;
  UINT32   numberOfBytesToProgram;


  /* Check that the right number of arguments are given.
   */
  if ( argc != 2 ) {

    printf("FpgaFlashLoad needs 1 argument\n\n");
    printf("Syntax:\n");
    printf("   FpgaFlashLoad deviceNumber\n");

    return (-1);

  }

  deviceNumber = atoi(argv[1]);


  /* Get the base address of the flash.
   */
  numberOfDpio2Modules = dpio2Scan ( &dpio2Info);

  if ( numberOfDpio2Modules ==  ERROR ) {

    printf("Failed to scan for DPIO2 Devices\n");
    return (ERROR);

  }

  if ( numberOfDpio2Modules < 1 ) {

    printf("Failed find any DPIO2 Modules\n");
    return (ERROR);

  }

  if ( (deviceNumber < 1) || (deviceNumber > numberOfDpio2Modules) ) {
    
    printf("Cannot find DPIO2 module %d\n", deviceNumber);
    return ERROR;

  }

  status = dpio2Open ( deviceNumber, DPIO2_INPUT);

  if ( status != OK ) {

    printf("Cannot open DPIO2 modules\n");
    return ERROR;

  } else {

//    dpio2Ioctl(deviceNumber, DPIO2_CMD_GET_FLASH_PTR, (int)((unsigned long)&localBaseAddressOfFlash));
      dpio2Ioctl(deviceNumber, DPIO2_CMD_GET_FLASH_PTR, &localBaseAddressOfFlash);                             //rsr
    
    printf("Base address of flash: %08X\n", (int) localBaseAddressOfFlash);

    pFlash = createFlash(localBaseAddressOfFlash);

    if ( pFlash != NULL ) {

      printf("Name of EXO file: ");
      gets(exoFileName);

      exoFileDescriptor = fopen(exoFileName, "rt");
      if ( exoFileDescriptor == NULL ) {

        printf("Cannot open '%s' for reading\n", exoFileName);

      } else {

        /* Find the length of the file and allocate a buffer that can hold it.
       */
        fseek(exoFileDescriptor, 0, SEEK_END);
        lengthOfExoFile = ftell(exoFileDescriptor);
        fseek(exoFileDescriptor, 0, SEEK_SET);

        printf("The file '%s' contains %d characters.\n", 
	       exoFileName, (int) lengthOfExoFile);

        pExoFileData = (char*) malloc(lengthOfExoFile + 1);
        if ( pExoFileData == NULL ) {

          printf("Failed to allocate buffer for EXO file\n");

        } else {

          /* Allocate buffer for binary data.
         */
          pBinaryData = (UINT8*) malloc(SIZE_OF_FLASH);
          if ( pBinaryData == NULL ) {

            printf("Failed to allocated buffer for binary data\n");

          } else {

            /* Read the whole contents of the EXO file.
           */
            numTotalBytesRead = 0;
            while ( numTotalBytesRead < lengthOfExoFile ) {

              fseek(exoFileDescriptor, numTotalBytesRead, SEEK_SET);
              numBytesRead = fread((void*) (pExoFileData + numTotalBytesRead), 
                                   sizeof(char), 
                                   (lengthOfExoFile - numTotalBytesRead), 
                                   exoFileDescriptor);

              numTotalBytesRead += numBytesRead;

            }

        
            /* Decode the EXO file into the binary buffer.
           */
            lengthOfBinaryData = SIZE_OF_FLASH;
            SrecToBin( (unsigned char*) pExoFileData, 
		       (unsigned int) lengthOfExoFile, 
		       pBinaryData,
		       (unsigned int*) &lengthOfBinaryData);

            printf("EXO file is converted to %d bytes of binary data\n", 
		   (int) lengthOfBinaryData);

            /* Search for the start of the first image (marked by 0xFFFFFFFF).
           */
            iByte = 0;
            iFirstImage = lengthOfBinaryData;
            iSecondImage = lengthOfBinaryData;

            while ( (iByte < (lengthOfBinaryData - 3)) && (iFirstImage == lengthOfBinaryData) ) {

              if ( (pBinaryData[iByte + 0] == 0xFF)
                   && (pBinaryData[iByte + 1] == 0xFF)
                   && (pBinaryData[iByte + 2] == 0xFF)
                   && (pBinaryData[iByte + 3] == 0xFF)
                   && (pBinaryData[iByte + 4] == 0x55)
                   && (pBinaryData[iByte + 5] == 0x99)
                   && (pBinaryData[iByte + 6] == 0xAA)
                   && (pBinaryData[iByte + 7] == 0x66) ) {

                iFirstImage = iByte;

              }

              iByte++;

            }

            while ( (iByte < (lengthOfBinaryData - 3)) && (iSecondImage == lengthOfBinaryData) ) {

              if ( (pBinaryData[iByte + 0] == 0xFF)
                   && (pBinaryData[iByte + 1] == 0xFF)
                   && (pBinaryData[iByte + 2] == 0xFF)
                   && (pBinaryData[iByte + 3] == 0xFF)
                   && (pBinaryData[iByte + 4] == 0x55)
                   && (pBinaryData[iByte + 5] == 0x99)
                   && (pBinaryData[iByte + 6] == 0xAA)
                   && (pBinaryData[iByte + 7] == 0x66) ) {

                iSecondImage = iByte;

              }

              iByte++;

            }

            printf("lengthOfBinaryData = %08X\n", (int) lengthOfBinaryData);
            printf("iFirstImage        = %08X\n", (int) iFirstImage);
            printf("iSecondImage       = %08X\n", (int) iSecondImage);


            /* Program the first FPGA image into the specified sectors.
           */
            sectorFirstImage = 0;
            sectorSecondImage = 8;

            numberOfSectors = pFlash->getNumberOfSectors();

            iSector = sectorFirstImage;
            iData = iFirstImage;
            while ( (iSector < numberOfSectors) && (iData < iSecondImage) ) {

              pFlash->getSizeOfSector(iSector, numberOfBytesInSector);

              if ( numberOfBytesInSector > (iSecondImage - iData) ) {

                numberOfBytesToProgram = iSecondImage - iData;

              } else {

                numberOfBytesToProgram = numberOfBytesInSector;

              }

              pFlash->eraseAndProgramSector(iSector, (pBinaryData + iData), numberOfBytesToProgram);

              iSector++;
              iData += numberOfBytesToProgram;

            }

        
            /* Program the second FPGA image into the specified sectors.
         */
            iSector = sectorSecondImage;
            iData = iSecondImage;
            while ( (iSector < numberOfSectors) && (iData < lengthOfBinaryData) ) {

              pFlash->getSizeOfSector(iSector, numberOfBytesInSector);

              if ( numberOfBytesInSector > (lengthOfBinaryData - iData) ) {

                numberOfBytesToProgram = lengthOfBinaryData - iData;

              } else {

                numberOfBytesToProgram = numberOfBytesInSector;

              }

              pFlash->eraseAndProgramSector(iSector, (pBinaryData + iData), numberOfBytesToProgram);

              iSector++;
              iData += numberOfBytesToProgram;

            }

        
            free(pBinaryData);

          }

          free(pExoFileData);

        }

        fclose(exoFileDescriptor);

      }

      delete pFlash;

    }

    dpio2Close ( deviceNumber );

  }


  return (0);

}

