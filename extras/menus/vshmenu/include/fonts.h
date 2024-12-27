#ifndef _FONT_H
#define _FONT_H


#include <psputility.h>

#include "vsh.h"


#define FONT_WIDTH 8
#define FONT_HEIGHT 8


typedef struct _font_Data{
	u8 *bitmap;
	SceUID mem_id;
	int width, height;
}font_Data;


char** font_list(void);
font_Data* font_data_pointer(void);

int font_load(vsh_Menu *vsh);
int load_external_font(const char *file);
void release_font(void);


#endif