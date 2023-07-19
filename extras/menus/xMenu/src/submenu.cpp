/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */




#include "menu.h"
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "../../arkMenu/include/conf.h"

//static vsh_Config _vsh_Config;
//vsh_Config* vsh_config = &_vsh_Config;
//static t_conf _config;
t_conf* config;

static SEConfig _se_conf;
SEConfig* se_config = &_se_conf;

int pwidth;
int pheight, bufferwidth, pixelformat;
unsigned int* vram32;
extern unsigned char msx[];
static unsigned char *g_cur_font = msx;

u32 fcolor;
u32 bcolor;



static char* OPTIONS[] = {
	(char*)"Disabled",
	(char*)"Always",
	(char*)"PS1",
	(char*)"Launcher",
};


struct submenu_elements {
	char** options;
	const char* restart;
	const char* exit;
};




SubMenu::SubMenu() {
	this->getItems();	
}



void SubMenu::getItems() {
	uint16_t memory_stick_speed = se_config->msspeed;
	struct submenu_elements submenu_elem = {
		OPTIONS,
		"Restart",
		"Exit"
	};
}

u32 SubMenu::adjust_alpha(u32 col)
{   
    u32 alpha = col>>24;
    u8 mul;
    u32 c1,c2;
    
    if(alpha==0)    return col;
    if(alpha==0xff) return col;
    
    c1 = col & 0x00ff00ff;
    c2 = col & 0x0000ff00;
    mul = (u8)(255-alpha);
    c1 = ((c1*mul)>>8)&0x00ff00ff;
    c2 = ((c2*mul)>>8)&0x0000ff00;
    return (alpha<<24)|c1|c2;
}

int SubMenu::blit_string(int sx,int sy,const char *msg)
{
    int x,y,p;
    int offset;
    u8 code, font;
    u32 fg_col,bg_col;

    u32 col,c1,c2;
    u32 alpha;

    fg_col = this->adjust_alpha(fcolor);
    bg_col = this->adjust_alpha(bcolor);


    if ((bufferwidth==0) || (pixelformat!=3))
        return -1;

    for (x = 0; msg[x] && x < (pwidth / 8); x++) {
        code = (u8)msg[x]; // no truncate now

        for (y = 0; y < 8; y++) {
            offset = (sy + y) * bufferwidth + sx + x * 8;
            font = y>=7 ? 0x00 : g_cur_font[code * 8 + y];
            for (p = 0; p < 8; p++){
                col = (font & 0x80) ? fg_col : bg_col;
                alpha = col>>24;
                if (alpha == 0)
                    vram32[offset] = col;
                else if (alpha != 0xff) {
                    c2 = vram32[offset];
                    c1 = c2 & 0x00ff00ff;
                    c2 = c2 & 0x0000ff00;
                    c1 = ((c1*alpha)>>8)&0x00ff00ff;
                    c2 = ((c2*alpha)>>8)&0x0000ff00;
                    vram32[offset] = (col&0xffffff) + c1 + c2;
                }

                font <<= 1;
                offset++;
            }
        }
    }
    return sx + x * 8;
}

int SubMenu::blitSetup() {
		int unk;
		sceDisplayGetMode(&unk, &pwidth, &pheight);
		sceDisplayGetFrameBuf(reinterpret_cast<void**>(&vram32), &bufferwidth, &pixelformat, PSP_DISPLAY_SETBUF_NEXTFRAME);
		if( (bufferwidth==0) || (pixelformat!=3))
			return -1;
		fcolor = 0x00ffffff;
		bcolor = 0x000000ff;
		//fcolor = config->vsh_fg_color;
		//bcolor = config->vsh_bg_color;
		//bcolor = ark_config.vsh_bg_color;

		return 0;
}

void SubMenu::run() {
	this->blitSetup();	
	//static int submenu_start_x = (pwidth - window_pixel) / 2;
	//void blit_rect_fill(int sx, int sy, int w, int h);
	this->blit_rect_fill(80, 80, 4, 8);
	this->blit_string(200, 100, "TEST");
	sceKernelDelayThread(5000000);
}

void SubMenu::blit_rect_fill(int sx, int sy, int w, int h){
    int x, y;
    u32 col, c1, c2;
    u32 bg_col;
    u32 offset, alpha;
    bg_col = this->adjust_alpha(bcolor);

    for (y = 0; y < h; y++){
        for (x = 0; x < w; x++){
            col = bg_col;
            alpha = col >> 24;
            offset = (sy + y) * bufferwidth + (sx + x);
            if(alpha == 0)
                vram32[offset] = col;
            else if (alpha != 0xff) {
                c2 = vram32[offset];
                c1 = c2 & 0x00ff00ff;
                c2 = c2 & 0x0000ff00;
                c1 = ((c1 * alpha) >> 8) & 0x00ff00ff;
                c2 = ((c2 * alpha) >> 8) & 0x0000ff00;
                vram32[offset] = (col & 0xffffff) + c1 + c2;
            }
        }
    }
}


SubMenu::~SubMenu() {}
