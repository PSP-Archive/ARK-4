/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

/*
 * vshMenu by neur0n
 * based booster's vshex
 */
#include <pspkernel.h>
#include <psputility.h>
#include <stdio.h>
#include <pspumd.h>

#include "common.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "kubridge.h"
#include "vpl.h"
#include "blit.h"
#include "trans.h"

#include "../arkMenu/include/conf.h"

int TSRThread(SceSize args, void *argp);

/* Define the module info section */
PSP_MODULE_INFO("VshCtrlSatelite", 0, 2, 2);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

extern int scePowerRequestColdReset(int unk);
extern int scePowerRequestStandby(void);
extern int scePowerRequestSuspend(void);

extern char umdvideo_path[256];

int menu_mode  = 0;
int submenu_mode  = 0;
u32 cur_buttons = 0xFFFFFFFF;
u32 button_on  = 0;
int stop_flag=0;
int sub_stop_flag=0;
SceCtrlData ctrl_pad;
int stop_stock=0;
int sub_stop_stock=0;
int thread_id=0;

t_conf config;

SEConfig cnf;
static SEConfig cnf_old;

u32 psp_fw_version;
u32 psp_model;

UmdVideoList g_umdlist;

ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;
extern int is_pandora;

int module_start(int argc, char *argv[])
{
	SceUID thid;
/*	SceKernelThreadInfo info;
	info.size = sizeof(info);
	SceUID def_thread = sceKernelReferThreadStatus("VshMenu_Thread", &info);
	SceUID mod = sceUtilityLoadModule("ms0:/PSP/SAVEDATA/ARK_01234/VSHMENU.PRX");
	sceKernelStopModule(mod, 0, NULL, NULL, NULL);
	sceKernelUnloadModule(mod);
*/


	sctrlHENGetArkConfig(ark_config);
	psp_model = kuKernelGetModel();
	psp_fw_version = sceKernelDevkitVersion();
	vpl_init();
	thid = sceKernelCreateThread("AVshMenu_Thread", TSRThread, 16 , 0x1000 ,0 ,0);

	thread_id=thid;

	if (thid>=0) {
		sceKernelStartThread(thid, 0, 0);
	}
	
	return 0;
}

int module_stop(int argc, char *argv[])
{
	SceUInt time = 100*1000;
	int ret;

	stop_flag = 1;
	ret = sceKernelWaitThreadEnd( thread_id , &time );

	if(ret < 0) {
		sceKernelTerminateDeleteThread(thread_id);
	}

	return 0;
}

int EatKey(SceCtrlData *pad_data, int count)
{
	u32 buttons;
	int i;

	// copy true value

#ifdef CONFIG_639
	if(psp_fw_version == FW_639)
		scePaf_memcpy(&ctrl_pad, pad_data, sizeof(SceCtrlData));
#endif

#ifdef CONFIG_635
	if(psp_fw_version == FW_635)
		scePaf_memcpy(&ctrl_pad, pad_data, sizeof(SceCtrlData));
#endif

#ifdef CONFIG_620
	if (psp_fw_version == FW_620)
		scePaf_memcpy_620(&ctrl_pad, pad_data, sizeof(SceCtrlData));
#endif

#if defined(CONFIG_660) || defined(CONFIG_661)
	if ((psp_fw_version == FW_660) || (psp_fw_version == FW_661))
		scePaf_memcpy_660(&ctrl_pad, pad_data, sizeof(SceCtrlData));
#endif

	// buttons check
	buttons     = ctrl_pad.Buttons;
	button_on   = ~cur_buttons & buttons;
	cur_buttons = buttons;

	// mask buttons for LOCK VSH controll
	for(i=0;i < count;i++) {
		pad_data[i].Buttons &= ~(
				PSP_CTRL_SELECT|PSP_CTRL_START|
				PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|
				PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|
				PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|
				PSP_CTRL_HOME|PSP_CTRL_NOTE);

	}

	return 0;
}

static void button_func(void)
{
	int res;

	// menu controll
	switch(menu_mode) {
		case 0:	
			if( (cur_buttons & ALL_CTRL) == 0) {
				menu_mode = 1;
			}
			break;
		case 1:
			res = menu_ctrl(button_on);

			if(res != 0) {
				stop_stock = res;
				menu_mode = 2;
			}
			break;
		case 2: // exit waiting 
			// exit menu
			if((cur_buttons & ALL_CTRL) == 0) {
				stop_flag = stop_stock;
			}
			break;
	}
}

static void subbutton_func(void)
{
	int res;

	// menu controll
	switch(submenu_mode) {
		case 0:	
			if( (cur_buttons & ALL_CTRL) == 0) {
				submenu_mode = 1;
			}
			break;
		case 1:
			res = submenu_ctrl(button_on);

			if(res != 0) {
				sub_stop_stock = res;
				submenu_mode = 2;
			}
			break;
		case 2: // exit waiting 
			// exit menu
			if((cur_buttons & ALL_CTRL) == 0) {
				sub_stop_flag = sub_stop_stock;
			}
			break;
	}
}

int load_start_module(char *path)
{
	int ret;
	SceUID modid;

	modid = sceKernelLoadModule(path, 0, NULL);

	if(modid < 0) {
		return modid;
	}

	ret = sceKernelStartModule(modid, strlen(path) + 1, path, NULL, NULL);

	return ret;
}


void exec_recovery_menu(){
    char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config->arkpath);
    strcat(menupath, ARK_RECOVERY);
    
    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(menupath) + 1;
    param.argp = menupath;
    param.key = "game";
    sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
}

void import_classic_plugins() {
	SceUID game, vsh, pops, plugins;
	int i = 0;
	int chunksize = 256;
	int bytesRead;
	char * buf = (char *)malloc(chunksize);
	char *gameChar = "game, ";
	int gameCharLength = strlen(gameChar);
	char *vshChar = "vsh, ";
	int vshCharLength = strlen(vshChar);
	char *popsChar = "pops, ";
	int popsCharLength = strlen(popsChar);
	char filename[27];

	if (psp_model == PSP_GO)
		snprintf(filename, sizeof(filename), "%s0:/seplugins/plugins.txt", "ef");
	else
		snprintf(filename, sizeof(filename), "%s0:/seplugins/plugins.txt", "ms");
	
	if (psp_model == PSP_GO) {
		game = sceIoOpen("ef0:/seplugins/game.txt", PSP_O_RDONLY, 0777);
		vsh = sceIoOpen("ef0:/seplugins/vsh.txt", PSP_O_RDONLY, 0777);
		pops = sceIoOpen("ef0:/seplugins/pops.txt", PSP_O_RDONLY, 0777);
		plugins = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	} else {
		game = sceIoOpen("ms0:/seplugins/game.txt", PSP_O_RDONLY, 0777);
		vsh = sceIoOpen("ms0:/seplugins/vsh.txt", PSP_O_RDONLY, 0777);
		pops = sceIoOpen("ms0:/seplugins/pops.txt", PSP_O_RDONLY, 0777);
		plugins = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	}
	// GAME.txt

	i = 0;
	while ((bytesRead = sceIoRead(game, buf, chunksize)) > 0) {
		for(; i < bytesRead; i++) {
			if (i == 0 || buf[i-1] == '\n')
				sceIoWrite(plugins, gameChar, gameCharLength);
			if (buf[i] == ' ')
				sceIoWrite(plugins, ",", 1);
			sceIoWrite(plugins, &buf[i], 1);
		}
	}
	
	sceIoClose(game);


	memset(buf, 0, chunksize);

	// VSH.txt
	i = 0;
	while ((bytesRead = sceIoRead(vsh, buf, chunksize)) > 0) {
		for(; i < bytesRead; i++) {
			if (i == 0 || buf[i-1] == '\n')
				sceIoWrite(plugins, vshChar, vshCharLength);
			if (buf[i] == ' ')
				sceIoWrite(plugins, ",", 1);

			sceIoWrite(plugins, &buf[i], 1);
		}
	}

	sceIoClose(vsh);

	memset(buf, 0, chunksize);


	// POP.txt

	i = 0;
	while ((bytesRead = sceIoRead(pops, buf, chunksize)) > 0) {
		for(; i < bytesRead; i++) {
			if (i == 0 || buf[i-1] == '\n')
				sceIoWrite(plugins, popsChar, popsCharLength);
			if (buf[i] == ' ')
				sceIoWrite(plugins, ",", 1);

			sceIoWrite(plugins, &buf[i], 1);
		}
	}

	sceIoClose(pops);

	sceIoClose(plugins);
	free(buf);

	sctrlKernelExitVSH(NULL);
	
}





static int get_umdvideo(UmdVideoList *list, char *path)
{
	SceIoDirent dir;
	int result = 0, dfd;
	char fullpath[256];

	memset(&dir, 0, sizeof(dir));
	dfd = sceIoDopen(path);

	if(dfd < 0) {
		return dfd;
	}

	while (sceIoDread(dfd, &dir) > 0) {
		const char *p;

		p = strrchr(dir.d_name, '.');

		if(p == NULL)
			p = dir.d_name;

		if(0 == stricmp(p, ".iso") || 0 == stricmp(p, ".cso") || 0 == stricmp(p, ".zso")) {
#ifdef CONFIG_639
			if(psp_fw_version == FW_639)
				scePaf_sprintf(fullpath, "%s/%s", path, dir.d_name);
#endif

#ifdef CONFIG_635
			if(psp_fw_version == FW_635)
				scePaf_sprintf(fullpath, "%s/%s", path, dir.d_name);
#endif

#ifdef CONFIG_620
			if (psp_fw_version == FW_620)
				scePaf_sprintf_620(fullpath, "%s/%s", path, dir.d_name);
#endif

#if defined(CONFIG_660) || defined(CONFIG_661)
			if ((psp_fw_version == FW_660) || (psp_fw_version == FW_661))
				scePaf_sprintf_660(fullpath, "%s/%s", path, dir.d_name);
#endif

			umdvideolist_add(list, fullpath);
		}
	}

	sceIoDclose(dfd);

	return result;
}

void exec_random_game() {
	int MAX_GAMES = 100;
	char GAME_DIR[32];
	int num_games = 0;
	char *games[MAX_GAMES];
	char *cat_games[MAX_GAMES];
	int count = 0;

	if(psp_model == PSP_GO) 
		sprintf(GAME_DIR, "%s", "ef0:/PSP/GAME/");
	else
		sprintf(GAME_DIR, "%s", "ms0:/PSP/GAME/");

	SceUID dir = sceIoDopen(GAME_DIR);
	SceIoDirent dirent;


	memset(&dirent, 0, sizeof(dirent));
	while(sceIoDread(dir, &dirent) > 0 && num_games < MAX_GAMES) {
		if(dirent.d_name != '.' && dirent.d_name != "..") {
			games[num_games] = malloc(strlen(GAME_DIR) + strlen(dirent.d_name) + 1);
			sprintf(games[num_games], "%s%s/", GAME_DIR, dirent.d_name);
			num_games++;
		}
	}
	sceIoDclose(dir);



	srand(time(NULL));
	int rand_idx = rand() % num_games;
	char *selected_game = games[rand_idx];


	if(strstr(selected_game, "/../") != NULL || strstr(selected_game, "/./") != NULL || strstr(selected_game, "/Infinity/") != NULL || 
			strstr(selected_game, "/TIMEMACHINE/") != NULL || strstr(selected_game, "/ARK_cIPL/") != NULL || 
			strstr(selected_game, "/ARK_Live/") != NULL || strstr(selected_game, "/UPDATE/") != NULL || strstr(selected_game, "%") != NULL) {
		rand_idx = rand() % num_games;
		selected_game = games[rand_idx];
	}

	strcat(selected_game, "EBOOT.PBP");
	int exists;;
	while(!(exists = sceIoOpen(selected_game, PSP_O_RDONLY, 0777))) {
		sceIoClose(exists);
		rand_idx = rand() % num_games;
		selected_game = games[rand_idx]; 
		sceIoClose(selected_game);
		exists = sceIoOpen(selected_game, PSP_O_RDONLY, 0777);
		if(strstr(selected_game, "CAT_") != NULL) {
			char rm_eboot[64];
			size_t rm_eboot_len = strlen("/EBOOT.PBP");
			rm_eboot[strlen(rm_eboot) - rm_eboot_len] = '\0';
			strcpy(selected_game, rm_eboot);
			SceUID cat_dir = sceIoDopen(selected_game);
			SceIoDirent catdir;
			num_games = 0;
			while(sceIoDread(cat_dir, &catdir) > 0 && num_games < MAX_GAMES) {
				if(catdir.d_name != '.' && catdir.d_name != "..") {
					cat_games[num_games] = malloc(strlen(selected_game) + strlen(catdir.d_name) + 1);
					sprintf(cat_games[num_games], "%s%s/", selected_game, dirent.d_name);
					num_games++;
				}
			}
		}
		if(strstr(selected_game, "/../") != NULL || strstr(selected_game, "/./") != NULL || strstr(selected_game, "/Infinity/") != NULL || 
			strstr(selected_game, "/TIMEMACHINE/") != NULL || strstr(selected_game, "/ARK_cIPL/") != NULL || 
			strstr(selected_game, "/ARK_Live/") != NULL || strstr(selected_game, "/UPDATE/") != NULL ||
			strstr(selected_game, '%') != NULL || strstr(selected_game, "UCA") != NULL || strstr(selected_game, "UCU") != NULL || strstr(selected_game, "@ISOGAME@") != NULL) {
			rand_idx = rand() % num_games;
			selected_game = games[rand_idx];
			exists = sceIoOpen(selected_game, PSP_O_RDONLY, 0777);
		}

		count++;

		if (count == 8) {
			count = 0;
			sceIoClose(exists);
			exec_random_game();
		}

		free(games);
		free(cat_games);

	}

	struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
	param.args = strlen(selected_game) + 1;
	param.argp = selected_game;
	param.key = "game";
	int apitype = 0x141;
	
	if (strstr(selected_game, "SLU") != NULL || strstr(selected_game, "CAT_PRX") != NULL) {
			param.key = "pops";
			apitype = 0x144;
	}
	/*
	char *ptr;
	char dir_eboot[64];
	if ((ptr = strstr(selected_game, "ms0:/PSP/GAME/@ISOGAME@")) != NULL) {
		memmove(ptr + strlen("/ISO/"), ptr + strlen("/PSP/GAME/"), strlen(ptr + strlen("/PSP/GAME/")) + 1);
		strncpy(ptr, "@ISOGAME@", strlen("@ISOGAME@"));
		strcpy(dir_eboot, selected_game);
		size_t fileNameLength = strlen("/EBOOT.PBP");
		dir_eboot[strlen(dir_eboot) - fileNameLength] = '\0';
		//strcpy(selected_game, "ms0:/ISO/");
		strcpy(selected_game, ptr);
		//strcat(selected_game, strchr(dir_eboot, '/') + 1);
	}
	*/
	
	// Testing to see what file is being called
	SceUID test = sceIoOpen("ms0:/selected_game.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    sceIoWrite(test, selected_game, strlen(selected_game));	
	sceIoClose(test);


	sctrlKernelLoadExecVSHWithApitype(apitype, selected_game, &param);

}

static void exec_custom_launcher(void) {
	char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config->arkpath);
    strcat(menupath, ARK_MENU);
    
    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(menupath) + 1;
    param.argp = menupath;
    param.key = "game";
    sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
}


static void launch_umdvideo_mount(void)
{
	SceIoStat stat;
	char *path;
	int type;

	if(0 == umdvideo_idx) {
		if(sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
			// cancel mount
			sctrlSESetUmdFile("");
			sctrlSESetBootConfFileIndex(MODE_UMD);
			sctrlKernelExitVSH(NULL);
		}

		return;
	}

	path = umdvideolist_get(&g_umdlist, (size_t)(umdvideo_idx-1));

	if(path == NULL) {
		return;
	}

	if(sceIoGetstat(path, &stat) < 0) {
		return;
	}

	type = vshDetectDiscType(path);
	printk("%s: detected disc type 0x%02X for %s\n", __func__, type, path);

	if(type < 0) {
		return;
	}

	sctrlSESetUmdFile(path);
	sctrlSESetBootConfFileIndex(MODE_VSHUMD);
	sctrlSESetDiscType(type);
	sctrlKernelExitVSH(NULL);
}

static char g_cur_font_select[256] __attribute((aligned(64)));

int load_recovery_font_select(void)
{
	SceUID fd;

	g_cur_font_select[0] = '\0';
	fd = sceIoOpen("ef0:/seplugins/font_recovery.txt", PSP_O_RDONLY, 0777);

	if(fd < 0) {
		fd = sceIoOpen("ms0:/seplugins/font_recovery.txt", PSP_O_RDONLY, 0777);

		if(fd < 0) {
			return fd;
		}
	}

	sceIoRead(fd, g_cur_font_select, sizeof(g_cur_font_select));
	sceIoClose(fd);

	return 0;
}

void clear_language(void)
{
	if (g_messages != g_messages_en) {
		free_translate_table((char**)g_messages, MSG_END);
	}

	g_messages = g_messages_en;
}

static char ** apply_language(char *translate_file)
{
	char path[512];
	char **message = NULL;
	int ret;

	sprintf(path, "ms0:/seplugins/%s", translate_file);
	ret = load_translate_table(&message, path, MSG_END);

	if(ret >= 0) {
		return message;
	}

	sprintf(path, "ef0:/seplugins/%s", translate_file);
	ret = load_translate_table(&message, path, MSG_END);

	if(ret >= 0) {
		return message;
	}

	return (char**) g_messages_en;
}

int cur_language = 0;

static void select_language(void)
{
	int ret, value;

	if(cnf.language == -1) {
		ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &value);

		if(ret != 0) {
			value = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
		}
	} else {
		value = cnf.language;
	}

	cur_language = value;
	clear_language();

	switch(value) {
		case PSP_SYSTEMPARAM_LANGUAGE_JAPANESE:
			g_messages = (const char**)apply_language("satelite_jp.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_ENGLISH:
			g_messages = (const char**)apply_language("satelite_en.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_FRENCH:
			g_messages = (const char**)apply_language("satelite_fr.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_SPANISH:
			g_messages = (const char**)apply_language("satelite_es.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_GERMAN:
			g_messages = (const char**)apply_language("satelite_de.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_ITALIAN:
			g_messages = (const char**)apply_language("satelite_it.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_DUTCH:
			g_messages = (const char**)apply_language("satelite_nu.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_PORTUGUESE:
			g_messages = (const char**)apply_language("satelite_pt.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN:
			g_messages = (const char**)apply_language("satelite_ru.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_KOREAN:
			g_messages = (const char**)apply_language("satelite_kr.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL:
			g_messages = (const char**)apply_language("satelite_cht.txt");
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED:
			g_messages = (const char**)apply_language("satelite_chs.txt");
			break;
		default:
			g_messages = g_messages_en;
			break;
	}

	if(g_messages == g_messages_en) {
		cur_language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	}
}

void (*SysconGetBaryonVersion)(u32*);
u32 (*SysregGetTachyonVersion)();
int (*SysconCmdExec)(u8*, int);

u32 write_eeprom(u8 addr, u16 data)
{
    int res;
    u8 param[0x60];
	struct KernelCallArg args;

    if(addr > 0x7F) return 0x80000102;

    param[0x0C] = 0x73;
    param[0x0D] = 5;
    param[0x0E] = addr;
    param[0x0F] = data;
    param[0x10] = data >> 8;

	memset(&args, 0, sizeof(args));
	args.arg1 = param;
	kuKernelCall(SysconCmdExec, &args);
	res = args.ret1;

    if(res < 0) return res;

    return 0;
}

u32 read_eeprom(u8 addr)
{
    int res;
    u8 param[0x60];
	struct KernelCallArg args;

    if(addr > 0x7F) return 0x80000102;

    param[0x0C] = 0x74;
    param[0x0D] = 3;
    param[0x0E] = addr;

	memset(&args, 0, sizeof(args));
	args.arg1 = param;
	kuKernelCall(SysconCmdExec, &args);
	res = args.ret1;

    if(res < 0) return res;

    return((param[0x21] << 8) | param[0x20]);
}

int errCheck(u32 data)
{
    if((data & 0x80250000) == 0x80250000) return -1;
    else if(data & 0xFFFF0000) return((data & 0xFFFF0000) >> 16);
    return 0;
}

int ReadSerial(u16* pdata)
{
    int err = 0;
    u32 data;

    data = read_eeprom(0x07);
    err = errCheck(data);
    if(!(err < 0))
    {
        pdata[0] = (data & 0xFFFF);
        data = read_eeprom(0x09);
        err = errCheck(data);
        if(!(err < 0)) pdata[1] = (data & 0xFFFF);
        else err = data;
    }
    else err = data;

    return err;
}

int WriteSerial(u16* pdata)
{
    int err = 0;

    err = write_eeprom(0x07, pdata[0]);
    if(!err) err = write_eeprom(0x09, pdata[1]);

    return err;
}

static void convert_battery(void) {
	if (is_pandora < 0) return;
	u16 buffer[2];

	if (is_pandora){
		buffer[0] = 0x1234;
        buffer[1] = 0x5678;
	}
	else{
		buffer[0] = 0xFFFF;
        buffer[1] = 0xFFFF;
	}
	WriteSerial(buffer);
};

static void check_battery(void) {

    int sel = 0;
    int batsel;
    u32 baryon, tachyon;
	struct KernelCallArg args;

	SysconGetBaryonVersion = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
	SysregGetTachyonVersion = sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0xE2A5D1EE);
	SysconCmdExec = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x5B9ACC97);

	memset(&args, 0, sizeof(args));
	args.arg1 = &baryon;
	kuKernelCall(SysconGetBaryonVersion, &args);

	memset(&args, 0, sizeof(args));
	kuKernelCall(SysregGetTachyonVersion, &args);
	tachyon = args.ret1;

    if(tachyon >= 0x00500000 && baryon >= 0x00234000) is_pandora = -1;
    else
    {   
        u16 serial[2];
        ReadSerial(serial);
    
        if(serial[0] == 0xFFFF && serial[1] == 0xFFFF) is_pandora = 1;
        else is_pandora = 0;
    }
}

void delete_hibernation(){
	if (psp_model == PSP_GO){
		vshCtrlDeleteHibernation();
		sctrlKernelExitVSH(NULL);
	}
}

static int activate_codecs()
{
	set_registry_value("/CONFIG/BROWSER", "flash_activated", 1);
	set_registry_value("/CONFIG/BROWSER", "flash_play", 1);
	set_registry_value("/CONFIG/MUSIC", "wma_play", 1);

	sctrlKernelExitVSH(NULL);
	
	return 0;
}

static int swap_buttons()
{
	u32 value;
	get_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", &value);
	value = !value;
	set_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", value);
	
	sctrlKernelExitVSH(NULL);

	return 0;
}

void loadConfig(){

	char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, CONFIG_PATH);

    int fp = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fp < 0){
        return;
    }
	sceIoRead(fp, &config, sizeof(t_conf));
    sceIoClose(fp);

	cnf.vsh_bg_colors = config.vsh_bg_color;
	cnf.vsh_fg_colors = config.vsh_fg_color;
}

void saveConfig(){
	char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, CONFIG_PATH);

    int fp = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
	if (fp < 0) return;

	sceIoWrite(fp, &config, sizeof(t_conf));
    sceIoClose(fp);
}

int TSRThread(SceSize args, void *argp)
{
	sceKernelChangeThreadPriority(0, 8);
	vctrlVSHRegisterVshMenu(EatKey);
	sctrlSEGetConfig(&cnf);

	check_battery();

	loadConfig();

	load_recovery_font_select();
	select_language();

	if(g_cur_font_select[0] != '\0') {
		load_external_font(g_cur_font_select);
	}

	umdvideolist_init(&g_umdlist);
	umdvideolist_clear(&g_umdlist);
	get_umdvideo(&g_umdlist, "ms0:/ISO/VIDEO");
	get_umdvideo(&g_umdlist, "ef0:/ISO/VIDEO");
	kuKernelGetUmdFile(umdvideo_path, sizeof(umdvideo_path));

	if(umdvideo_path[0] == '\0') {
		umdvideo_idx = 0;
		strcpy(umdvideo_path, g_messages[MSG_NONE]);
	} else {
		umdvideo_idx = umdvideolist_find(&g_umdlist, umdvideo_path);

		if(umdvideo_idx >= 0) {
			umdvideo_idx++;
		} else {
			umdvideo_idx = 0;
			strcpy(umdvideo_path, g_messages[MSG_NONE]);
		}
	}

#ifdef CONFIG_639
	if(psp_fw_version == FW_639)
		scePaf_memcpy(&cnf_old, &cnf, sizeof(SEConfig));
#endif

#ifdef CONFIG_635
	if(psp_fw_version == FW_635)
		scePaf_memcpy(&cnf_old, &cnf, sizeof(SEConfig));
#endif

#ifdef CONFIG_620
	if (psp_fw_version == FW_620)
		scePaf_memcpy_620(&cnf_old, &cnf, sizeof(SEConfig));
#endif

#if defined(CONFIG_660) || defined(CONFIG_661)
	if ((psp_fw_version == FW_660) || (psp_fw_version == FW_661))
		scePaf_memcpy_660(&cnf_old, &cnf, sizeof(SEConfig));
#endif
resume:
	while(stop_flag == 0) {
		if( sceDisplayWaitVblankStart() < 0)
			break; // end of VSH ?

		if(menu_mode > 0) {
			menu_setup();
			menu_draw();
		}

		button_func();
	}

	if(scePaf_memcmp(&cnf_old, &cnf, sizeof(SEConfig))){
		vctrlVSHUpdateConfig(&cnf);
	}

	if (stop_flag ==2) {
		scePowerRequestColdReset(0);
	} else if (stop_flag ==3) {
		scePowerRequestStandby();
	} else if (stop_flag ==4) {
		sctrlKernelExitVSH(NULL);
	} else if (stop_flag == 5) {
		scePowerRequestSuspend();
	} else if (stop_flag == 6) {
		launch_umdvideo_mount();
	} else if (stop_flag == 7) {
		exec_custom_launcher();
	} else if (stop_flag == 8) {
		exec_recovery_menu();
	/*} else if (stop_flag == 9) {
		convert_battery();
	} else if (stop_flag == 10) {
		delete_hibernation();
	} else if (stop_flag == 11) {
		activate_codecs();
	} else if (stop_flag == 12) {
		swap_buttons();
	} else if (stop_flag == 13) {
		import_classic_plugins();
	} else if (stop_flag == 14) {
		exec_random_game(); */
	} else if(stop_flag == 15) {
		// AVSHMENU START
		while(sub_stop_flag == 0) {
			if( sceDisplayWaitVblankStart() < 0)
				break; // end of VSH ?
			if(submenu_mode > 0) {
				submenu_setup();
				submenu_draw();
			}

			subbutton_func();
		}
	}

	if ( sub_stop_flag == 6)
		launch_umdvideo_mount();
	else if (sub_stop_flag == 9)
		convert_battery();
	else if (sub_stop_flag == 10)
		delete_hibernation();
	else if (sub_stop_flag == 11)
		activate_codecs();
	else if (sub_stop_flag == 12)
		swap_buttons();
	else if (sub_stop_flag == 13)
		import_classic_plugins();
	else if (sub_stop_flag == 14)
		exec_random_game();
	else if(sub_stop_flag == 1 ) {
		stop_flag = 0;
		menu_mode = 0;
		sub_stop_flag = 0;
		goto resume;
	}
		


	config.vsh_bg_color = cnf.vsh_bg_colors;
	config.vsh_fg_color = cnf.vsh_fg_colors;
	saveConfig();

	vctrlVSHUpdateConfig(&cnf);

	umdvideolist_clear(&g_umdlist);
	clear_language();
	vpl_finish();

	vctrlVSHExitVSHMenu(&cnf, NULL, 0);
	release_font();

	return sceKernelExitDeleteThread(0);
}
