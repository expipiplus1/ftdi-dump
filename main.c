/*
	To build use the following gcc statement
	(assuming you have the d2xx library in the /usr/local/lib directory).
	gcc -o read main.c -L. -lftd2xx -Wl,-rpath,/usr/local/lib
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ftd2xx.h"

int print_user_data();

int main(int argc, char *argv[])
{
	FT_STATUS	ftStatus;
	FT_HANDLE	ftHandle0;
	int iport;
	static FT_PROGRAM_DATA Data;
	static FT_DEVICE ftDevice;
	DWORD libraryVersion = 0;
	int retCode = 0;

	ftStatus = FT_GetLibraryVersion(&libraryVersion);
	if (ftStatus == FT_OK)
	{
		printf("Library version = 0x%x\n", (unsigned int)libraryVersion);
	}
	else
	{
		printf("Error reading library version.\n");
		return 1;
	}

	if(argc > 1) {
		sscanf(argv[1], "%d", &iport);
	}
	else {
		iport = 0;
	}
	printf("Opening port %d\n", iport);

	ftStatus = FT_Open(iport, &ftHandle0);
	if(ftStatus != FT_OK) {
		/*
			This can fail if the ftdi_sio driver is loaded
		 	use lsmod to check this and rmmod ftdi_sio to remove
			also rmmod usbserial
		 */
		printf("FT_Open(%d) failed\n", iport);
		return 1;
	}

	printf("FT_Open succeeded.  Handle is %p\n", ftHandle0);

	ftStatus = FT_GetDeviceInfo(ftHandle0,
	                            &ftDevice,
	                            NULL,
	                            NULL,
	                            NULL,
	                            NULL);
	if (ftStatus != FT_OK)
	{
		printf("FT_GetDeviceType FAILED!\n");
		retCode = 1;
		goto exit;
	}

	printf("FT_GetDeviceInfo succeeded.  Device is type %d.\n",
	       (int)ftDevice);

	/* MUST set Signature1 and 2 before calling FT_EE_Read */
	Data.Signature1 = 0x00000000;
	Data.Signature2 = 0xffffffff;
	Data.Manufacturer = (char *)malloc(256); /* E.g "FTDI" */
	Data.ManufacturerId = (char *)malloc(256); /* E.g. "FT" */
	Data.Description = (char *)malloc(256); /* E.g. "USB HS Serial Converter" */
	Data.SerialNumber = (char *)malloc(256); /* E.g. "FT000001" if fixed, or NULL */
	if (Data.Manufacturer == NULL ||
	    Data.ManufacturerId == NULL ||
	    Data.Description == NULL ||
	    Data.SerialNumber == NULL)
	{
		printf("Failed to allocate memory.\n");
		retCode = 1;
		goto exit;
	}

	ftStatus = FT_EE_Read(ftHandle0, &Data);
	if(ftStatus != FT_OK) {
		printf("FT_EE_Read failed\n");
		retCode = 1;
		goto exit;
	}

	if (ftDevice != FT_DEVICE_2232H){
    printf("FTDI Chip isn't a 2232H, is this a Nexys Video board?\n");
    goto exit;
  }

	printf("FT_EE_Read succeeded.\n\n");

	printf("Signature1 = %d\n", (int)Data.Signature1);
	printf("Signature2 = %d\n", (int)Data.Signature2);
	printf("Version = %d\n", (int)Data.Version);

	printf("VendorId = 0x%04X\n", Data.VendorId);
	printf("ProductId = 0x%04X\n", Data.ProductId);
	printf("Manufacturer = %s\n", Data.Manufacturer);
	printf("ManufacturerId = %s\n", Data.ManufacturerId);
	printf("Description = %s\n", Data.Description);
	printf("SerialNumber = %s\n", Data.SerialNumber);
	printf("MaxPower = %d\n", Data.MaxPower);
	printf("PnP = %d\n", Data.PnP) ;
	printf("SelfPowered = %d\n", Data.SelfPowered);
	printf("RemoteWakeup = %d\n", Data.RemoteWakeup);

  printf("\n");

  const int read_user_data = print_user_data(ftHandle0);

  if(!read_user_data){
    printf("Failed to read user data\n");
  }

exit:
	free(Data.Manufacturer);
	free(Data.ManufacturerId);
	free(Data.Description);
	free(Data.SerialNumber);
	FT_Close(ftHandle0);
	printf("Returning %d\n", retCode);
	return retCode;
}

unsigned long hash(unsigned char *str, const DWORD length)
{
  unsigned long hash = 5381;
	DWORD c = 0;
	for(c = 0; c < length; ++c){
    hash = ((hash << 5) + hash) + str[c]; /* hash * 33 + c */
	}

  return hash;
}

int print_user_data(FT_HANDLE ftHandle0){
  printf("Reading User Data\n");

	unsigned char * pucUAdata;
	DWORD 	dwUASize, dwUARead;
	FILE * fp;
	FT_STATUS	ftStatus;

	ftStatus = FT_EE_UASize(ftHandle0, &dwUASize);
	if(ftStatus == FT_OK)
		printf("dwUASize = %d\n", (int)dwUASize);
	else {
		printf("Could not read UA size\n");
		return 0;
	}
	pucUAdata = (unsigned char *)malloc(dwUASize);
	if(pucUAdata == NULL) {
		printf("Out of resources\n");
		return 0;
	}
	ftStatus = FT_EE_UARead(ftHandle0, pucUAdata, dwUASize, &dwUARead);
	if(ftStatus == FT_OK) {
		fp = fopen("UA_DATA.bin", "w+");
		fwrite(pucUAdata, 1, dwUARead, fp);
		fclose(fp);
	}
	else{
		printf("could not read UA\n");
	}

  DWORD c = 0;
  for(c = 0; c < dwUASize; ++c){
    if(c % 8 == 0){
      printf("\n");
    }
    printf("0x%02x ", pucUAdata[c]);
  }
  printf("\n");

	printf("0x%08lx\n", hash(pucUAdata, dwUASize));

	free(pucUAdata);
  printf("Successfully read user data.\n\n");
  return 1;
}


