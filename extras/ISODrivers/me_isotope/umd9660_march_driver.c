
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>

#include <stdio.h>
#include <string.h>

#include "systemctrl_me.h"
#include "isoread.h"
#include "csoread.h"
#include "umd9660_driver.h"


SceUID umd_sema = -1;	//data2740
SceUID umdfd = -1;		//data23D0
int umd_file_len = 0x7FFFFFFF;	//data23D4 (sector len)
//int DiscType = 0x10;	//
//int DiscFlag = 1;		//
//int umd_cur_sector = -1;//
u8 *sectorbuf = NULL;	//data2484
int umd_is_cso;			//data2488
int umd_open = 0;		//data248C
//int data7DE4 = -1;		//
//SceOff umd_cur_offset; //
char *umdfilename;	//data2790


UmdReadParams umd_read_params;//data2784

void sceKernelSetQTGP3(void*);

typedef struct
{
	int flag;
	int offset;
}IO_STATUS;

IO_STATUS io_status[8];//data2744
//+0 index
//+4 offset

typedef struct
{
	const char *id;
	int type;
}UMD_ID_LIST;

UMD_ID_LIST disc_list[] = {
	{ "ULES-00124", 1},
	{ "ULUS-10019", 1},
	{ "ULJM-05024", 1},
	{ "ULAS-42009", 1},
	{ "NPUG-80086", 2},//FlOw
//	{ "ULJS-00430", 2},
};

int data2480 = 0;

void sub_00000368()
{
	int sp = 0;
	int r;

	const char *device = ( sctrlKernelMsIsEf() == 0)?"mscmhc0:":"mscmhcemu0:";

	while(1)
	{
		r = sceIoDevctl( device , 0x02025801 , NULL , 0 , &sp , 4);
		if( r >= 0)
		{

			if(sp == 4)
				break;
		}

		sceKernelDelayThread( 20000 );
	}
}

int  OpenIso()
{
	sceIoClose( umdfd );
	umd_open = 0;

	sub_00000368();

	int fd = sceIoOpen(umdfilename , 0x000F0000 | PSP_O_RDONLY , 511 );
	if(fd<0)
		return -1;

	umdfd = fd;
	umd_is_cso = 0;

	if(CisoOpen(umdfd) >= 0 )
		umd_is_cso=1;

	umd_file_len/*data23D4*/ = GetIsoDiscSize();
	umd_open = 1;

	return 0;
}

//sub_00000CB0:
int umd9660_init()
{
	u8 *buff;
	int i;

	buff = (u8 *)sctrlKernelMalloc(SECTOR_SIZE );

	if( buff== NULL)
			return -1;

	sectorbuf = buff;
	umd_sema = sceKernelCreateSema( "EcsUmd9660DeviceFile" ,0 ,1 ,1 ,NULL);

	if(umd_sema < 0)
	{
		return umd_sema;
	}

	while(umd_open == 0)
	{
		OpenIso(0);
		sceKernelDelayThread(20000);
	}

	memset( io_status , 0 , sizeof(IO_STATUS) * 8);

	umd_read_params.lba = 0x8000;
	umd_read_params.buf = sectorbuf;
	umd_read_params.nsectors = 0x800;

	Umd9660ReadSectors3(&umd_read_params);

	buff = sectorbuf + 883;

	for(i=0;i<(sizeof(disc_list)/sizeof(UMD_ID_LIST));i++)
	{
		if(	memcmp( buff , disc_list[i].id , 10) == 0)
		{
			data2480 = disc_list[i].type;
			break;
		}
	}

	return 0;
}


int umd9660_exit(PspIoDrvArg* arg)
{
	SceUInt sp = 0x7A120;

	sceKernelWaitSema( umd_sema /*data2740*/ , 1, &sp);

	if (sectorbuf)
	{
		sctrlKernelFree(sectorbuf);
		sectorbuf = NULL;
	}

	if (umd_sema >= 0)
	{
		sceKernelDeleteSema(umd_sema);
		umd_sema = -1;
	}

	//sceIoClose(umdfd);

	return 0;
}

//sub_00000A78:
int umd9660_open(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
	int i=0;
	
	for(i=0;i<16;i++)
	{

		if(sceIoLseek32( umdfd ,0,PSP_SEEK_SET) < 0)
		{
			 OpenIso();
		}
		else
		{

			if(sceKernelWaitSema(umd_sema, 1, NULL) < 0)
				return -1;

			for(i=0;i<8;i++)
			{
				if( !(io_status[i].flag))
				{
					arg->arg = (void *)i;
					io_status[i].flag = 1;

					if(sceKernelSignalSema( umd_sema,1) < 0)
						return -1;

					return 0;
				}

			}

		}

	}

	return 0x80010013;
}

//sub_00000740:
int umd9660_read(PspIoDrvFileArg *arg, char *data, int sector)
{
  if(sceKernelWaitSema(umd_sema, 1, 0) < 0)
	  return -1;
  		

  int i = sector;
  int offset = io_status[(int)(arg->arg)].offset;

  if(umd_file_len < offset + sector ) //len >= pos + a2
  {
    i = umd_file_len - offset; //len - pos
  }

  if(sceKernelSignalSema(umd_sema, 1) < 0)
	  return -1;

  int ret = Umd9660ReadSectors( offset << 11 , data , i << 11);//loc_000003E0

  if(ret < 0)
	  return -1;

  if(sceKernelWaitSema(umd_sema, 1, 0) < 0)
	  return -1;
	  
  int ret_sector = ret >> 11;
  io_status[(int)(arg->arg)].offset += ret_sector;

  if(sceKernelSignalSema(umd_sema, 1) < 0)
	  return -1;

  return ret_sector;
}


//sub_00000250:
int umd9660_close(PspIoDrvFileArg *arg)
{
	if(sceKernelWaitSema( umd_sema ,1,NULL) < 0)
		return -1;

	int ret = 0;

	if( io_status[(int)(arg->arg)].flag )
	{
		io_status[(int)(arg->arg)].flag = 0;
	}
	else
	{
		ret = 0x80010016;
	}

	if(sceKernelSignalSema( umd_sema ,1) < 0)
		return -1;

	return ret;
}

//sub_000000D8:
SceOff umd9660_lseek(PspIoDrvFileArg *arg ,SceOff offset,int whence)
{
  if(sceKernelWaitSema(umd_sema, 1, 0)<0)
	  return -1;

  u32 umd_cur_offset = io_status[(int)(arg->arg)].offset;

  if(whence == PSP_SEEK_SET)
  {
    umd_cur_offset  = offset;//dataB0EC
  }
  else if(whence == PSP_SEEK_CUR)
  {
    umd_cur_offset += offset;
  }
  else if(whence == PSP_SEEK_END)
  {
    umd_cur_offset = umd_file_len - offset;//dataA6E0 - offset
  }
  else
  {
    if(sceKernelSignalSema(umd_sema, 1)<0)
		return -1;

    return 0x80010016;
  }


  umd_cur_offset = (umd_cur_offset < umd_file_len) ? umd_cur_offset : umd_file_len;

  io_status[(int)(arg->arg)].offset = umd_cur_offset;

  if(sceKernelSignalSema(umd_sema, 1) < 0)
	  return -1;

  return umd_cur_offset;
}

int loc_00000488( void *outdata , int outlen , void *indata )
{
	u32 *date_buff = (u32 *)indata;

	int offset = date_buff[8/4] << 11;
	int size = date_buff[16/4];

	if( outlen >= size )
	{
		if(date_buff[24/4])
		{
			if(date_buff[20/4])
			{	
				offset += 0x800 - date_buff[24/4];
			}
			else
			{
				if(date_buff[28/4])
				{
					offset += 0x800 - date_buff[24/4];
				}
				else
				{
					offset += date_buff[24/4];
				}
			}
		}

		return Umd9660ReadSectors( offset , outdata , size );//loc_000003E0
	}

	return 0x80010069;
}

//loc_000004F4:
int umd9660_devctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{			
	switch(cmd)
	{
	case 0x01F00003:
		return 0;
		break;
	case 0x01E28035:		
		((u32 *)outdata)[0] = (u32)sectorbuf;//data2484	
		return 0;
		break;
		/*
	case 0x01E18030:
		return 1;
		break;
*/
		//add
/*	case 0x01E18033:
	case 0x01E180C1:
	case 0x01E180D0:
	case 0x01E080D4:
	case 0x01E000D5:
*/
	case 0x01E180D3:
	case 0x01E080A8:
		return 0x80010086;
		break;

	case 0x01E38012:		
		if( outlen < 0)
			outlen += 3;

		memset( outdata , 0 , outlen);
		((u32 *)outdata)[0]= 0xE0000800;
		((int *)outdata)[8/4]= 0;
		((int *)outdata)[28/4]= umd_file_len;//data23D4
		((int *)outdata)[36/4]= umd_file_len;//data23D4		
		return 0;
		break;

	case 0x01E280A9:
		((int *)outdata)[0]= 0x800;	
		return 0;
		break;
	case 0x01E38034:
		if(indata && outdata)
		{			
			((int *)outdata)[0]= 0;		
			return 0;
		}
		return 0x80010016;
		break;
	case 0x01E380C0:
		if(indata && outdata)
		{
			return loc_00000488( outdata , outlen , indata );		
		}
		return 0x80010016;
		break;
	case 0x01F20001:
		((int *)outdata)[0]= -1;	
		((int *)outdata)[1]= 16;
		return 0;
		break;

	case 0x01F010DB:			
		return 0;
		break;

		//add
	case 0x01F100A6:
	case 0x01F100A8:
		if( indata && (inlen >= 4))
			return 0;
		return 0x80010016;

		break;
	case 0x01F100A5:
	case 0x01F100A4:
	case 0x01F100A3:
		return 0;
		break;

	case 0x01F20002:
	case 0x01F20003:
		if( outdata && (outlen >=4) )
		{
			((int *)outdata)[0] = umd_file_len;//data23D4
			return 0;
		}
		return 0x80010016;

		break;		
		//add
	case 0x01F300A5:
		if( indata && (inlen >= 16))
		{
			if( outdata && (outlen >=4) )
				return 0;
		}
		return 0x80010016;
		break;

	case 0x00208811:
	case 0x00208810:
		return 0;
/*
	case :
		break;
		*/
	}

	Kprintf("unknown devctl 0x%08X\n",cmd);
	return 0x80010086;
}

//sub_0000083C:
int umd9660_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{			

	switch( cmd )
	{
	case 0x01F010DB:
		return 0;
		break;

	case 0x01D20001:
		if(sceKernelWaitSema( umd_sema, 1, NULL) < 0)
			return -1;

		((int *)outdata)[0] = io_status[(int)(arg->arg)].offset;

		if(sceKernelSignalSema( umd_sema ,1) < 0)
			return -1;

		return 0;
		break;
	case 0x01F100A6:
		if(indata && (inlen >= 4))
		{
			return umd9660_lseek( arg , ((SceOff *)indata)[0] , ((int *)indata)[3] );//sub_000000D8		
		}
		return 0x80010016;
		break;

	case 0x01F30003:
		if(indata && (inlen >= 4) && outdata)
		{
			int size = *((int *)indata);		
			if(size <= outlen )
			{
				return umd9660_read( arg , outdata, size );//sub_00000740			
			}
		}
		return 0x80010016;		
		break;

	case 0x00208082:

	case 0x00208011:
	case 0x00208010:

	case 0x00208001:
	case 0x0020800C:
	case 0x00208006:
		return 0;
		break;
	}

	Kprintf("Unknown ioctl 0x%08X\n",cmd);
	return 0x80010086;
}


//loc_00000BB4:
int  ReadUmdFileRetry(void *buf, int size, int fpointer)
{
	int i, read;

	for(i = 0; i < 16; i++)
	{
		if(sceIoLseek32(umdfd, fpointer, PSP_SEEK_SET) >= 0)
		{
			for(i = 16; i > 0; i--)
			{
				if((read = sceIoRead(umdfd, buf, size)) >= 0)
				{
					return read;
				}
				OpenIso();
			}
			return 0x80010013;
		}
		OpenIso();
	}
	return 0x80010013;
}

int Umd9660ReadSectors3(UmdReadParams *read_params)
{
	if(umd_is_cso == 0) 
		return ReadUmdFileRetry(read_params->buf,  read_params->nsectors, read_params->lba );
	else
		return CisofileReadSectors(read_params->buf,  read_params->nsectors, read_params->lba );
}


//loc_000003E0
int  Umd9660ReadSectors(int lba, void *buf, int nsectors)//, int *eod
{
	if(sceKernelWaitSema(umd_sema, 1, 0) < 0)
	  return -1;

	umd_read_params.lba = lba;
	umd_read_params.buf = buf;
	umd_read_params.nsectors = nsectors;

	int ret = sceKernelExtendKernelStack(0x2000, (void *)Umd9660ReadSectors3/*loc_00000C7C*/, &umd_read_params );

	if(sceKernelSignalSema(umd_sema, 1) < 0)
	  return -1;

	return ret;
}

//sub_000009B4
int GetIsoDiscSize()
{
	if(umd_is_cso == 0)
		return IsofileGetDiscSize(umdfd);
	else
		return CisofileGetDiscSize(umdfd);
}



PspIoDrvFuncs umd9660_funcs =//
{
	umd9660_init,//cB0
	umd9660_exit,//2e8
	umd9660_open,//a78
	umd9660_close,//250
	umd9660_read,//740
	NULL, /* no write */
	umd9660_lseek,//d8
	umd9660_ioctl,//83c
	NULL, /* no remove */
	NULL, /* no mkdir */
	NULL, /* no rmdir */
	NULL,//umd9660_dopen
	NULL,//umd9660_dclose
	NULL,//umd9660_dread
	NULL,//umd9660_getstat
	NULL, /* no chstat */
	NULL, /* no rename */
	NULL,//umd9660_chdir
	NULL,//umd9660_mount
	NULL,//umd9660_umount
	umd9660_devctl,//4f4
	NULL
};

PspIoDrv umd9660_driver = { "umd" , 0x4, 0x800, "UMD9660"/*2240*/, &umd9660_funcs/*23EC*/ };//data2444

int data2248[4] = { -1 , -1 , -1 , -1};


//sub_00000090
int march_init()
{
	umdfilename = sctrlSEGetUmdFile();

	int r = sceIoAddDrv( &umd9660_driver );
	if(r<0)
		return r;

	sceKernelSetQTGP3(data2248);
	return 0;
}

int march_test()
{
	sceIoDelDrv("umd");
	march_init();
	return 0;
}