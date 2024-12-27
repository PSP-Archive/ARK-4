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
#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <psputility.h>

#include <string.h>

#include "common.h"

#include <systemctrl.h>
#include <systemctrl_se.h>
#include "vpl.h"

#include "scepaf.h"
#include "vsh.h"

#include "trans.h"


#define READ_BUF_SIZE 1024


static char *read_buf = NULL;
static char *read_ptr = NULL;
static int read_cnt = 0;


static int buf_read(SceUID fd, char *p)
{
	if(read_cnt <= 0) {
		read_cnt = sceIoRead(fd, read_buf, READ_BUF_SIZE);

		if(read_cnt < 0) {
			return read_cnt;
		}

		if(read_cnt == 0) {
			return read_cnt;
		}

		read_ptr = read_buf;
	}

	read_cnt--;
	*p = *read_ptr++;

	return 1;
}

static int read_lines(SceUID fd, char *lines, size_t linebuf_size)
{
	char *p;
	int ret;
	size_t re;

	if(linebuf_size == 0) {
		return -1;
	}

	p = lines;
	re = linebuf_size;

	while(re -- != 0) {
		ret = buf_read(fd, p);

		if(ret < 0) {
			break;
		}

		if(ret == 0) {
			if(p == lines) {
				ret = -1;
			}

			break;
		}

		if(*p == '\r') {
			continue;
		}

		if(*p == '\n') {
			break;
		}

		p++;
	}

	if(p < lines + linebuf_size) {
		*p = '\0';
	}

	return ret >= 0 ? p - lines : ret;
}

static void set_translate_table_item(char ***table, char *linebuf, int pos, int nr_trans)
{
	if(*table == NULL) {
		*table = (char**)vpl_alloc(sizeof(char*) * nr_trans);
		scePaf_memset(*table, 0, sizeof(char*) * nr_trans);
	}

	(*table)[pos] = vpl_strdup(linebuf);
}

SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath) {

	int pkg = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);
	if (pkg < 0)
		return 0;
	 
	unsigned pkgsize = sceIoLseek32(pkg, 0, PSP_SEEK_END);
	unsigned size2 = 0;
	 
	sceIoLseek32(pkg, 0, PSP_SEEK_SET);

	if (size != NULL)
		*size = 0;

	unsigned offset = 0;
	char name[64];
		   
	while (offset != 0xFFFFFFFF) {
		sceIoRead(pkg, &offset, 4);
		if (offset == 0xFFFFFFFF) {
			sceIoClose(pkg);
			return 0;
		}
		unsigned namelength;
		sceIoRead(pkg, &namelength, 4);
		sceIoRead(pkg, name, namelength+1);
				   
		if (!scePaf_strncmp(name, filename, namelength)){
			sceIoRead(pkg, &size2, 4);
	
			if (size2 == 0xFFFFFFFF)
				size2 = pkgsize;

			if (size != NULL)
				*size = size2 - offset;
	 
			sceIoClose(pkg);
			return offset;
		}
	}
	return 0;
}

int load_translate_table(char ***table, char *file, int nr_trans) {
	SceUID fd;
	char linebuf[128];
	char *read_alloc_buf;
	int i;
	
	vsh_Menu *vsh = vsh_menu_pointer();
	

	if (table == NULL) {
		return -1;
	}

	*table = NULL;

	scePaf_strcpy(linebuf, vsh->config.p_ark->arkpath);
	strcat(linebuf, "VSH_LANG.TXT");

	SceOff offset = 0;

	SceIoStat stat;
	if (sceIoGetstat(linebuf, &stat) < 0){
		scePaf_strcpy(linebuf, vsh->config.p_ark->arkpath);
		strcat(linebuf, "LANG.ARK");

		unsigned size = 0;
		offset = findPkgOffset(file, &size, linebuf);

		if (size == 0 || offset == 0) return -1;
	}

	fd = sceIoOpen(linebuf, PSP_O_RDONLY, 0);
	sceIoLseek(fd, offset, PSP_SEEK_SET);

	if(fd < 0) {
		return fd;
	}

	read_alloc_buf = vpl_alloc(READ_BUF_SIZE + 64);

	if(read_alloc_buf == NULL) {
		sceIoClose(fd);
		return -1;
	}

	read_buf = (void*)(((u32)read_alloc_buf & (~(64-1))) + 64);
	i = 0;

	while(i < nr_trans) {
		if (read_lines(fd, linebuf, sizeof(linebuf)-1) < 0) {
			break;
		}

//		printf("linebuf %s\n", linebuf);
		set_translate_table_item(table, linebuf, i, nr_trans);
		i++;
	}

	if (i < nr_trans) {
		sceIoClose(fd);
		vpl_free(read_alloc_buf);
		return -1;
	}

	sceIoClose(fd);
	vpl_free(read_alloc_buf);

	return 0;
}

void free_translate_table(char **table, int nr_trans)
{
	int i;

	if(table == NULL)
		return;

	for(i=0; i<nr_trans; ++i) {
		vpl_free(table[i]);
	}

	vpl_free(table);
}

void clear_language(void) {
	if (g_messages != g_messages_en)
		free_translate_table((char**)g_messages, MSG_END);

	g_messages = g_messages_en;
}

char ** apply_language(char *translate_file) {
	char **message = NULL;
	int ret;

	ret = load_translate_table(&message, translate_file, MSG_END);

	if(ret >= 0) {
		return message;
	}

	return (char**) g_messages_en;
}

int cur_language = 0;

void select_language(void) {

	static char *languages[] = { "jp", "en", "fr", "es", "de", "it", "nl", "pt", "ru", "kr", "cht", "chs" };

	int ret, value;
	ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &value);

	if(ret < 0)
		value = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

	cur_language = value;
	clear_language();

	char file[64];
	scePaf_sprintf(file, "satelite_%s.txt", languages[value]);
	g_messages = (const char**)apply_language(file);

	if(g_messages == g_messages_en) {
		cur_language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	}
}
