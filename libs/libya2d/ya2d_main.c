/*
    libya2d
    Copyright (C) 2013  Sergi (xerpi) Granell (xerpi.g.12@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "ya2d_main.h"
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <vram.h>
#include <time.h>
#include <psputils.h>


static unsigned int __attribute__((aligned(16))) _ya2d_gu_list[YA2D_GU_LIST_SIZE];
static int          _ya2d_current_fb    = 0;
static void *       _ya2d_fb[2]         = {NULL, NULL};
static void *       _ya2d_drawfbp       = NULL;
static void *       _ya2d_zfb           = NULL;
static int          _ya2d_inited        = 0;
static clock_t      _ya2d_before_clock  = 0;
static clock_t      _ya2d_after_clock   = 0;
static clock_t      _ya2d_frame_count   = 0;
static float        _ya2d_fps           = 0;
static int          _ya2d_vsync_enabled = 0;

int ya2d_init()
{
    if (_ya2d_inited) {
        return 2;
    }
    
    sceGuInit();
    
    sceGuStart(GU_DIRECT, _ya2d_gu_list);
    
    _ya2d_fb[0] = valloc(BUF_WIDTH * SCR_HEIGHT * 4);
    _ya2d_fb[1] = valloc(BUF_WIDTH * SCR_HEIGHT * 4);
    _ya2d_zfb   = valloc(BUF_WIDTH * SCR_HEIGHT * 2);
    
    _ya2d_drawfbp = _ya2d_fb[0];
    
    sceGuDrawBuffer(GU_PSM_8888, vrelptr(_ya2d_fb[0]), BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, vrelptr(_ya2d_fb[1]), BUF_WIDTH);
    sceGuDepthBuffer(vrelptr(_ya2d_zfb), BUF_WIDTH);
    
    sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuFrontFace(GU_CW);
    sceGuEnable(GU_CULL_FACE);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuEnable(GU_ALPHA_TEST);
    sceGuEnable(GU_BLEND);
    
    sceGuAlphaFunc(GU_GREATER, 0, 255);
    sceGuDepthFunc(GU_LEQUAL);
    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuTexScale(1.0f, 1.0f);
    sceGuTexOffset(0.0f, 0.0f);
    
    sceGuFinish();
    sceGuSync(GU_SYNC_WHAT_DONE, GU_SYNC_FINISH);
    
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
    
    sceGuFinish();
    sceGuSync(GU_SYNC_WHAT_DONE, GU_SYNC_FINISH);
    
    _ya2d_inited = 1;
    return 1;
}


int ya2d_shutdown()
{
    if(!_ya2d_inited) {
        return 2;
    }

    sceGuFinish();
    sceGuSync(GU_SYNC_WHAT_DONE, GU_SYNC_DONE);    
    sceGuTerm();
    
    vfree(_ya2d_fb[0]);
    vfree(_ya2d_fb[1]);
    vfree(_ya2d_zfb);
    
    _ya2d_inited = 0;
    return 1;
}

void ya2d_start_drawing()
{
    sceGuStart(GU_DIRECT, _ya2d_gu_list);
}

void ya2d_finish_drawing()
{
    sceGuFinish();
    sceGuSync(GU_SYNC_WHAT_DONE, GU_SYNC_FINISH);
}

void ya2d_clear_screen(unsigned int color)
{
    sceGuClearColor(color);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
}

void ya2d_swapbuffers()
{
    if (_ya2d_vsync_enabled) {
        sceDisplayWaitVblankStart();
    }
    _ya2d_drawfbp = sceGuSwapBuffers();
    
    _ya2d_current_fb ^= 1;    
}

void ya2d_calc_fps()
{
    _ya2d_after_clock = sceKernelLibcClock();    
    ++_ya2d_frame_count;
    register clock_t delta = _ya2d_after_clock - _ya2d_before_clock;
    if(delta >= uS_PER_SEC) {
        _ya2d_fps = (float)_ya2d_frame_count/(float)(delta/uS_PER_SEC);
        _ya2d_frame_count = 0;
        _ya2d_before_clock = sceKernelLibcClock();
    }  
}

void ya2d_set_vsync(int enabled)
{
    _ya2d_vsync_enabled = enabled;
}

void *ya2d_get_drawbuffer()
{
    return _ya2d_drawfbp;
}

float ya2d_get_fps()
{
    return _ya2d_fps;
}
