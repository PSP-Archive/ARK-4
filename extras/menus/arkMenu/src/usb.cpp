#include "usb.h"
#include "common.h"

#include <kubridge.h>
#include <systemctrl.h>

bool USB::is_enabled = false;
static SceUID usbdev_id = -1;

extern "C" {
    int pspUsbDeviceSetDevice(int, int, int);
    int pspUsbDeviceFinishDevice();
}

static void load_start_usbdevice(void)
{
    ARKConfig* ark_config = common::getArkConfig();
	SceUID modid = -1;
	int ret;
	char mod[ARK_PATH_SIZE];
	strcpy(mod, ark_config->arkpath);
	strcat(mod, "USBDEV.PRX");

    int usbid = sceKernelLoadModule("flash0:/kd/usb.prx", 0, NULL);
    sceKernelStartModule(usbid, 0, NULL, NULL, NULL);

	modid = sceKernelLoadModule(mod, 0, NULL);

	if (modid < 0) modid = sceKernelLoadModule("flash0:/vsh/module/ark_usbdev.prx", 0, NULL); // retry flash0

	if (modid < 0) {
		return;
	}

	ret = sceKernelStartModule(modid, 0, NULL, NULL, NULL);

	if (ret < 0) {
		sceKernelUnloadModule(modid);

		return;
	}

	usbdev_id = modid;

    pspUsbDeviceSetDevice(0, 0, 0);
}

static void stop_unload_usbdevice(void)
{
	int ret;

    pspUsbDeviceFinishDevice();

	ret = sceKernelStopModule(usbdev_id, 0, NULL, NULL, NULL);

	ret = sceKernelUnloadModule(usbdev_id);

	if (ret >= 0) {
		usbdev_id = -1;
	}
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