#include "usb.h"
#include "common.h"

bool USB::is_enabled = false;
static SceUID usbdev_id = -1;

void USB::enable(){
    if (is_enabled) return;
    ARKConfig* ark_conf = common::getArkConfig();
    if (IS_PSP(ark_conf)){
        // load/start USBDEV.PRX

        is_enabled = true;
    }
    else if (IS_VITA_ADR(ark_conf)){
        // call sctrlStartUsb

        is_enabled = true;
    }
}

void USB::disable(){
    if (!is_enabled) return;
    ARKConfig* ark_conf = common::getArkConfig();
    if (IS_PSP(ark_conf)){
        // stop/unload USBDEV.PRX

        is_enabled = false;
    }
    else if (IS_VITA_ADR(ark_conf)){
        // call sctrlStopUsb
        
        is_enabled = false;
    }
}