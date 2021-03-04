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

/*
    PSP VSH MENU controll
    based Booster's vshex
*/

#include "common.h"
#include <psputility.h>

const char **g_messages = g_messages_en;

extern int pwidth;

#define TMENU_MAX 7

enum{
    TMENU_RECOVERY_MENU,
    TMENU_LAUNCHER,
    TMENU_SHUTDOWN_DEVICE,
    TMENU_SUSPEND_DEVICE,
    TMENU_RESET_DEVICE,
    TMENU_RESET_VSH,
    TMENU_EXIT
};

int item_fcolor[TMENU_MAX];
const char *item_str[TMENU_MAX];

static int menu_sel = TMENU_RECOVERY_MENU;

const int xyPoint[] ={0x98, 0x30, 0xC0, 0xA0, 0x70, 0x08, 0x0E, 0xA8};//data243C=
const int xyPoint2[] ={0xB0, 0x30, 0xD8, 0xB8, 0x88, 0x08, 0x11, 0xC0};//data2458=

int menu_draw(void)
{
    u32 fc,bc;
    const char *msg;
    int max_menu, cur_menu;
    const int *pointer;
    int xPointer;
    
    // check & setup video mode
    if( blit_setup() < 0) return -1;

    if(pwidth==720) {
        pointer = xyPoint;
    } else {
        pointer = xyPoint2;
    }

    // show menu list
    blit_set_color(0xffffff,0x8000ff00);
    blit_string(pointer[0], pointer[1], g_messages[MSG_ARK_VSH_MENU]);

    for(max_menu=0;max_menu<TMENU_MAX;max_menu++) {
        fc = 0xffffff;
        bc = (max_menu==menu_sel) ? 0xff8080 : 0xc00000ff;
        blit_set_color(fc,bc);

        msg = g_messages[MSG_RECOVERY_MENU + max_menu];

        if(msg) {
            switch(max_menu) {
                case TMENU_EXIT:
                    xPointer = 0xD8; //pointer[2];
                    break;
                case TMENU_RESET_DEVICE:
                    xPointer = 0xC0; //pointer[3];
                    break;
                case TMENU_RESET_VSH:
                    xPointer = 0xC0; //pointer[7];
                    break;
                case TMENU_RECOVERY_MENU:
                    xPointer = 168;
                    break;
                case TMENU_LAUNCHER:
                    xPointer = 168;
                    break;
                case TMENU_SHUTDOWN_DEVICE:
                    xPointer = 176;
                    break;
                case TMENU_SUSPEND_DEVICE:
                    xPointer = 176;
                    break;
                default:
                    xPointer=pointer[4];
                    break;
            }

            cur_menu = max_menu;
            blit_string(xPointer, (pointer[5] + cur_menu)*8, msg);
            msg = item_str[max_menu];

            if(msg) {
                blit_set_color(item_fcolor[max_menu],bc);
                blit_string( (pointer[6] * 8) + 128, (pointer[5] + cur_menu)*8, msg);
            }
        }
    }

    blit_set_color(0x00ffffff,0x00000000);

    return 0;
}

int menu_setup(void)
{
    int i;
    const char *bridge;
    const char *umdvideo_disp;

    // preset
    for(i=0;i<TMENU_MAX;i++) {
        item_str[i] = NULL;
        item_fcolor[i] = RGB(255,255,255);
    }
    return 0;
}

int menu_ctrl(u32 button_on)
{
    int direction;

    if( (button_on & PSP_CTRL_SELECT) ||
        (button_on & PSP_CTRL_HOME)) {
        menu_sel = TMENU_EXIT;
        return 1;
    }

    // change menu select
    direction = 0;

    if(button_on & PSP_CTRL_DOWN) direction++;
    if(button_on & PSP_CTRL_UP) direction--;

    menu_sel = limit(menu_sel+direction, 0, TMENU_MAX-1);

    // LEFT & RIGHT
    direction = -2;

    if(button_on & PSP_CTRL_LEFT)   direction = -1;
    if(button_on & PSP_CTRL_CROSS) direction = 0;
    if(button_on & PSP_CTRL_CIRCLE) direction = 0;
    if(button_on & PSP_CTRL_RIGHT)  direction = 1;

    if(direction <= -2)
        return 0;

    switch(menu_sel) {
        case TMENU_RECOVERY_MENU:
            if(direction==0) {
                return 6; // Recovery menu flag
            }
            break;
        case TMENU_LAUNCHER:
            if(direction==0) {
                return 7; // Recovery menu flag
            }
            break;
        case TMENU_SHUTDOWN_DEVICE:            
            if(direction==0) {
                return 3; // SHUTDOWN flag
            }
            break;
        case TMENU_RESET_DEVICE:    
            if(direction==0) {
                return 2; // RESET flag
            }
            break;
        case TMENU_RESET_VSH:    
            if(direction==0) {
                return 4; // RESET VSH flag
            }
            break;
        case TMENU_SUSPEND_DEVICE:    
            if(direction==0) {
                return 5; // SUSPEND flag
            }
            break;
        case TMENU_EXIT:
            if(direction==0) return 1; // finish
            break;
    }

    return 0; // continue
}
