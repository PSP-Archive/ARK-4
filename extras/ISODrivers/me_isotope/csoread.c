#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>
#include <string.h>

#include "systemctrl_me.h"
#include "csoread.h"
#include "umd9660_driver.h"

extern int sceKernelDeflateDecompress(void *dst,int dsize,void *src,int *pparam);

#define DEV_NAME "CSO"

// index buffer size
#define CISO_INDEX_SIZE (512/4)//128

// compresed data buffer cache size
#define CISO_BUF_SIZE 0x2000


/****************************************************************************
****************************************************************************/
static u8 *ciso_read_buf = NULL;//dataF80
static u8 *ciso_data_buf = NULL;//dataF84

static u32 ciso_index_buf[CISO_INDEX_SIZE] __attribute__((aligned(64)));//dataFC0 ~ 

static u32 ciso_buf_pos;    // file poisiotn of the top of ciso_data_buf//data11C0
static u32 ciso_cur_index;  // index(LBA) number of the top of ciso_index_buf//data11C4

// header buffer
static CISO_H ciso;//data11C8

/****************************************************************************
	Mount UMD event callback
****************************************************************************/
static int max_sectors;//data11E0

//sub_00000670:
int CisoOpen(int umdfd)
{
	int result;

	// check CISO header
	ciso.magic = 0;
	ciso_buf_pos  = 0x7FFFFFFF;//data11C0

	sceIoLseek(umdfd, 0, PSP_SEEK_SET);
	result = sceIoRead(umdfd, &ciso, sizeof(ciso)/*24*/);
	if(result<0)
	{
		return result;
	}

	if( ciso.magic == 0x4F534943 )//C I S O
	{
		//log_iso("CISO found\n");

		max_sectors = (int)(ciso.total_bytes) / ciso.block_size;
		ciso_cur_index = 0xffffffff;//data11C4

		if (!ciso_data_buf)//dataF84
		{
			ciso_data_buf = (u8 *)sctrlKernelMalloc(CISO_BUF_SIZE+64);
			
			if (!ciso_data_buf)
				return -1;

			if ((((u32)ciso_data_buf) % 64) != 0)
			{
				ciso_data_buf += (64 - (((u32)ciso_data_buf) % 64));
			}			
		}

		if(!ciso_read_buf)//dataF80
		{
			ciso_read_buf = (u8 *)sctrlKernelMalloc( SECTOR_SIZE );

			if(!ciso_read_buf)
				return -1;
		}
		return 0;
	}

	//log_iso("not CISO\n");

	// header check error
	return SCE_KERNEL_ERROR_NOFILE;//0x8002012F
}

/****************************************************************************
	get file pointer in sector
****************************************************************************/
//sub_00005988
static int inline ciso_get_index(u32 sector,int *pindex)
{
	int result;
	int index_off;

	// search index
	index_off = sector - ciso_cur_index;//sector - dataAA44

	if((ciso_cur_index==0xffffffff) || (index_off<0) || (index_off>=CISO_INDEX_SIZE) )
	{
		// out of area
		//		log_iso("CISO READ INDEX sector=%X , loc = %X\n",sector,sizeof(ciso)+sector*4);
		result = ReadUmdFileRetry(ciso_index_buf, sizeof(ciso_index_buf), sizeof(ciso)+ sector*4);//sub_000051A0
		

		if(result<0) return result;

		ciso_cur_index = sector;
		index_off = 0;//a3=0
	}

	//log_iso("CISO INDEX base %X offset %X\n",ciso_cur_index,index_off);

	// get file posision and sector size
	*pindex = ciso_index_buf[index_off];//dataA840
	return 0;
}

/****************************************************************************
	Read one sector
****************************************************************************/

//sub_00000790
static int ciso_read_one(void *buf,int sector)
{
	int result;
	int index,index2;
	int dpos,dsize;


	// get current index
	result = ciso_get_index(sector,&index);//sub_00005988
	if(result<0) 
	{
		return result;
	}

	// get file posision and sector size
	dpos  = (index & 0x7fffffff) << ciso.align;

	if(index & 0x80000000)
	{
		// plain sector
		//log_iso("CISO plain read fpos=%08X\n",dpos);

		result = ReadUmdFileRetry(buf, 0x800, dpos);//sub_000051A0
	
		return result;
	}

	// compressed sector
	//log_iso("CISO compress read\n");

	// get sectoer size from next index
	result = ciso_get_index(sector+1,&index2);
	if(result<0) return result;
	
	dsize = ((index2 & 0x7fffffff) << ciso.align) - dpos;
	
	// adjust to maximum size for scramble(shared) sector index
	if( (dsize <= 0) || (dsize > 0x800) ) dsize = 0x800;

#ifdef CISO_BUF_SIZE
	//SWITCH_THREAD();
	// read sector buffer
	if( (dpos < ciso_buf_pos) || ( (dpos+dsize) > (ciso_buf_pos+CISO_BUF_SIZE))  )
	{
		// seek & read
		result = ReadUmdFileRetry(ciso_data_buf, CISO_BUF_SIZE, dpos);//sub_000051A0
		//SWITCH_THREAD();
		if(result<0)
		{
			ciso_buf_pos = 0xfff00000; // set invalid position
			return result;
		}
		ciso_buf_pos = dpos;
	}
	result = sceKernelDeflateDecompress(buf, 0x800, ciso_data_buf + dpos - ciso_buf_pos , NULL);

#else
	// seek
	// read compressed data
	//result = dhReadFileRetry(&ciso_fd,dpos,ciso_data_buf,dsize);
	result = ReadUmdFileRetry(ciso_data_buf, dsize, dpos);
	if(result<0) return result;

	result = sceKernelDeflateDecompress(buf,0x800,ciso_data_buf,NULL);
	SWITCH_THREAD();

#endif

	//log_iso("result sceKernelDeflateDecompress %08X\n",result);
	if(result<0) return result;

	return 0x800;
}

/****************************************************************************
	Read Request
****************************************************************************/
//loc_00000998
int CisofileReadSectors(void *buf,int read_size, int fpointer )
{
	int result;
	int i;
	int ret=0;
	int offset;

//	int nsectors = fpointer / SECTOR_SIZE;
	int nsectors = fpointer >> 11;

	int size = (fpointer & 0x7FF);

	if( size )
	{
		result = ciso_read_one( ciso_read_buf , nsectors );//sub_00000790
		if(result < 0)
		{
			return result;
		}

		offset = size;
		size = SECTOR_SIZE - offset;

		if(size > read_size)
			size = read_size;

		memcpy( buf , ciso_read_buf + offset , size );

		ret = size;
		read_size -= size;
		buf += size;
		nsectors++;

	}

	int cnt = 0;
	
	if(read_size)
		cnt = read_size / SECTOR_SIZE;
//		cnt = read_size >> 11;

	if(cnt > 0)
	{
		for(i=0;i < cnt ;i++)
		{
			result = ciso_read_one( buf, nsectors );//sub_00000790
			if(result < 0)
			{
				return result;
			}

			ret += 0x800;
			read_size -= 0x800;
			buf += 0x800;
			nsectors++;
		}
	}

	if(read_size)
	{
		result = ciso_read_one( ciso_read_buf , nsectors );//sub_00000790
		if(result < 0)
		{
			return result;
		}

		memcpy( buf , ciso_read_buf , read_size );

		ret += read_size;

	}

	return ret;
}

/****************************************************************************/

//loc_0000064C:
int CisofileGetDiscSize(int umdfd)
{
	return (int)(ciso.total_bytes) / ciso.block_size;
}

