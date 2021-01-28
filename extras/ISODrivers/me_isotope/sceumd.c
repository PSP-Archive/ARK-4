
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>
#include <pspumd.h>
#include <pspthreadman.h>

#include <stdio.h>
#include <string.h>

#include "umd9660_driver.h"

int sceKernelCancelSema(int semaid, int signal, int *result);

/* UMD Info struct 
typedef struct pspUmdInfo
{
	// Set to sizeof(pspUmdInfo) 
	unsigned int size;
	// One or more of ::pspUmdTypes 
	unsigned int type;
} pspUmdInfo;
*/

extern int data2480;

int UmdStatus = 0;
int umdcbid = -1;
int umdErrorStat = 0;
int sceUmdSemaid = 0;

//int event_id;

int sceUmd_Init()
{
	UmdStatus = 0x32 ;//data27B0//( PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT)
	umdErrorStat = 0;
	umdcbid =-1;

//	event_id = sceKernelCreateEventFlag("SceMediaManUser" , 513 , 0 , 0 );
	sceUmdSemaid = sceKernelCreateSema( "MediaManSema" ,0 ,0 ,1 ,NULL);

	return (0 < sceUmdSemaid)? 0: sceUmdSemaid ;
}

void pspUmdCallback(int a0)
{
	if(umdcbid >= 0)
	{
		//SceUID 	cb,int 	arg2	 
		sceKernelNotifyCallback( umdcbid , a0);
	}
}

int sceUmdCheckMedium(void)
{
	return 1;
}

int sceUmdGetDiscInfo(pspUmdInfo *info)
{
	int k1 = pspSdkSetK1(0);
	if(info)
	{
		if( info->size == 8 )
		{
			info->type=16;//PSP_UMD_TYPE_GAME
			pspSdkSetK1(k1);
			return 0;
		}
	}

	pspSdkSetK1(k1);
	return 0x80010016;
}

int sceUmdActivate(int unit, const char *drive)
{
	int k1 = pspSdkSetK1(0);
	int sp;

//	cls(0x0000FF00);

	if(strcmp(drive,"disc0:") != 0)
	{
		pspSdkSetK1(k1);
		return 0x80010016;
	}

	sp=1;

	sceIoAssign( drive ,"umd0:","isofs0:", IOASSIGN_RDONLY ,&sp,4);

	UmdStatus = 0x32 ;//( PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT)

	if(data2480== 1 )
	{
		pspUmdCallback( 0x22 );//(PSP_UMD_READY | PSP_UMD_PRESENT)
	}
	else if(!(UmdStatus & 0x20) )//(PSP_UMD_READY)
	{
		pspUmdCallback( 0x32 );//( PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT)
	}

	pspSdkSetK1(k1);
	return 0;
}

int sceUmdDeactivate(int unit, const char *drive)
{
	int k1 = pspSdkSetK1(0);

	int r =sceIoUnassign(drive);

	if(r>=0)
	{
		pspUmdCallback( 0x12 );//( PSP_UMD_INITED | PSP_UMD_PRESENT)
		UmdStatus = 0x12 ;//( PSP_UMD_INITED | PSP_UMD_PRESENT)
	}

	pspSdkSetK1(k1);
	return r;
}

int loc_00001758(int stat , unsigned int timeout , int flag)
{
	int k1 = pspSdkSetK1(0);
	u32 sp = timeout;//SceUInt
	int ret = 0;

	if( stat & (0x3B))//( PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY )
	{

		if( stat & 0x20)//PSP_UMD_READY
		{
			UmdStatus |= 0x20;
		}
		else	if( !(stat & 0x12 ))//(PSP_UMD_INITED | PSP_UMD_PRESENT)
		{
			if(stat & 0x9)//(PSP_UMD_INITING | PSP_UMD_NOT_PRESENT)		
			{
			
				int (* WaitSema)();

				if(flag)
					WaitSema = (void *)sceKernelWaitSemaCB;
				else
					WaitSema = (void *)sceKernelWaitSema;
	
				if(sp)
					WaitSema( sceUmdSemaid , 1 , &sp);
				else
					WaitSema( sceUmdSemaid , 1 , NULL );

			}
		}

	}
	else
	{
		ret = 0x80010016;
	}
		
	if(flag)
	{
		if( data2480 == 2)
		{
			sceKernelCheckCallback();
		}
	}

	pspSdkSetK1(k1);
	return ret;
}

int sceUmdWaitDriveStat(int stat)
{
	return loc_00001758(stat ,0 ,0);
}

int sceUmdWaitDriveStatWithTimer(int stat, unsigned int timeout)
{
	return loc_00001758( stat, timeout , 0);
}

int sceUmdWaitDriveStatCB(int stat, unsigned int timeout)
{
	return loc_00001758(stat , timeout , 1);
}

int sceUmdCancelWaitDriveStat(void)
{
	int k1 = pspSdkSetK1(0);

//	int r = sceKernelCancelEventFlag( event_id , UmdStatus  , 0);
	int r =sceKernelCancelSema( sceUmdSemaid , -1 /* UmdStatus */ , NULL);

	pspSdkSetK1(k1);
	return r;

}

void sceUmdSetErrorStatus(int stat)
{
	umdErrorStat = stat;
}

void sceUmdSetDriveStatus(int stat)
{
	int intr = sceKernelCpuSuspendIntr();

	if( stat & 1)//PSP_UMD_NOT_PRESENT
	{
		UmdStatus &= 0xFFFFFFC1;//~( PSP_UMD_READY |  PSP_UMD_INITED | PSP_UMD_INITING | PSP_UMD_CHANGED | PSP_UMD_PRESENT)
	}
	else if(stat & 0x3E)//( PSP_UMD_READY |  PSP_UMD_INITED | PSP_UMD_INITING | PSP_UMD_CHANGED | PSP_UMD_PRESENT)
	{
		UmdStatus &= 0xFFFFFFFE;//~(PSP_UMD_NOT_PRESENT)
	}

	if(stat & 8)//PSP_UMD_INITING
	{
		UmdStatus &= 0xFFFFFFCF;
		//~ (PSP_UMD_INITED | PSP_UMD_READY)
		//( PSP_UMD_NOT_PRESENT|PSP_UMD_PRESENT|PSP_UMD_CHANGED|PSP_UMD_INITING)
	}
	else if(stat & 0x30)//(PSP_UMD_READY |  PSP_UMD_INITED)
	{
		UmdStatus &= 0xFFFFFFF7;
		// ~ (PSP_UMD_INITING)
		//(PSP_UMD_READY |  PSP_UMD_INITED | PSP_UMD_NOT_PRESENT| PSP_UMD_PRESENT| PSP_UMD_CHANGED)
	}

	if( !(stat & 0x20) )//
	{
		UmdStatus &= 0xFFFFFFDF;
	}

	if((stat | UmdStatus ) & 0x20)// PSP_UMD_READY 
	{
		UmdStatus |= stat;
		UmdStatus |= 0x10;//PSP_UMD_INITED
	}

	if( (stat|UmdStatus) & 0x10)// PSP_UMD_INITED
	{
		UmdStatus |= 2;// PSP_UMD_PRESENT
		sceUmdSetErrorStatus(0);
	}

//	sceKernelSetEventFlag( event_id , UmdStatus );

	sceKernelCpuResumeIntr(intr);
}

int sceUmdGetDriveStatus(void)
{
	return UmdStatus;
}

int sceUmdGetDriveStat(void)
{
	int k1 = pspSdkSetK1(0);
	int r =UmdStatus;
	pspSdkSetK1(k1);
	return r;

}

int sceUmdGetErrorStatus(void)
{
	return umdErrorStat;
}
int sceUmdGetErrorStat(void)
{
	int k1 = pspSdkSetK1(0);

	int r = umdErrorStat;
	pspSdkSetK1(k1);
	return r;

}
int sceUmdRegisterUMDCallBack(int cbid)
{
	int k1 = pspSdkSetK1(0);
	
	int r =sceKernelGetThreadmanIdType(cbid);

	if(r == 8 )//SCE_KERNEL_TMID_ThreadEventHandler 
	{
		umdcbid = cbid;
		pspUmdCallback(UmdStatus);
	
		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);
	return 0x80010016;

}
int sceUmdUnRegisterUMDCallBack(int cbid)
{
	int k1 = pspSdkSetK1(0);
	uidControlBlock *sp;

	int r =sceKernelGetUIDcontrolBlock( cbid , &sp);
	if(r == 0)
	{
		if(umdcbid == cbid)
		{
			umdcbid = -1;
		}
	}
	else
	{
		r = 0x80010016;
	}

	pspSdkSetK1(k1);
	return r;

}

int sceUmdClearDriveStatus(int a0)
{
	int intr = sceKernelCpuSuspendIntr();

//	sceKernelClearEventFlag( event_id , a0);
	UmdStatus &= a0;

	sceKernelCpuResumeIntr(intr);
	return UmdStatus;
}

//sceUmd_36FF82F3:
int sceUmdReplacePermit(void)
{
	return 0;
}
//sceUmd_30DCD985:
int sceUmdReplaceProhibit(void)
{
	return 0;
}

int sceUmd_659587F7(int a0)
{
	if( a0 == 0)
		return 0;

	u32 version = sceKernelGetCompiledSdkVersion();
	if(!version)
	{
		switch( a0 )
		{
		case 0x80010074:
			return 0x8001006E;
			break;
		case 0x80010070:			
			return 0x80010062;
			break;
		case 0x80010071:			
			return 0x80010067;
			break;
		case 0x8001005B:
//			return 0x80010062;
			return 0x80010024;
			break;
		case 0x80010087:
			return 0x8001007B;
			break;
		case 0x8001B006:
			return 0x8001007C;
			break;
		case 0x80010086:
			return 0x8001B000;	
			break;
		}
	}
	return a0;
}
