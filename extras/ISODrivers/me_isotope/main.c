#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysevent.h>
#include <psploadexec_kernel.h>

#include <stdio.h>
#include <string.h>
//#include <systemctrl_se.h>
//#include <systemctrl.h>

PSP_MODULE_INFO("umd_driver", 0x1006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

u32 module_sdk_version =  0x03060010;

/*
#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000
#define SC_OPCODE	0x0000000C
#define JR_RA		0x03e00008

#define NOP	0x00000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a);
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a);
*/
extern SceUID umd_sema;

int sceUmd_Init();
int march_init();
void pspUmdCallback(int a0);

static int SysEventHandler(int ev_id, char* ev_name, void* param, int* result)
{
	if( ev_id == 0x400 )
	{
		if(sceKernelWaitSema(umd_sema, 1, 0) >= 0)
		{	
			pspUmdCallback( 0x9 );
		}
	}
	/*
	else if( ev_id == 0x10009 )
	{
//		wait_ms_flag = 1;
//		pspUmdCallback( 0x32 );
	}
	*/
	else if( ev_id == 0x400000 )
	{
		if(sceKernelSignalSema(umd_sema, 1) >= 0)
		{	
			pspUmdCallback( 0x32 );
		}
	}
	/*
	else
	{
//		printf("ev_id = 0x%08X \n", ev_id);
	}
*/
	return 0;
}

static PspSysEventHandler event_handler =
{
	sizeof(PspSysEventHandler),
	"",
	0x00FFFF00,
	SysEventHandler
};

static int sub_000013E0()
{
	sceKernelRegisterSysEventHandler( &event_handler );
	return 0;
}

/*
#include <pspdisplay.h>

typedef union 
{
	int rgba;
	struct 
	{
		char r;
		char g;
		char b;
		char a;
	} c;
} color_t;
void SetColor(int col)
{
	int i;
	color_t *pixel = (color_t *)0x44000000;
	for(i = 0; i < 512*272; i++) {
		pixel->rgba = col;
		pixel++;
	}
}

void cls(int color)
{
    sceDisplaySetFrameBuf((void *)0x44000000, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, 1);
    SetColor(color);
}

*/

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}


int module_start(SceSize args, void *argp)
{	

//	cls(0x000000ff);

	int r =sub_000013E0();
	if(r<0)
		return r;

	r = march_init();
	if(r<0)
	{
		return r;
	}

	r = sceUmd_Init();//sub_00001514
	if(r< 0)
		return r;
	
//	cls(0x00FF0000);


	return 0;
}

int module_stop(void)
{
	sceKernelUnregisterSysEventHandler( &event_handler );//data4B80
	sceIoDelDrv("umd");
	return 0;
}
