#include "usb.h"
#include "common.h"

bool USB::is_enabled = false;
static SceUID usbdev_id = -1;

static void load_start_usbdevice(void)
{
    ARKConfig* ark_config = common::getArkConfig();
	SceUID modid = -1;
	int ret;
	char mod[ARK_PATH_SIZE];
	strcpy(mod, ark_config->arkpath);
	strcat(mod, "USBDEV.PRX");

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
}

static void stop_unload_usbdevice(void)
{
	int ret;

	ret = sceKernelStopModule(usbdev_id, 0, NULL, NULL, NULL);

	ret = sceKernelUnloadModule(usbdev_id);

	if (ret >= 0) {
		usbdev_id = -1;
	}
}

void USB::enable(){
    if (is_enabled) return;
    ARKConfig* ark_conf = common::getArkConfig();
    if (IS_PSP(ark_conf)){
        // load/start USBDEV.PRX
        load_start_usbdevice();
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
        stop_unload_usbdevice();
        is_enabled = false;
    }
    else if (IS_VITA_ADR(ark_conf)){
        // call sctrlStopUsb

        is_enabled = false;
    }
}