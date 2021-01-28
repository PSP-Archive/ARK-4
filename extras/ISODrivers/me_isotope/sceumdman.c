#include <pspsdk.h>
#include <pspkernel.h>

#include <stdio.h>
#include <string.h>
#include <systemctrl_me.h>

#include "isof_patch_list.h"

void ClearCaches();

int data2794 = 0;
int data2798 = 0;
int data279C = 0;
int data27A0 = 0;

//500 = sceUmdMan_driver_B54D5BE8
int sceUmdMan_driver_454E1B06()
{
	return 0;
}

//500 = sceUmdMan_driver_B9B02322
int sceUmdMan_driver_E52119E7()
{
	return 0;
}

//reset cache read?
//500 = sceUmdMan_driver_31699C86
int sceUmdMan_driver_7AD43944(int a0)
{
	if(a0 != data279C )
		return 0x80010002;

	data2794 = 0;
	data2798 = 0;
	data279C = 0;
	data27A0 = 0;

	return 0;
}

//500= sceUmdMan_driver_988597A2
int sceUmdMan_driver_42D993AC()
{
	return 0;
}

#define GET_GP(gp) asm volatile ("move %0, $gp\n" : "=r" (gp))
#define SET_GP(gp) asm volatile ("move $gp, %0\n" :: "r" (gp))

//500 = sceUmdMan_driver_63B69CE1
int sceUmdMan_driver_26C75616(int a0,int a1, int a2)
{
	if(data279C)
		return 0x8001000C;

	data279C = a0;
	GET_GP(data2794);
//	data2794 = asm("$gp");
	data27A0 = a2;
	data2798 = a1;

	SceModule2 *mod = sceKernelFindModuleByName("sceIsofs_driver");
	u32 text_addr=mod->text_addr;
/*
#define ISOF_PATCH_ADDR1	0x00004020
#define ISOF_PATCH_ADDR2	0x00004058
#define ISOF_PATCH_ADDR3	0x0000410C
#define ISOF_PATCH_ADDR4	0x000042E8
*/
	//addu	$v0, $zr, $zr 
	_sw(0x00001021, text_addr + isof_patch_list.IsofPatchAddr1 );
	_sw(0x00001021, text_addr + isof_patch_list.IsofPatchAddr2 );
	_sw(0x00001021, text_addr + isof_patch_list.IsofPatchAddr3 );
	_sw(0x00001021, text_addr + isof_patch_list.IsofPatchAddr4 );

	ClearCaches();

	if(data2798)
	{
		int (* func)() = (void*)data2798;

		//$gp = data2794;
		SET_GP( data2794 );

		func( data279C , data27A0 , 1);
	}

	return 0;
}


int sceUmd9660_driver_94ACF219()
{
	return 0;
}

int sceUmd9660_driver_1D89BD8F()
{
	return 0;
}

int sceUmd9660_driver_385336B5()
{
	return 0;
}
void sceUmd9660_driver_5041FC4C(){}
void sceUmd9660_driver_1FE5B02F(){}

int sceNp9660_driver_1D642536()
{
	return 0;
}

int sceNp9660_driver_28DCC33D()
{
	return 0;
}

int sceNp9660_driver_FD85B350(int *a0)
{
	if( a0 )
	{
		*a0 = 0;
	}
	return 0;
}