#include "usb.h"
#include "common.h"

#include <kubridge.h>
#include <systemctrl.h>
#include <pspusb.h>
#include <pspusbstor.h>

bool USB::is_enabled = false;

extern "C" {
    int pspUsbDeviceSetDevice(int, int, int);
    int pspUsbDeviceFinishDevice();
}

static void load_start_usbdevice(void)
{

    int res;

    static char* mods[] = {
        "flash0:/kd/semawm.prx",
        "flash0:/kd/usbstor.prx",
        "flash0:/kd/usbstormgr.prx",
        "flash0:/kd/usbstorms.prx",
        "flash0:/kd/usbstorboot.prx",
        "flash0:/kd/usb.prx",
    };

    for (int i=0; i<6; i++){
        printf("loading %s\n", mods[i]);
        int mid = kuKernelLoadModule(mods[i], 0, NULL);
        printf("mod id: %p\n", mid);
        sceKernelStartModule(mid, 0, NULL, NULL, NULL);
    }

    res = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
    printf("usbStart bus: %p\n", res);
    res = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
    printf("usbStart store: %p\n", res);
	res = sceUsbActivate(0x1c8);
    printf("usbActivate: %p\n", res);
}

static void stop_unload_usbdevice(void)
{
	sceUsbDeactivate(0x1c8);
	sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
    sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
}

static void start_psp_usb(){
    struct KernelCallArg args;
    void* startUsb = (void*)&load_start_usbdevice;
    kuKernelCall(startUsb, &args);
}

static void stop_psp_usb(){
    struct KernelCallArg args;
    void* stopUsb = (void*)&stop_unload_usbdevice;
    kuKernelCall(stopUsb, &args);
}

static void start_adrenaline_usb(){
    struct KernelCallArg args;
    void* startUsb = (void*)sctrlHENFindFunction("ARKCompatLayer", "AdrenalineCtrl", 0x80C0ED7B);
    kuKernelCall(startUsb, &args);
}

static void stop_adrenaline_usb(){
    struct KernelCallArg args;
    void* stopUsb = (void*)sctrlHENFindFunction("ARKCompatLayer", "AdrenalineCtrl", 0x5FC12767);
    kuKernelCall(stopUsb, &args);
}

void USB::enable(){
    if (is_enabled) return;
    ARKConfig* ark_conf = common::getArkConfig();
    if (IS_PSP(ark_conf)){
        // load/start USBDEV.PRX
        start_psp_usb();
        is_enabled = true;
    }
    else if (IS_VITA_ADR(ark_conf)){
        // call sctrlStartUsb
        start_adrenaline_usb();
        is_enabled = true;
    }
}

void USB::disable(){
    if (!is_enabled) return;
    ARKConfig* ark_conf = common::getArkConfig();
    if (IS_PSP(ark_conf)){
        // stop/unload USBDEV.PRX
        stop_psp_usb();
        is_enabled = false;
    }
    else if (IS_VITA_ADR(ark_conf)){
        // call sctrlStopUsb
        stop_adrenaline_usb();
        is_enabled = false;
    }
}