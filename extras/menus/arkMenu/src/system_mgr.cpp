#include <sstream>
#include <pspkernel.h>
#include <psppower.h>

#include "system_mgr.h"
#include "common.h"
#include "controller.h"


extern "C" u32 sctrlHENGetMinorVersion();

string ark_version = "";
struct tm today;

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
static int screensaver = 0;

// options menu entries position of the entries
static int pEntryIndex;
static int cur_entry = 0;
static int page_start = 0;
static int menu_anim_state = 0;
static int menu_draw_state = 0;

static int MAX_ENTRIES = 0;
static SystemEntry** entries = NULL;

static void changeMenuState(){
    if (optionsDrawState == 1 || optionsDrawState == 3)
        return;
    common::playMenuSound();
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
                page_start--;
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
                page_start++;
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

    int offset = (480-(MAX_ENTRIES*15))/2;
    for (int i=0; i<MAX_ENTRIES; i++){
        if (i==pEntryIndex){
            common::printText(offset + (i+1)*15, 15, "*", LITEGRAY, SIZE_BIG, true);
        }
        else{
            common::printText(offset + (i+1)*15, 15, "*", LITEGRAY, SIZE_LITTLE);
        }
    }    
    
    int x = -130;
    for (int i=page_start-1; i<min(page_start+4, MAX_ENTRIES); i++){
        if (i<0){
            x += 160;
            continue;
        }
        entries[i]->getIcon()->draw(x+menu_anim_state, optionsAnimState+15);
        if (i==pEntryIndex && optionsDrawState==2)
            common::printText(x+25, 130, entries[i]->getName().c_str(), LITEGRAY, SIZE_BIG, 1);
        x += 160;
    }
    switch (menu_draw_state){
        case -1:
            menu_anim_state -= 20;
            if (menu_anim_state <= -160){
                //page_start++;
                menu_draw_state = 0;
            }
            break;
        case 0:
            menu_anim_state = 0;
            break;
        case 1:
            menu_anim_state += 20;
            if (menu_anim_state >= 160){
                //page_start--;
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
            common::printText(5, 13, entries[cur_entry]->getInfo().c_str(), LITEGRAY, SIZE_MEDIUM, 0, 0);
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
        if (!screensaver){
            entries[cur_entry]->draw();
            systemDrawer();
            if (common::getConf()->show_fps){
                ostringstream fps;
                ya2d_calc_fps();
                fps<<ya2d_get_fps();
                common::printText(460, 260, fps.str().c_str());
            }
        }
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
        if (pad.triangle() && !screensaver){
            changeMenuState();
        }
        else if (pad.RT()){
            screensaver ^= 1;
        }
        else if (!screensaver){
            if (system_menu) systemController(&pad);
            else entries[cur_entry]->control(&pad);
        }
        sceKernelDelayThread(0);
    }
    sceKernelExitDeleteThread(0);
    return 0;
}

void SystemMgr::initMenu(SystemEntry** e, int ne){
    draw_sema = sceKernelCreateSema("draw_sema", 0, 1, 1, NULL);
    entries = e;
    MAX_ENTRIES = ne;
    today = common::getDateTime();
    u32 ver = sctrlHENGetMinorVersion();
    u32 major = (ver&0xFF0000)>>16;
    u32 minor = (ver&0xFF00)>>8;
    u32 micro = (ver&0xFF);
    stringstream version;
    #ifdef DEBUG 
    version << "ARK Version " << major << "." << minor;
    if (micro>0) version << "." << micro << " DEBUG";
	else version << " DEBUG";
    ark_version = version.str();
	#else
	version << "ARK Version " << major << "." << minor;
    if (micro>0) version << "." << micro;
    ark_version = version.str();
	#endif
}

void SystemMgr::startMenu(){
    draw_thread = sceKernelCreateThread("draw_thread", &drawThread, 0x10, 0x10000, PSP_THREAD_ATTR_USER, NULL);
    sceKernelStartThread(draw_thread, 0, NULL);
    entries[cur_entry]->resume();
    controlThread(0, NULL);
}

void SystemMgr::stopMenu(){
    running = false;
    sceKernelWaitThreadEnd(draw_thread, NULL);
    sceKernelTerminateDeleteThread(draw_thread);
}

void SystemMgr::endMenu(){
    for (int i=0; i<MAX_ENTRIES; i++) delete entries[i];
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
