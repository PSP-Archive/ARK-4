#include <pspinit.h>
#include <pspkernel.h>
#include <pspsdk.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <psptypes.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <stdio.h>

#define PatchSyscall sctrlHENPatchSyscall

typedef struct
{
	int selectbut;
} UVMConfig;
UVMConfig uvmconf;

#define MENU_MAX 2

const char *menu_items[] =  {
	"MENU_TITLE",
	"MENU_RECOVERY",
	"MENU_LANCHER",
};

PSP_MODULE_INFO("test", 0, 1, 2);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

SceCtrlData pad;
int button = 0;
int(*vshCtrlReadBufferPositive)(SceCtrlData* pad_data, int count);
int main_thread(SceSize args, void *argp);
u32 new_pad, old_pad, now_pad;
extern u8 msx[];
int thread_id = 0;
const char *msg;

STMOD_HANDLER previous = NULL;



int module_start(SceSize args, void* argp)
{
    SceUID mainthid = sceKernelCreateThread("ARK VSH Menu", main_thread, 16, 0x1000, 0, 0);

	thread_id = mainthid;

    if(mainthid >= 0) 
		sceKernelStartThread(mainthid, 0, 0);

    return 0;
}

int module_stop(int argc, char *argv[]) {
	SceUInt time = 100000;
	int ret;
	ret = sceKernelWaitThreadEnd ( thread_id, &time );

	if (ret < 0)
		sceKernelTerminateDeleteThread(thread_id);
	return 0;
}



int main_thread(SceSize args, void* argp) {

	while(1) {

		sceCtrlPeekBufferPositive(&pad, 1);
		while (pad.Buttons & PSP_CTRL_LTRIGGER) {
				//sceCtrlReadBufferPositive(&pad, 1);
				vshmenu();
			}
			//if (!uvmconf.selectbut) vshmenu();
		sceKernelDelayThread(100000);
	}
	return 0;
}


/*int vshCtrlReadBufferPositivePatched(SceCtrlData* pad_data, int count) {
	int res = vshCtrlReadBufferPositive(pad_data, count);

	int k1 = pspSdkSetK1(0);
	for (int i = 0; i < count; i++) {
		if (pad_data[i].Buttons & PSP_CTRL_LTRIGGER) button = 1;
		if(button) pad_data[i].Buttons &= ~0xF0FFFF;
	}

	return 0;
}*/

void readpad() {
	static int n = 0;

	vshCtrlReadBufferPositive(&pad, 1);

	now_pad = pad.Buttons;
	new_pad = now_pad & ~old_pad;
	

	if(old_pad == now_pad) {
		n++;
		if(n >= 15) {
			new_pad = now_pad;
			n = 10;
		}
	}
	else {
		n = 0;
		old_pad = now_pad;
	}
}

int adjust_alpha(u32 col) {
	u8 mul;
	u32 c1, c2;
	u32 alpha = col >> 24;

	if ( alpha == 0 ) return col;
	if ( alpha == 0xFF) return col;

	c1 = col & 0x00FF00FF;
	c2 = col & 0x0000FF00;
	mul = (u8)(255 - alpha);
	c1 = (c2 * mul) >> 8 & 0x00FF00FF;
	c2 = (c2 * mul) >> 8 & 0x0000FF00;

	return ( alpha << 24 ) | c1 | c2;
}


int blitPrint(int sx, int sy, char* msg, u32 fg_col, u32 bg_col) {
	int x, y, p, pwidth, pheight, bufferwidth, pixelformat, unk;
	u32 c1, c2, alpha, col;
	u32* vram32;
	u32 fg = adjust_alpha(fg_col);
	u32 bg = adjust_alpha(bg_col);
	char code;
	u32 offset;
	u8 font;


	sceDisplayGetMode(&unk, &pwidth, &pheight);
	sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, unk);
	vram32 = (u32*)vram32;

	if( bufferwidth == 0 || pixelformat != 3 ) return -1;

	for (x = 0; msg[x] && x < (pwidth / 8); x++) {
		code = (u8)msg[x];

		for (y = 0; y < 8; y++) {
			offset = (sy * 8 + y) * bufferwidth + sx * 8 + x * 8;
			font = msx[code * 8 + y];

			for (p = 0; p < 8; p++) {
				col = (font & 0x80) ? fg : bg;
				alpha = col >> 24;
				if ( alpha == 0 ) vram32[offset] = col;
				else if ( alpha != 0xFF ) {
					c2 = vram32[offset];
					c1 = c2 & 0x00FF00FF;
					c2 = c2 & 0x0000FF00;
					c1 = (c1 * alpha) >> 8 & 0x00FF00FF;
					c2 = (c2 * alpha) >> 8 & 0x0000FF00;
					vram32[offset] = (col & 0xFFFFFF) + c1 + c2;
				}
				font <<= 1;
				offset++;
			}
		}
	}
	return 0;
}

void vshmenu() {

	//char string[128];

	while(1) {
		//readpad();
		blitPrint(9, 13, "Fuck-in-a", 0x00FFFFFF, 0x8000FF00);
		sceKernelDelayThreadCB(1);
	}
}
