#ifndef __ISOREAD_H__

#define __ISOREAD_H__

int IsofileReadSectors(int lba, int nsectors, void *buf);//, int *eod
int IsofileGetDiscSize(int umdfd);


#endif


