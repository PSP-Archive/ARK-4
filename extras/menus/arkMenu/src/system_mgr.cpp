#include <sstream>
#include <pspkernel.h>
#include <psppower.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <psprtc.h>

#include "system_mgr.h"
#include "common.h"
#include "controller.h"
#include "music_player.h"

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
static int fullscreen = 0;

// options menu entries position of the entries
static int pEntryIndex;
static int cur_entry = 0;
static int page_start = 0;
static int menu_anim_state = 0;
static int menu_draw_state = 0;

static int MAX_ENTRIES = 0;
static SystemEntry** entries = NULL;

static bool stillLoading(){
    for (int i=0; i<MAX_ENTRIES; i++){
        if (entries[i]->isStillLoading())
            return true;
    }
    return false;
}

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
            if (page_start+3 < MAX_ENTRIES && (common::getConf()->menusize == 0 || common::getConf()->menusize == 2 || common::getConf()->menusize == 3)){
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
	int loop_setup = 0;
	if(common::getConf()->menusize == 0 || common::getConf()->menusize == 3) {
    	common::getImage(IMAGE_DIALOG)->draw_scale(0, optionsAnimState, 480, 140); // LARGE
		loop_setup = min(page_start+4, MAX_ENTRIES);
	}
	else if(common::getConf()->menusize == 2) {
    	common::getImage(IMAGE_DIALOG)->draw_scale(0, optionsAnimState, 480, 100); // LARGE
		loop_setup = min(page_start+4, MAX_ENTRIES);
	}
	else {
    	common::getImage(IMAGE_DIALOG)->draw_scale(0, optionsAnimState, 480, 80); // SMALL
		loop_setup = MAX_ENTRIES;
	}

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
    for (int i=page_start-1; i<loop_setup; i++){ // SMALL
        if (i<0){
            x += 160;
            continue;
        }
		if(common::getConf()->menusize == 0 || common::getConf()->menusize == 3) {
        	entries[i]->getIcon()->draw(x+menu_anim_state, optionsAnimState+15); // LARGE
		}
		else if(common::getConf()->menusize == 2) {
        	entries[i]->getIcon()->draw_scale(x+menu_anim_state, optionsAnimState+15, 72, 72); // MEDIUM
		}
		else {
			entries[i]->getIcon()->draw_scale(x+menu_anim_state, optionsAnimState+7, 52, 52); // SMALL
		} 
        if (i==pEntryIndex && optionsDrawState==2)
			if(common::getConf()->menusize == 0 || common::getConf()->menusize == 3) {
            	common::printText(x+25, 130, entries[i]->getName().c_str(), LITEGRAY, SIZE_BIG, 1); // LARGE
			}
			else if(common::getConf()->menusize == 2) {
            	common::printText(x+16, 95, entries[i]->getName().c_str(), LITEGRAY, SIZE_MEDIUM, 1); // MEDIUM
			}
			else {
				common::printText(x+12, 75, entries[i]->getName().c_str(), LITEGRAY, SIZE_LITTLE, 1); // SMALL
			}

			if(common::getConf()->menusize == 0 || common::getConf()->menusize == 3)
        		x += 160;
			else if(common::getConf()->menusize == 2)
        		x += 120;
			else
        		x += 100;
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

static void dateTime() {
	pspTime date;
    sceRtcGetCurrentClockLocalTime(&date);

	char dateStr[100];
	sprintf(dateStr, "%04d/%02d/%02d %02d:%02d:%02d", date.year, date.month, date.day, date.hour, date.minutes, date.seconds);
    common::printText( common::getConf()->battery_percent ? 270:300, 13, dateStr, LITEGRAY, SIZE_MEDIUM, 0, 0);
}

static void drawBattery(){

    if (scePowerIsBatteryExist()) {
        int percent = scePowerGetBatteryLifePercent();
        
        if (percent < 0)
            return;

        u32 color;

        if (scePowerIsBatteryCharging()){
            color = BLUE;
        }
        else{
            if (percent == 100)
                color = GREEN;
            else if (percent >= 17)
                color = LITEGRAY;
            else
                color = RED;
        }

        if (common::getConf()->battery_percent) {
            char batteryPercent[4];
            sprintf(batteryPercent, "%d%%", percent);
            common::printText(415, 13, batteryPercent, color, SIZE_MEDIUM);
        }

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
        case 0:
            // draw border, battery and datetime
            common::getImage(IMAGE_DIALOG)->draw_scale(0, 0, 480, 20);
            drawBattery();
			dateTime();
            // draw entry text
            entries[cur_entry]->drawInfo();
            // draw music icon is music player is open
            if (MusicPlayer::isPlaying()){
                common::getIcon(FILE_MUSIC)->draw( common::getConf()->battery_percent ? 250:280, 3);
            }
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
        if (stillLoading()){
            common::getImage(IMAGE_BG)->draw(0, 0);
        }
        else{
            common::drawScreen();
        }
        if (!screensaver){
            entries[cur_entry]->draw();
            if (!fullscreen){
                systemDrawer();
                if (common::getConf()->show_fps){
                    ostringstream fps;
                    ya2d_calc_fps();
                    fps<<ya2d_get_fps();
                    common::printText(460, 260, fps.str().c_str());
                }
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
    static int screensaver_times[] = {0, 5, 10, 20, 30, 60};
    Controller pad;
    clock_t last_pressed = clock();
    while (running){
    	int screensaver_time = screensaver_times[common::getConf()->screensaver];
        pad.update();
        if (pad.triangle() && !screensaver){
            changeMenuState();
        }
        else if (pad.home() && screensaver_time != 0){
            screensaver = !screensaver;
            pad.flush();
            continue;
        }
        else if (!screensaver){
            if (system_menu) systemController(&pad);
            else entries[cur_entry]->control(&pad);
        }
        if (screensaver_time > 0 && !stillLoading()){
            if (pad.any()){
                last_pressed = clock();
                if (screensaver){
                    screensaver = 0;
                    continue;
                }
            }
            clock_t elapsed = clock() - last_pressed;
            double time_taken = ((double)elapsed)/CLOCKS_PER_SEC;
            if (time_taken > screensaver_time){
                screensaver = 1;
            }
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

    // get ARK version    
    u32 ver = sctrlHENGetMinorVersion();
    u32 major = (ver&0xFF0000)>>16;
    u32 minor = (ver&0xFF00)>>8;
    u32 micro = (ver&0xFF);

    // get OFW version (bypass patches)
    struct KernelCallArg args;
    u32 getDevkitVersion = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0x3FC9AE6A);    
    kuKernelCall((void*)getDevkitVersion, &args);
    u32 fw = args.ret1;
    u32 fwmajor = fw>>24;
    u32 fwminor = (fw>>16)&0xF;
    u32 fwmicro = (fw>>8)&0xF;

    stringstream version;
    version << "FW " << fwmajor << "." << fwminor << fwmicro;
	version << " ARK " << major << "." << minor;
    if (micro>9) version << "." << micro;
    else if (micro>0) version << ".0" << micro;
    version << " " << common::getArkConfig()->exploit_id;
    #ifdef DEBUG
	version << " DEBUG";
	#endif
    ark_version = version.str();
}

void SystemMgr::startMenu(){
    draw_thread = sceKernelCreateThread("draw_thread", &drawThread, 0x10, 0x10000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
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

void SystemMgr::enterFullScreen(){
    fullscreen = 1;
}

void SystemMgr::exitFullScreen(){
    fullscreen = 0;
}

SystemEntry* SystemMgr::getSystemEntry(unsigned index){
    return (index < MAX_ENTRIES)? entries[index] : NULL;
}
