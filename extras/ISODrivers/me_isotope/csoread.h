#ifndef __CISOREAD_H__
#define __CISOREAD_H__

/*
	complessed ISO(9660) header format
*/
typedef struct ciso_header
{
	u32 magic	;					/* +00 : 'C','I','S','O'  0x4F534943               */
	unsigned long header_size;		/* +04 : header size (==0x18)                      */
	unsigned long long total_bytes;	/* +08 : number of original data size              */
	unsigned long block_size;		/* +10 : number of compressed block size           */
	unsigned char ver;				/* +14 : version 01                                */
	unsigned char align;			/* +15 : align of index (offset = index[n]<<align) */
	unsigned char rsv_06[2];		/* +16 : reserved                                  */
#if 0
// INDEX BLOCK
	unsigned int index[0];			/* +18 : block[0] index (data offset = index<<align) */
	unsigned int index[1];			/* +1C : block[1] index (data offset = index<<align) */
             :
             :
	unsigned int index[last];		/* +?? : block[last]                                 */
	unsigned int index[last+1];		/* +?? : end of last data point                      */
// DATA BLOCK
	unsigned char data[];			/* +?? : compressed data                            */
#endif
}CISO_H;

int CisoOpen(int umdfd);
int CisofileGetDiscSize(int umdfd);
//int CisofileReadSectors(int lba, int nsectors, void *buf);
int CisofileReadSectors(void *buf,int read_size, int fpointer );

#endif

