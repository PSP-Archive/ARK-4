#include "config.h"

#include <pspiofilemgr.h>

#include "common.h"
#include "module2.h"
#include "kubridge.h"
#include "systemctrl.h"

#include "scepaf.h"
#include "battery.h"
#include "color.h"


void config_load(vsh_Menu *vsh) {
	int is_pandora;
	char path[ARK_PATH_SIZE];
	scePaf_strcpy(path, vsh->config.ark.arkpath);
	strcat(path, CONFIG_PATH);

	int fp = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if (fp < 0)
		return;
		
	sceIoRead(fp, &vsh->config.ark_menu, sizeof(vsh->config.ark_menu));
	sceIoClose(fp);

	get_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", &vsh->status.swap_xo);
	is_pandora = battery_check();

	if (IS_VITA_ADR(vsh->config.p_ark) || is_pandora < 0)
		vsh->battery = 2;
	else
		vsh->battery = is_pandora;
	
	if (IS_VITA_ADR(vsh->config.p_ark))
		vsh->config.se.usbdevice_rdonly = 2;
	
	color_check_random(vsh);
}

void config_save(vsh_Menu *vsh, int saveumdregion, int savevshregion){
	int fp;
	char path[ARK_PATH_SIZE];
	scePaf_strcpy(path, vsh->config.ark.arkpath);
	strcat(path, CONFIG_PATH);

	fp = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fp < 0)
		return;

	sceIoWrite(fp, &vsh->config.ark_menu, sizeof(vsh->config.ark_menu));
	sceIoClose(fp);

	if (savevshregion){
		char tmp[128];
		scePaf_sprintf(tmp, "fakeregion_%d", vsh->config.se.vshregion);
		config_recreate_region_setting(vsh, "fakeregion_", tmp);
	}

	if (saveumdregion){
		config_recreate_umd_keys();
		static char* regions[] = {"region_none", "region_jp", "region_us", "region_eu"};
		config_recreate_region_setting(vsh, "region_", regions[vsh->config.se.umdregion]);
	}
}

void config_check(vsh_Menu *vsh) {
	color_check_random(vsh);
	if (scePaf_memcmp(&vsh->config.old_se, &vsh->config.se, sizeof(vsh->config.se)) || scePaf_memcmp(&vsh->config.old_ark_menu, &vsh->config.ark_menu, sizeof(vsh->config.ark_menu))){
		vctrlVSHUpdateConfig(&vsh->config.se);
		config_save(vsh, vsh->config.old_se.umdregion != vsh->config.se.umdregion, vsh->config.old_se.vshregion != vsh->config.se.vshregion);	
	}
}



void config_recreate_region_setting(vsh_Menu *vsh, char* oldtext, char* newtext) {
	char path[ARK_PATH_SIZE];
	scePaf_strcpy(path, vsh->config.ark.arkpath);
	strcat(path, "SETTINGS.TXT");

	// open file and get size
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);

	if (fd < 0) {
		fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd >= 0) {
			sceIoWrite(fd, "\nxmb, ", 6);
			sceIoWrite(fd, newtext, scePaf_strlen(newtext));
			sceIoWrite(fd, ", on\n", 5);
			sceIoClose(fd);
		}
		return;
	}

	size_t size = sceIoLseek32(fd, 0, SEEK_END);
	sceIoLseek32(fd, 0, SEEK_SET);

	// allocate buffer
	int memid = sceKernelAllocPartitionMemory(2, "tmp", PSP_SMEM_High, size + 1, NULL);
	if (memid < 0) 
		return;
	
	char* buf = sceKernelGetBlockHeadAddr(memid);
	scePaf_memset(buf, 0, size + 1);

	// read file and close
	sceIoRead(fd, buf, size);
	sceIoClose(fd);

	// open file for writing
	fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	char* vshreg = strstr(buf, oldtext);
	if (vshreg != NULL) {
		while (vshreg[-1] != ',' && vshreg[-1] != ' ' && vshreg[-1] != '\t'){
			vshreg = strstr(vshreg + 1, oldtext);
			if (vshreg == NULL)
				break;
		}
	}

	if (vshreg) {
		u32 p1_size = (u32)vshreg - (u32)buf;
		char* p2 = strstr(vshreg, ",");
		sceIoWrite(fd, buf, p1_size);
		sceIoWrite(fd, newtext, scePaf_strlen(newtext));
		sceIoWrite(fd, p2, scePaf_strlen(p2));
	} else {
		sceIoWrite(fd, buf, size);
		sceIoWrite(fd, "\nxmb, ", 6);
		sceIoWrite(fd, newtext, scePaf_strlen(newtext));
		sceIoWrite(fd, ", on\n", 5);
	}

	sceIoClose(fd);
	sceKernelFreePartitionMemory(memid);
}

void config_recreate_umd_keys(void) {
	struct KernelCallArg args;
	scePaf_memset(&args, 0, sizeof(args));
	
	void* generate_umd_keys = (void*)sctrlHENFindFunction("ARKCompatLayer", "PSPCompat", 0x2EE76C36);
	kuKernelCall(generate_umd_keys, &args);

	// patch region check if not done already
	void* hookImport = (void*)sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", 0x869F24E9);
	SceModule2 mod; 
	kuKernelFindModuleByName("vsh_module", &mod);
	args.arg1 = (u32)&mod;
	args.arg2 = (u32)"sceVshBridge";
	args.arg3 = 0x5C2983C2;
	args.arg4 = 1;
	kuKernelCall(hookImport, &args);
}
