#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <globals.h>
#include <systemctrl.h>
#include "systemctrl_private.h"
#include "imports.h"

static char *cache_buffer = NULL;

//cache offset
static u32 cache_offset = 0;
static u32 cache_flag = 0;

int (*OldIoRead)(PspIoDrvFileArg *arg, char *data, int len) = NULL;
int (*OldIoWrite)(PspIoDrvFileArg *arg, const char *data, int len) = NULL;
SceOff ( *OldIoLseek)(PspIoDrvFileArg *arg, SceOff ofs, int whence) = NULL;

//write
int NewIoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
	if( cache_flag != 0 ) {
		SceOff offset = OldIoLseek( arg , 0 , PSP_SEEK_CUR );
		if( offset >= 0 )
		{
			u32 sector_offset = offset / 512;
			if( cache_offset >= sector_offset ){
				cache_flag = 0;
			}else if( sector_offset < (cache_offset + 8)) {
				cache_flag = 0;
			}
			
		}else{
			cache_flag = 0;
		}
	}

	return OldIoWrite( arg , data , len );
}

int NewIoRead(PspIoDrvFileArg *arg, char *out, int len)
{
	char *data = out;
	SceOff offset = OldIoLseek( arg , 0 , PSP_SEEK_CUR );
	if( offset < 0)
	{
		cache_flag = 0;
		goto RETURN;
	}

	u32 sector_cnt = len / 512;
	u32 sector_offset = offset / 512;

	if( cache_flag != 0)
	{
		if(
			( sector_offset < cache_offset )||
			( sector_offset >= (cache_offset + 8) ) )
		{
			//recache
			goto DO_CACHE;
		}
		else
		{
			//load cache
			u32 limit_cnt = sector_offset - cache_offset;
			u32 read_cnt =  8 - limit_cnt;
			if(read_cnt > sector_cnt)
				read_cnt = sector_cnt;
			
			u32 read_size = read_cnt << 9;
			memcpy( data , cache_buffer + (limit_cnt << 9) ,  read_size );
//			printf("Hit! 0x%08X\n", read_size);

			OldIoLseek( arg , read_size , 1 );//lseek

			sector_cnt -= read_cnt;
			if(sector_cnt <= 0)
				return len;

			data += read_size;
			OldIoRead( arg , data , sector_cnt << 9 );
			return len;
		}
	}
	else
	{
DO_CACHE:
		if( sector_cnt <= 4 )
		{
			cache_offset = sector_offset;

			if( OldIoRead( arg , cache_buffer , 512 * 8 ) == (512 * 8) )
			{
				cache_flag = 1;
				memcpy( data , cache_buffer , len );
				OldIoLseek( arg , offset + len , 0);
				return len;
			}

			OldIoLseek( arg , offset , 0);
			cache_offset = 0;
			cache_flag = 0;
		}
	}

RETURN:
	return OldIoRead( arg , out , len );
}



int msstorCacheInit(const char* driver, int bufSize)
{
	PspIoDrv *fast_ms = sctrlHENFindDriver(driver);
    PspIoDrvFuncs *new_funcs=fast_ms->funcs;

    OldIoRead = new_funcs->IoRead;
    OldIoWrite = new_funcs->IoWrite;
    OldIoLseek = new_funcs->IoLseek;

    new_funcs->IoRead  = NewIoRead;
    new_funcs->IoWrite = NewIoWrite;

    SceUID id = sceKernelAllocPartitionMemory( 1 , "", 0 , 4096 + 64 , NULL);
    cache_buffer =sceKernelGetBlockHeadAddr( id );				
    if ((((u32)cache_buffer) % 64) != 0)
    {
        cache_buffer += (64 - (((u32)cache_buffer) % 64));
    }			

    cache_flag = 0;

    flushCache();
	
    return 0;
}

void msstorCacheStat(){

}

void msstorCacheDisable(){

}

