#ifndef __UMD9660_DRIVER_H__

#define __UMD9660_DRIVER_H__

#define SECTOR_SIZE	0x800


typedef struct
{
  int   lba; 
  void  *buf;
  int   nsectors;
} UmdReadParams;

int OpenIso();
int ReadUmdFileRetry(void *buf, int size, int fpointer);
int Umd9660ReadSectors(int lba, void *buf, int nsectors);
int Umd9660ReadSectors3(UmdReadParams *read_params);

int umd9660_init();

int GetIsoDiscSize();

#endif

