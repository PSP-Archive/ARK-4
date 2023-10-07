#include "color.h"

#include <stdlib.h>
#include <time.h>

#include "scepaf.h"

u32 colors[] = {
	0x000000ff, // Default (random) 0
	0x000000ff, // Red              1
	0xa00000ff, // Light Red        2
	0x0000a5ff, // Orange           3
	0xa000a5ff, // Light Orange     4
	0x0000e6e6, // Yellow           5
	0xa000e6e6, // Light Yellow     6
	0x0000ff00, // Green            7
	0xa000b300, // Light Green      8
	0x00ff0000, // Blue             9
	0xa0ff0000, // Light Blue       10
	0x0082004b, // Indigo           11
	0xa082004b, // Light Indigo     12
	0x00ee82ee, // Violet           13
	0xa0ee82ee, // Light Violet     14
	0x00cbc0ff, // Pink             15
	0xa0cbc0ff, // Light Pink       16
	0x00993366, // Purple           17
	0xa0993366, // Light Purple     18
	0x00808000, // Teal             19
	0xa0808000, // Light Teal       20
	0x00cccc00, // Aqua             21
	0xa0cccc00, // Light Aqua       22
	0x00737373, // Grey             23
	0xa0737373, // Light Grey       24
	0x00000000, // Black            25
	0xa0000000, // Light Black      26
	0x00ffffff, // White            27
	0xafffffff, // Light White      28
};

static struct{
	u8 fg_color;
	u8 bg_color;
} random_colors[] = {
	{1, 11},
	{20, 11},
	{1, 12},
	{7, 12},
	{1, 14},
	{3, 14},
	{5, 14},
	{7, 14},
	{7, 8},
	{21, 8},
	{27, 20},
	{27, 23},
	{27, 24},
	{3, 25},
	{5, 25},
	{1, 25},
	{7, 25},
	{9, 25},
	{11, 25},
	{13, 25},
	{15, 25},
	{17, 25},
	{19, 25},
	{21, 25},
	{23, 25},
	{27, 25},
	{1, 26},
	{27, 26},
	{25, 7}, 
	{27, 3},
	{27, 8},
	{27, 9},
	{27, 11},
	{27, 25},
};


u32* color_data_pointer(void) {
	return (u32*)colors;
}

void color_check_random(vsh_Menu *vsh) {
	int picked;
	// Random Colors
	if ((vsh->config.ark_menu.vsh_fg_color || vsh->config.ark_menu.vsh_bg_color) == 0) {
		srand(time(NULL));
		picked = rand() % (sizeof(random_colors) / sizeof(random_colors[0]));
		vsh->config.ark_menu.vsh_fg_color = random_colors[picked].fg_color;
		vsh->config.ark_menu.vsh_bg_color = random_colors[picked].bg_color;
	}
}
