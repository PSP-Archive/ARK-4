#include <pspkernel.h>
#include <psppower.h>

#include "system_mgr.h"
#include "common.h"
#include "gamemgr.h"
#include "browser.h"
#include "ftp_mgr.h"
#include "vshmenu.h"
#include "controller.h"

#define MAX_ENTRIES 4

static SystemEntry* entries[MAX_ENTRIES];
static SceUID draw_thread = -1;
static SceUID draw_sema = -1;
static bool running = true;
static bool system_menu = false;

/* State of the options Menu drawing function and animation state
    optionsDrawState has 4 possible values
    0: draw closed menu
    1: draw popup animation
    2: draw menu
    3: draw popout animation
*/
static int optionsDrawState = 0;
static int optionsAnimState; // state of the animation
static int optionsTextAnim; // -1 for no animation, other for animation

// options menu entries position of the entries
static int pEntryIndex;
static int cur_entry = 0;
static int page_start = 0;
static int menu_anim_state = 0;
static int menu_draw_state = 0;

static void changeMenuState(){
    if (optionsDrawState == 1 || optionsDrawState == 3)
        return;
    if (system_menu){
        optionsAnimState = 0;
        optionsDrawState = 3;
        system_menu = false;
    }
    else{
        optionsTextAnim = -1;
        optionsAnimState = -120;
        optionsDrawState = 1;
        system_menu = true;
    }
    
    common::playMenuSound();
}

static void systemController(Controller* pad){
    if (optionsDrawState != 2)
        return;

    if (pad->accept()){
        entries[cur_entry]->pause();
        changeMenuState();
        cur_entry = pEntryIndex;
        entries[cur_entry]->resume();
    }
    else if (pad->decline()){
        pEntryIndex = cur_entry;
        changeMenuState();
    }
    else if (pad->left()){
        if (pEntryIndex == 0)
            return;
        else if (pEntryIndex == page_start){
            pEntryIndex--;
            if (page_start>0){
                //page_start--;
                menu_draw_state = 1;
            }
        }
        else
            pEntryIndex--;
        common::playMenuSound();
    }
    else if (pad->right()){
        if (pEntryIndex == (MAX_ENTRIES-1))
            return;
        else if (pEntryIndex-page_start == 2){
            if (pEntryIndex+1 < MAX_ENTRIES)
                pEntryIndex++;
            if (page_start+3 < MAX_ENTRIES){
                //page_start++;
                menu_draw_state = -1;
            }
        }
        else if (pEntryIndex+1 < MAX_ENTRIES)
            pEntryIndex++;
        common::playMenuSound();
    }
}

static void drawOptionsMenuCommon(){
    common::getImage(IMAGE_DIALOG)->draw_scale(0, optionsAnimState, 480, 140);
    /*
    common::getImage(IMAGE_GAME)->draw(30, optionsAnimState+10);
    common::getImage(IMAGE_FTP)->draw(190, optionsAnimState+10);
    common::getImage(IMAGE_BROWSER)->draw(350, optionsAnimState+10);
    */
    
    int x = -130;
    for (int i=page_start-1; i<min(page_start+4, MAX_ENTRIES); i++){
        if (i<0){
            x += 160;
            continue;
        }
        entries[i]->getIcon()->draw(x+menu_anim_state, optionsAnimState+10);
        if (i==pEntryIndex && optionsDrawState==2)
            common::printText(x+25, 125, entries[i]->getName(), LITEGRAY, SIZE_BIG, true);
        x += 160;
    }
    switch (menu_draw_state){
        case -1:
            menu_anim_state -= 20;
            if (menu_anim_state <= -160){
                page_start++;
                menu_draw_state = 0;
            }
            break;
        case 0:
            menu_anim_state = 0;
            break;
        case 1:
            menu_anim_state += 20;
            if (menu_anim_state >= 160){
                page_start--;
                menu_draw_state = 0;
            }
            break;
    }
}

static void drawBattery(){

    if (scePowerIsBatteryExist()) {
        int percent = scePowerGetBatteryLifePercent();
        
        if (percent < 0)
            return;

        u32 color;

        if (percent == 100)
            color = GREEN;
        else if (percent >= 17)
            color = LITEGRAY;
        else
            color = RED;

        ya2d_draw_rect(455, 6, 20, 8, color, 0);
        ya2d_draw_rect(454, 8, 1, 5, color, 1);
        ya2d_draw_pixel(475, 14, color);
        
        if (percent >= 5){
            int width = percent*17/100;
            ya2d_draw_rect(457+(17-width), 8, width, 5, color, 1);
        }
    }
}

static void systemDrawer(){

    switch (optionsDrawState){
        case 0: // draw border and battery
            common::getImage(IMAGE_DIALOG)->draw_scale(0, 0, 480, 20);
            drawBattery();
            common::printText(0, 13, entries[cur_entry]->getInfo().c_str(), LITEGRAY, SIZE_BIG, true);
            break;
        case 1: // draw opening animation
            drawOptionsMenuCommon();
            optionsAnimState += 20;
            if (optionsAnimState > 0)
                optionsDrawState = 2;
            break;
        case 2: // draw menu
            optionsAnimState = 0;
            drawOptionsMenuCommon();
            break;
        case 3: // draw closing animation
            drawOptionsMenuCommon();
            optionsAnimState -= 20;
            if (optionsAnimState < -120)
                optionsDrawState = 0;
            break;
    }
}

static int drawThread(SceSize _args, void *_argp){
    while (running){
        sceKernelWaitSema(draw_sema, 1, NULL);
        common::clearScreen(CLEAR_COLOR);
        common::drawScreen();
        entries[cur_entry]->draw();
        systemDrawer();
        common::flipScreen();
        sceKernelSignalSema(draw_sema, 1);
        sceKernelDelayThread(0);
    }
    sceKernelExitDeleteThread(0);
    return 0;
}

static int controlThread(SceSize _args, void *_argp){
    Controller pad;
    while (running){
        pad.update();
        if (pad.triangle()){
            changeMenuState();
        }
        else{
            if (system_menu) systemController(&pad);
            else entries[cur_entry]->control(&pad);
        }
    }
    sceKernelExitDeleteThread(0);
    return 0;
}

void SystemMgr::initMenu(){
    draw_sema = sceKernelCreateSema("draw_sema", 0, 1, 1, NULL);
    entries[0] = new GameManager();
    entries[1] = new FTPManager();
    entries[2] = new Browser();
    entries[3] = new VSHMenu();
}

void SystemMgr::startMenu(){
    draw_thread = sceKernelCreateThread("draw_thread", &drawThread, 0x10, 0x8000, PSP_THREAD_ATTR_USER, NULL);
    sceKernelStartThread(draw_thread, 0, NULL);
    controlThread(0, NULL);
}

void SystemMgr::endMenu(){
    for (int i=0; i<MAX_ENTRIES; i++) delete entries[i];
    running = false;
}

void SystemMgr::pauseDraw(){
    sceKernelWaitSema(draw_sema, 1, NULL);
}

void SystemMgr::resumeDraw(){
    sceKernelSignalSema(draw_sema, 1);
    sceKernelDelayThread(0);
}

SystemEntry* SystemMgr::getSystemEntry(unsigned index){
    return (index < MAX_ENTRIES)? entries[index] : NULL;
}
