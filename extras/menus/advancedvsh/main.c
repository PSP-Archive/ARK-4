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

/* avshMenu by krazynez
 * based on PRO vsh, ME vsh, and ultimate vsh, and the Original ARK-4 vshmenu.
 * Plus myself and acid_snake's mentally insane thoughts and awesomeness ;-)
 */
#include <pspkernel.h>
#include <psputility.h>
#include <stdio.h>
#include <stdbool.h>
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

#define MAX_GAMES 50

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
	if ((psp_fw_version == FW_660) || (psp_fw_version == FW_661))
		scePaf_memcpy_660(&ctrl_pad, pad_data, sizeof(SceCtrlData));

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

	// menu control
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

	// menu control
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
			if ((psp_fw_version == FW_660) || (psp_fw_version == FW_661))
				scePaf_sprintf_660(fullpath, "%s/%s", path, dir.d_name);
			umdvideolist_add(list, fullpath);
		}
	}

	sceIoDclose(dfd);

	return result;
}
/*
int skip_game(char *game) {
	sceKernelDelayThread(500000);
	//char path2[] = "ms0:/a_blacklist.txt";
	//FILE *wb = fopen(path2, "a");
	//fwrite(game, 1, strlen(game), wb);
	//fwrite("\n", 1, sizeof(char), wb);
	//fclose(wb);
	char path[20];
	SceUID ms0_on_GO;
	if (psp_model == PSP_GO) {
		if(!(ms0_on_GO = sceIoOpen("ms0:/blacklist.txt", PSP_O_RDONLY, 0777)))
			snprintf(path, sizeof(path), "%s", "ef0:/blacklist.txt");
		else
			snprintf(path, sizeof(path), "%s", "ms0:/blacklist.txt");
	}
	else
		snprintf(path, sizeof(path), "%s", "ms0:/blacklist.txt");

	char line[256];
	if(strstr(game, "/.") != NULL || strstr(game, "/..") != NULL || strstr(game, "/!") != NULL)
		return 1;
	//SceUID blacklist = sceIoOpen(path, PSP_O_RDONLY, 0777);
	FILE *blacklist = fopen(path, "r");
	while(fgets(line, sizeof(line), blacklist)) {
		line[strcspn(line, "\n")] = '\0';
		if(strcasecmp(line, game) == 0) {
			fclose(blacklist);
			//sceIoClose(blacklist);
			return 1;
		}
	}
	//sceIoClose(blacklist);
	fclose(blacklist);
	return 0;
}

int game_exist(char* game, char* tmp) {
	strcpy(tmp, game);
	strcat(tmp, "EBOOT.PBP");
	SceUID exists = sceIoOpen(tmp, PSP_O_RDONLY, 0777);
	if (!exists) {
		return 1;
	}
	else {
		sceIoClose(exists);
		return 0;
	}
} // END game_exist
*/
void exec_random_game() {
    char* iso_dir = "xx0:/ISO/";
    int num_games = 0;
    char game[256];
    //char selected_game;
    //chariso_games[MAX_GAMES];

    if(psp_model == PSP_GO){
        iso_dir[0] = 'e';
        iso_dir[1] = 'f';
    }
    else{
        iso_dir[0] = 'm';
        iso_dir[1] = 's';
    }

    SceUID iso_path = sceIoDopen(iso_dir);
    SceIoDirent isos;

    memset(&isos, 0, sizeof(isos));
    while(sceIoDread(iso_path, &isos) > 0) {
        if(FIO_SO_ISREG(isos.d_stat.st_attr) && isos.d_name[0] != '.') {
            num_games++;
        }
    }

    sceIoDclose(iso_path);

    srand(time(NULL));
    int rand_idx = rand() % num_games;
    num_games = 0;

    iso_path = sceIoDopen(iso_dir);

    memset(&isos, 0, sizeof(isos));
    while(sceIoDread(iso_path, &isos) > 0) {
        if(isos.d_name[0] == '.') continue;
        else if(rand_idx == num_games) {
            break;
        }
        else if (FIO_SO_ISREG(isos.d_stat.st_attr)){
            num_games++;
        }
    }

    sceIoDclose(iso_path);

    strcpy(game, iso_dir);
    strcat(game, isos.d_name);

    struct SceKernelLoadExecVSHParam param;
    int apitype;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = 33;
    param.argp = (char*)"disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
    param.key = "umdemu";
    if (psp_model == PSP_GO)
        apitype = 0x125;
    else
        apitype = 0x123;


    sctrlSESetBootConfFileIndex(3);
    sctrlSESetUmdFile(game);

    sctrlKernelLoadExecVSHWithApitype(apitype, game, &param);

}
/*
	SceUID dir = sceIoDopen(GAME_DIR);
	SceIoDirent dirent;

	memset(&dirent, 0, sizeof(dirent));
	while(sceIoDread(dir, &dirent) > 0) {
		if(dirent.d_name == "." || dirent.d_name == "..") {
			bad_count++;
			continue;
		}
		else { 
			bad_count++;
			games[num_games] = (char *)malloc(strlen(GAME_DIR) + strlen(dirent.d_name) + 1);
			sprintf(games[num_games], "%s%s/", GAME_DIR, dirent.d_name);
			num_games++;
		}
	}
	sceIoDclose(dir);

	// No Games detected
	if(bad_count==2)
		sctrlKernelExitVSH(NULL);

	int rand_idx = rand() % num_games;
	selected_game = games[rand_idx];
/\/\*
	 while(skip_game(selected_game)) {
		rand_idx = rand() % num_games;
		selected_game = games[rand_idx];
	 }

	//char *tmp_game_holder = (char *)malloc(strlen(selected_game)+ 11);
	//while(game_exist(selected_game, tmp_game_holder) > 0 && strstr(selected_game, "CAT_") != NULL && skip_game(selected_game)) {
	//while(game_exist(selected_game, tmp_game_holder) > 0 || skip_game(selected_game) > 0) {
	while(skip_game(selected_game) > 0) {
		rand_idx = rand() % num_games;
		selected_game = games[rand_idx];
	}

	//free(tmp_game_holder);

#ifdef DEBUG
	char *all_games = "ms0:/all_games.txt";

	int all_games_ret = sceIoRemove(all_games);

	int z;
	char *tmp_list = (char *)malloc(sizeof(char)*64);
	SceUID t = sceIoOpen("ms0:/all_games.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
	for(z = 0; z < num_games; z++) {
		strcpy(tmp_list, games[z]);
		strcat(tmp_list, "\n");
		sceIoWrite(t, tmp_list, strlen(tmp_list));
	}
	sceIoClose(t);
	free(tmp_list);
#endif


	// Added for better stability
	sceKernelDelayThread(500000);


#ifdef DEBUG
	SceUID test_norm = sceIoOpen("ms0:/selected_game.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	char *tmp_g = (char *)malloc(strlen(selected_game)); // + strlen(" Normal Game"));
	strcpy(tmp_g, selected_game);
	strcat(tmp_g, " Normal Game");
    sceIoWrite(test_norm, tmp_g, strlen(tmp_g));	
	sceIoClose(test_norm);
#endif

	while(strstr(selected_game, "CAT_") != NULL || skip_game(selected_game) > 0) {
			//if(strstr(selected_game, ".") != NULL || strstr(selected_game, "..") != NULL || skip_game(selected_game) > 0) {
			if(strstr(selected_game, ".") != NULL || strstr(selected_game, "..") != NULL) {
				rand_idx = rand() % num_games; 
				selected_game = games[rand_idx];
			}
			rand_idx = rand() % num_games; 
			selected_game = games[rand_idx];
	}

*/
/*
	// CATEGORY LITE SUPPORT
	if (strstr(selected_game, "CAT_") != NULL) {
		num_games = 0;


		SceUID cat_dir = sceIoDopen(selected_game);
		SceIoDirent catdir;
		memset(&catdir, 0, sizeof(catdir));
		bad_count = 0;	

		while(sceIoDread(cat_dir, &catdir) > 0) {
			if(strstr(catdir.d_name, "/.") != NULL) {
				bad_count++;
				continue;
			}
			else {
				cat_games[num_games] = (char *)malloc(strlen(selected_game) + strlen(catdir.d_name) + 1);
				sprintf(cat_games[num_games], "%s%s/", selected_game, catdir.d_name);
				num_games++;
			}


			// Random Game from Categories
			rand_idx = rand() % num_games;
			selected_game = cat_games[rand_idx];

		
		}
		sceIoDclose(cat_dir);
		
		// No Games detected
		if(bad_count==2)
			sctrlKernelExitVSH(NULL);

		while(skip_game(selected_game)) {
				rand_idx = rand() % num_games;
				selected_game = cat_games[rand_idx];
			}

		char *tmp_game_cat = (char *)malloc(strlen(selected_game) + 11);		
		while(game_exist(selected_game, tmp_game_cat) > 0 && skip_game(selected_game)) {
			rand_idx = rand() % num_games;
			selected_game = cat_games[rand_idx];
		}
		free(tmp_game_cat);
#ifdef DEBUG
		SceUID test_cat = sceIoOpen("ms0:/selected_game.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		char *tmp_cat_g = (char *)malloc(strlen(selected_game) + 10);
		strcpy(tmp_cat_g, selected_game);
		strcat(tmp_cat_g, " CAT GAME");
    	sceIoWrite(test_cat, tmp_cat_g, strlen(tmp_cat_g));	
		sceIoClose(test_cat);
		free(tmp_cat_g);
#endif
	} // END OF CAT LITE CHECK
	// Append EBOOT.PBP to end of gamelist
	strcat(selected_game, "EBOOT.PBP");


*/


/*	
	//if (strstr(selected_game, "SLU") != NULL || strstr(selected_game, "CAT_PRX") != NULL || strstr(selected_game, "/NPU") != NULL) {
	if (strstr(selected_game, "SLU") != NULL || strstr(selected_game, "/NPU") != NULL) {
			param.key = "pops";
			apitype = 0x144;
	}

	//else if ((strstr(selected_game, "SLU") || strstr(selected_game, "CAT_PRX") || strstr(selected_game, "/NPU")) != NULL && psp_model == PSP_GO) {
	else if ((strstr(selected_game, "SLU") || strstr(selected_game, "/NPU")) != NULL && psp_model == PSP_GO) {
			param.key = "pops";
			apitype = 0x155;
	}

	else if (strstr(selected_game, "ISOGAME") != NULL) {
		param.argp = (char*)"disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"; 
		if(psp_model == PSP_GO)
			apitype = 0x125;
		else
			apitype = 0x123;

		param.key = "umdemu";
		param.args = 33;


*/
/*
		while(skip_game(selected_game) > 0) {
			rand_idx = rand() % num_games; 
			selected_game = iso_games[rand_idx];
		}
		if(iso_num_games != 0) {
			rand_idx = rand() % iso_num_games; 
			selected_game = cat_iso_games[rand_idx];

		}
			SceUID cat_iso_dir = sceIoDopen(selected_game);
			SceIoDirent catIsoDir;
			memset(&catIsoDir, 0, sizeof(catIsoDir));
			iso_num_games = 0;
			//char *isos;

			while(sceIoDread(cat_iso_dir, &catIsoDir) > 0) {
				isos[iso_num_games] = (char *)malloc(sizeof(char)*64);
				sprintf(isos[iso_num_games], "%s", catIsoDir.d_name);
				iso_num_games++;
			}


			rand_idx = rand() % iso_num_games; 
			selected_game = isos[rand_idx];
			sceIoDclose(cat_iso_dir);


		} // END ISO_NUM_GAMES


		while(skip_game(selected_game)) {
			rand_idx = rand() % num_games; 
			selected_game = iso_games[rand_idx];
		}

		// Testing to see what file is being called
#ifdef DEBUG
	SceUID test = sceIoOpen("ms0:/selected_game.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    sceIoWrite(test, selected_game, strlen(selected_game));	
	sceIoClose(test);
#endif

*/

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

	u32 tmp_swap_xo_32;
	get_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", &tmp_swap_xo_32);
	cnf.swap_xo = tmp_swap_xo_32;
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

if ((psp_fw_version == FW_660) || (psp_fw_version == FW_661))
	scePaf_memcpy_660(&cnf_old, &cnf, sizeof(SEConfig));

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
		submenu_mode = 0;
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
