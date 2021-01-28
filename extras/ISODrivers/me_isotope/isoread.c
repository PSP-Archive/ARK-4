#include <pspsdk.h>
#include <pspkernel.h>

#include "umd9660_driver.h"

/*
//loc_00005934:
int IsofileReadSectors(int lba, void *buf, int nsectors)
{
	int read = ReadUmdFileRetry(buf, SECTOR_SIZE*nsectors, lba*SECTOR_SIZE);//sub_000051A0

	if (read < 0)
	{
		return read;
	}

	read = read / SECTOR_SIZE;
	
	return read;	
}
*/

//loc_000058A8:
int IsofileGetDiscSize(int umdfd)
{
	int ret = sceIoLseek(umdfd, 0, PSP_SEEK_CUR);
	int size = sceIoLseek(umdfd, 0, PSP_SEEK_END);

	sceIoLseek(umdfd, ret, PSP_SEEK_SET);

	if (size < 0)
		return size;

	return size / SECTOR_SIZE;
}

