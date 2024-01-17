#include "launcher.h"

#include <stdlib.h>
#include <string.h>
#include <kubridge.h>

#include "scepaf.h"
#include "vpl.h"


void exec_recovery_menu(vsh_Menu *vsh) {
	// try recovery app
	char menupath[ARK_PATH_SIZE];
	scePaf_strcpy(menupath, vsh->config.ark.arkpath);
	strcat(menupath, ARK_RECOVERY);

	SceIoStat stat; int res = sceIoGetstat(menupath, &stat);
	
	if (res >= 0){
		struct SceKernelLoadExecVSHParam param;
		scePaf_memset(&param, 0, sizeof(param));
		param.size = sizeof(param);
		param.args = scePaf_strlen(menupath) + 1;
		param.argp = menupath;
		param.key = "game";
		sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
	}
	else {
		// reboot system in recovery mode
		vsh->config.ark.recovery = 1;
		struct KernelCallArg args;
		args.arg1 = &(vsh->config.ark);
		u32 setArkConfig = sctrlHENFindFunction("SystemControl", "SystemCtrlPrivate", 0x6EAFC03D);    
		kuKernelCall((void*)setArkConfig, &args);

		vsh->status.reset_vsh = 1;
	}
}

void exec_random_game(vsh_Menu *vsh) {
	char iso_dir[128];
	char game[256];
	int num_games = 0;
	scePaf_strcpy(iso_dir, "ms0:/ISO/");

	if(vsh->psp_model == PSP_GO) {
		iso_dir[0] = 'e';
		iso_dir[1] = 'f';
	}

	SceIoDirent isos;
	SceUID iso_path;
	pspMsPrivateDirent* pri_dirent = (pspMsPrivateDirent*)vpl_alloc(sizeof(pspMsPrivateDirent));

find_random_game:

	iso_path = sceIoDopen(iso_dir);

	scePaf_memset(&isos, 0, sizeof(isos));
	scePaf_memset(pri_dirent, 0, sizeof(*pri_dirent));
	pri_dirent->size = sizeof(*pri_dirent);
	isos.d_private = (void*)pri_dirent;
	while(sceIoDread(iso_path, &isos) > 0) {
		if(isos.d_name[0] != '.' && scePaf_strcmp(isos.d_name, "VIDEO") != 0) {
			num_games++;
		}
	}

	sceIoDclose(iso_path);

	if (num_games == 0){
		vpl_free(pri_dirent);
		return;
	};

	srand(time(NULL));
	int rand_idx = rand() % num_games;
	num_games = 0;

	iso_path = sceIoDopen(iso_dir);

	scePaf_memset(&isos, 0, sizeof(isos));
	scePaf_memset(pri_dirent, 0, sizeof(*pri_dirent));
	pri_dirent->size = sizeof(*pri_dirent);
	isos.d_private = (void*)pri_dirent;
	while(sceIoDread(iso_path, &isos) > 0) {
		if(isos.d_name[0] != '.' && scePaf_strcmp(isos.d_name, "VIDEO") != 0) {
			if (num_games == rand_idx) break;
			else num_games++;
		}
	}

	sceIoDclose(iso_path);

	if (FIO_SO_ISDIR(isos.d_stat.st_attr)){
		strcat(iso_dir, isos.d_name);
		strcat(iso_dir, "/");
		goto find_random_game;
	}

	scePaf_strcpy(game, iso_dir);
	if (pri_dirent->s_name[0])
		strcat(game, pri_dirent->s_name);
	else
		strcat(game, isos.d_name);

	vpl_free(pri_dirent);

	struct SceKernelLoadExecVSHParam param;
	int apitype;
	char* loadexec_file;
	scePaf_memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	
	//set iso file for reboot
	sctrlSESetUmdFile(game);

	//set iso mode for reboot
	sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
	sctrlSESetBootConfFileIndex(MODE_INFERNO);

	param.key = "umdemu";

	static char pboot_path[256];
	int has_pboot = has_update_file(game, pboot_path);

	if (has_pboot){
		// configure to use dlc/update
		loadexec_file = param.argp = pboot_path;
		param.args = scePaf_strlen(pboot_path) + 1;

		if (vsh->psp_model == PSP_GO && game[0] == 'e' && game[1] == 'f') {
			apitype = 0x126;
		}
		else {
			apitype = 0x124;
		}
	}
	else{
		//reset and configure reboot parameter
		loadexec_file = game;

		if (vsh->psp_model == PSP_GO && game[0] == 'e' && game[1] == 'f') {
			apitype = 0x125;
		}
		else {
			apitype = 0x123;
		}

		if (has_prometheus_module(game)) {
			param.argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.OLD";
		} else {
			param.argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
		}
		param.args = 33;
	}

	sctrlKernelLoadExecVSHWithApitype(apitype, game, &param);

}

void launch_umdvideo_mount(vsh_Menu *vsh) {
	SceIoStat stat;
	char *path;
	int type;

	if(0 == vsh->status.umdvideo_idx) {
		if(sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
			// cancel mount
			sctrlSESetUmdFile("");
			sctrlSESetBootConfFileIndex(MODE_UMD);
			vsh->status.reset_vsh = 1;
		}
		return;
	}

	path = umdvideolist_get(&vsh->umdlist, (size_t)(vsh->status.umdvideo_idx-1));

	if (path == NULL)
		return;

	if (sceIoGetstat(path, &stat) < 0)
		return;

	type = vshDetectDiscType(path);
	#ifdef DEBUG
	printk("%s: detected disc type 0x%02X for %s\n", __func__, type, path);
	#endif

	if (type < 0)
		return;

	sctrlSESetUmdFile(path);
	sctrlSESetBootConfFileIndex(MODE_VSHUMD);
	sctrlSESetDiscType(type);
	vsh->status.reset_vsh = 1;
}
