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

#include "xmbiso.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "isoreader.h"
#include "printk.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include "virtual_pbp.h"
#include "strsafe.h"
#include "dirent_track.h"

#define MAGIC_DFD_FOR_DELETE 0x9000
#define MAGIC_DFD_FOR_DELETE_2 0x9001

static char g_iso_dir[128];
static char g_temp_delete_dir[128];
static int g_delete_eboot_injected = 0;

static int is_iso_dir(const char *path)
{
	const char *p;

	if (path == NULL)
		return 0;

	p = strchr(path, '/');

	if (p == NULL)
		return 0;

	if (p <= path + 1 || p[-1] != ':')
		return 0;

	p = strstr(p, ISO_ID);

	if (NULL == p) {
		return 0;
	}

	p = strrchr(path, '@') + 1;
	p += 8;

	while(*p != '\0' && *p == '/')
		p++;

	if (*p != '\0')
		return 0;

	return 1;
}

static int is_iso_eboot(const char* path)
{
	const char *p;

	if (path == NULL)
		return 0;

	p = strchr(path, '/');

	if (p == NULL)
		return 0;

	if (p <= path + 1 || p[-1] != ':')
		return 0;

	p = strstr(p, ISO_ID);

	if (NULL == p) {
		return 0;
	}

	p = strrchr(path, '@') + 1;
	p += 8;

	if (0 != strcmp(p, "/EBOOT.PBP"))
		return 0;

	return 1;
}

static inline int is_game_dir(const char *dirname)
{
	const char *p;
	char path[256];
	SceIoStat stat;

	p = strchr(dirname, '/');

	if (p == NULL) {
		return 0;
	}

	if (0 != strnicmp(p, "/PSP/GAME", sizeof("/PSP/GAME")-1)) {
		return 0;
	}

	if (0 == strnicmp(p, "/PSP/GAME/_DEL_", sizeof("/PSP/GAME/_DEL_")-1)) {
		return 0;
	}

	STRCPY_S(path, dirname);
	STRCAT_S(path, "/EBOOT.PBP");

	if(0 == sceIoGetstat(path, &stat)) {
		return 0;
	}

	STRCPY_S(path, dirname);
	STRCAT_S(path, "/PARAM.PBP");

	if(0 == sceIoGetstat(path, &stat)) {
		return 0;
	}

	return 1;
}

//open directory
SceUID gamedopen(const char * dirname)
{
	SceUID result;
	u32 k1;

	if(is_iso_dir(dirname)) {
		result = MAGIC_DFD_FOR_DELETE;
		g_delete_eboot_injected = 0;
		strncpy(g_iso_dir, dirname, sizeof(g_iso_dir));
		g_iso_dir[sizeof(g_iso_dir)-1] = '\0';
//		printk("%s:<virtual> %s -> 0x%08X\n", __func__, dirname, result);
		
		return result;
	}

	if(0 == strcmp(dirname, g_temp_delete_dir)) {
		result = MAGIC_DFD_FOR_DELETE_2;
//		printk("%s:<virtual2> %s -> 0x%08X\n", __func__, dirname, result);
		
		return result;
	}
   
	result = sceIoDopen(dirname);
	
	if(is_game_dir(dirname)) {
		char path[256];
		const char *p;
		int iso_dfd, ret;
		
		get_device_name(path, sizeof(path), dirname);
		STRCAT_S(path, "/ISO");

		p = strstr(dirname, "/PSP/GAME");

		if(p != NULL) {
			p += sizeof("/PSP/GAME") - 1;
			STRCAT_S(path, p);
		}

		k1 = pspSdkSetK1(0);
		iso_dfd = vpbp_dopen(path);
		pspSdkSetK1(k1);

		if(iso_dfd < 0) {
			goto exit;
		}

		if(result < 0) {
			result = iso_dfd;
		}
		
		ret = dirent_add(result, iso_dfd, dirname); 

		if(ret < 0) {
			printk("%s: dirent_add -> %d\n", __func__, ret);

			result = -1;
			goto exit;
		}
	}

exit:
	printk("%s: %s -> 0x%08X\n", __func__, dirname, result);

	return result;
}

//read directory
int gamedread(SceUID fd, SceIoDirent * dir)
{
	int result;
	u32 k1;

	if(fd == MAGIC_DFD_FOR_DELETE || fd == MAGIC_DFD_FOR_DELETE_2) {
		if (0 == g_delete_eboot_injected) {
			u32 k1;
		   
			memset(dir, 0, sizeof(*dir));
			k1 = pspSdkSetK1(0);
			result = vpbp_getstat(g_iso_dir, &dir->d_stat);
			pspSdkSetK1(k1);

			if(fd == MAGIC_DFD_FOR_DELETE) {
				strcpy(dir->d_name, "EBOOT.PBP");
			} else {
				strcpy(dir->d_name, "_EBOOT.PBP");
			}

			g_delete_eboot_injected = 1;
			result = 1;
		} else {
			result = 0;
		}
		
//		printk("%s:<virtual> 0x%08X -> 0x%08X\n", __func__, fd, result);
		
		return result;
	}

	result = sceIoDread(fd, dir);

	if(result <= 0) {
		struct IoDirentEntry *entry;

		entry = dirent_search(fd);

		if(entry != NULL) {
			k1 = pspSdkSetK1(0);
			result = vpbp_dread(fd, dir);
			pspSdkSetK1(k1);
		}
	}

	printk("%s: 0x%08X %s -> 0x%08X\n", __func__, fd, dir->d_name, result);

	return result;
}

//directory descriptor closer
int gamedclose(SceUID fd)
{
	int result;
	u32 k1;
	struct IoDirentEntry *entry;
   
	if(fd == MAGIC_DFD_FOR_DELETE || fd == MAGIC_DFD_FOR_DELETE_2) {
		result = 0;
		g_delete_eboot_injected = 0;
//		printk("%s:<virtual> 0x%08X -> 0x%08X\n", __func__, fd, result);
		
		return result;
	}
	
	entry = dirent_search(fd);

	if(entry != NULL) {
		if(entry->iso_dfd == fd) {
			k1 = pspSdkSetK1(0);
			vpbp_dclose(fd);
			pspSdkSetK1(k1);

			result = 0;
		} else if (entry->dfd == fd) {
			k1 = pspSdkSetK1(0);
			result = vpbp_dclose(fd);
			pspSdkSetK1(k1);
		} else {
			result = sceIoDclose(fd);
		}

		dirent_remove(entry);
	} else {
		result = sceIoDclose(fd);
	}

	printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, result);

	return result;
}

//open file
SceUID gameopen(const char * file, int flags, SceMode mode)
{
	//forward to firmware
	SceUID result;
   
	if (is_iso_eboot(file)) {
//		printk("%s:<virtual> %s", __func__, file);
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_open(file, flags, mode);
		pspSdkSetK1(k1);
//		printk(" -> 0x%08X\n", result);
	} else {
		result = sceIoOpen(file, flags, mode);
	}

	return result;
}

//read file
int gameread(SceUID fd, void * data, SceSize size)
{
	//forward to firmware
	int result;
   
	if (vpbp_is_fd(fd)) {
//		printk("%s: 0x%04X %d", __func__, fd, size);
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_read(fd, data, size);
		pspSdkSetK1(k1);
//		printk(" -> 0x%08X\n", result);
	} else {
		result = sceIoRead(fd, data, size);
	}

	return result;
}

//close file
int gameclose(SceUID fd)
{
	int result;

	if (vpbp_is_fd(fd)) {
//		printk("%s: %04X", __func__, fd);
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_close(fd);
		pspSdkSetK1(k1);
//		printk(" -> 0x%08X\n", result);
	} else {
		result = sceIoClose(fd);
	}
	
	return result;
}

SceOff gamelseek(SceUID fd, SceOff offset, int whence)
{
	SceOff result = 0;

	if (vpbp_is_fd(fd)) {
//		printk("%s: 0x%04X 0x%08X %d", __func__, fd, (u32)offset, whence);
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_lseek(fd, offset, whence);
		pspSdkSetK1(k1);
//		printk(" -> 0x%08X\n", (u32)result);
	} else {
		result = sceIoLseek(fd, offset, whence);
	}

	return result;
}

//get file status
int gamegetstat(const char * file, SceIoStat * stat)
{
	int result;
   
	//virtual iso eboot detected
	if (is_iso_eboot(file)) {
//		printk("%s:<virtual> %s", __func__, file);
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_getstat(file, stat);
		pspSdkSetK1(k1);
//		printk(" -> 0x%08X\n", result);
	} else {
		result = sceIoGetstat(file, stat);
	}

	return result;
}

//remove file
int gameremove(const char * file)
{
	int result;
   
	if(g_temp_delete_dir[0] != '\0' && 
			0 == strncmp(file, g_temp_delete_dir, strlen(g_temp_delete_dir))) {
		result = 0;
		printk("%s:<virtual> %s -> 0x%08X\n", __func__, file, result);
		
		return result;
	}
	
	result = sceIoRemove(file);
	printk("%s: %s -> 0x%08X\n", __func__, file, result);

	return result;
}

//remove folder
int gamermdir(const char * path)
{
	int result;
   
	if(0 == strcmp(path, g_temp_delete_dir)) {
		strcat(g_iso_dir, "/EBOOT.PBP");
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_remove(g_iso_dir);
		pspSdkSetK1(k1);
		printk("%s:<virtual> %s -> 0x%08X\n", __func__, path, result);
		g_iso_dir[0] = '\0';
		g_temp_delete_dir[0] = '\0';

		return result;
	}

	result = sceIoRmdir(path);
	printk("%s: %s 0x%08X\n", __func__, path, result);

	return result;
}

//load and execute file
int gameloadexec(char * file, struct SceKernelLoadExecVSHParam * param)
{
	//result
	int result = 0;

	printk("%s: %s %s\n", __func__, file, param->key);
	
	//enable high memory on demand
	SEConfig config;
	sctrlSEGetConfig(&config);
	if(config.retail_high_memory) sctrlHENSetMemory(55, 0);
	
	//virtual iso eboot detected
	if (is_iso_eboot(file)) {
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_loadexec(file, param);
		pspSdkSetK1(k1);

		return result;
	}

	//forward to ms0 handler
	if(strncmp(file, "ms", 2) == 0) result = sctrlKernelLoadExecVSHMs2(file, param);

	//forward to ef0 handler
	else result = sctrlKernelLoadExecVSHEf2(file, param);

	return result;
}

int gamerename(const char *oldname, const char *newfile)
{
	int result;

	if(is_iso_dir(oldname)) {
		result = 0;
		strncpy(g_iso_dir, oldname, sizeof(g_iso_dir));
		g_iso_dir[sizeof(g_iso_dir)-1] = '\0';
		strncpy(g_temp_delete_dir, newfile, sizeof(g_temp_delete_dir));
		g_temp_delete_dir[sizeof(g_temp_delete_dir)-1] = '\0';

		printk("%s:<virtual> %s %s -> 0x%08X\n", __func__, oldname, newfile, result);

		return 0;
	}

	if(g_temp_delete_dir[0] != '\0' &&
			0 == strncmp(oldname, g_temp_delete_dir, strlen(g_temp_delete_dir))) {
		result = 0;
		printk("%s:<virtual2> %s %s -> 0x%08X\n", __func__, oldname, newfile, result);

		return 0;
	}

	result = sceIoRename(oldname, newfile);
	printk("%s: %s %s -> 0x%08X\n", __func__, oldname, newfile, result);

	return result;
}

int gamechstat(const char *file, SceIoStat *stat, int bits)
{
	int result;

	if(g_temp_delete_dir[0] != '\0' && 
			0 == strncmp(file, g_temp_delete_dir, strlen(g_temp_delete_dir))) {
		result = 0;
		printk("%s:<virtual> %s -> 0x%08X\n", __func__, file, result);

		return 0;
	}

	result = sceIoChstat(file, stat, bits);
	printk("%s: %s -> 0x%08X\n", __func__, file, result);

	return result;
}
