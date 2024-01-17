#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include "globals.h"

#define PSP_USBSTOR_EF_DRIVERNAME "USBStorEFlash_Driver"

extern ARKConfig* ark_config;
extern SEConfig se_config;

int usb_is_enabled = 0;

int pspUsbDeviceSetDevice(int, int, int);
int pspUsbDeviceFinishDevice();

static void start_psp_usb(){
    int res;

    static char* mods[] = {
        "flash0:/kd/semawm.prx",
        "flash0:/kd/usbstor.prx",
        "flash0:/kd/usbstormgr.prx",
        "flash0:/kd/usbstorms.prx",
        "flash0:/kd/usbstoreflash.prx",
        "flash0:/kd/usbstorboot.prx",
        "flash0:/kd/usb.prx",
    };

    char usbdev[ARK_PATH_SIZE];
    strcpy(usbdev, ark_config->arkpath);
    strcat(usbdev, USBDEV_PRX);

    int modid = kuKernelLoadModule(usbdev, 0, NULL);
    if (modid < 0) modid = kuKernelLoadModule(USBDEV_PRX_FLASH, 0, NULL);
    sceKernelStartModule(modid, 0, NULL, NULL, NULL);

    for (int i=0; i<6; i++){
        int mid = kuKernelLoadModule(mods[i], 0, NULL);
        sceKernelStartModule(mid, 0, NULL, NULL, NULL);
    }

    if(se_config.usbdevice != 0) {
        int (*pspUsbDeviceSetDevice)(int, int, int) = sctrlHENFindFunction("pspUsbDev_Driver", "pspUsbDevice_driver", 0xD4D90520);
		if (pspUsbDeviceSetDevice){
            struct KernelCallArg args;
            memset(&args, 0, sizeof(args));
            args.arg1 = se_config.usbdevice-1;
            kuKernelCall(pspUsbDeviceSetDevice, &args);
        }
    }

    sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
    sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
    sceUsbStart(PSP_USBSTOR_EF_DRIVERNAME, 0, 0);
	sceUsbActivate(0x1c8);
}

static void stop_psp_usb(){
    sceUsbDeactivate(0x1c8);
    sceUsbStop(PSP_USBSTOR_EF_DRIVERNAME, 0, 0);
	sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
    sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
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

void USB_enable(){
    if (usb_is_enabled) return;
    ARKConfig* ark_conf = ark_config;
    if (IS_PSP(ark_conf)){
        // load/start USBDEV.PRX
        start_psp_usb();
        usb_is_enabled = 1;
    }
    else if (IS_VITA_ADR(ark_conf)){
        // call sctrlStartUsb
        start_adrenaline_usb();
        usb_is_enabled = 1;
    }
}

void USB_disable(){
    if (!usb_is_enabled) return;
    ARKConfig* ark_conf = ark_config;
    if (IS_PSP(ark_conf)){
        // stop/unload USBDEV.PRX
        stop_psp_usb();
        usb_is_enabled = 0;
    }
    else if (IS_VITA_ADR(ark_conf)){
        // call sctrlStopUsb
        stop_adrenaline_usb();
        usb_is_enabled = 0;
    }
}