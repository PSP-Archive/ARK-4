#include "fonts.h"

#include <string.h>

#include "scepaf.h"


extern unsigned char msx[];

font_Data font = {
	.bitmap = (u8*)msx,
	.mem_id = -1,
	.width = FONT_WIDTH,
	.height = FONT_HEIGHT
};


extern SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath);


char* available_fonts[] = {
	"8X8!FONT.pf",
	"8X8#FONT.pf",
	"8X8@FONT.pf",
	"8X8ITAL.pf",
	"SMEGA88.pf",
	"APEAUS.pf",
	"SMVGA88.pf",
	"APLS.pf",
	"SPACE8.pf",
	"Standard.pf",
	"TINYTYPE.pf",
	"FANTASY.pf",
	"THIN8X8.pf",
	"THIN_SS.pf",
	"CP111.pf",
	"CP112.pf",
	"CP113.pf",
	"CP437old.pf",
	"CP437.pf",
	"CP850.pf",
	"CP851.pf",
	"CP852.pf",
	"CP853.pf",
	"CP860.pf",
	"CP861.pf",
	"CP862.pf",
	"CP863.pf",
	"CP864.pf",
	"CP865.pf",
	"CP866.pf",
	"CP880.pf",
	"CP881.pf",
	"CP882.pf",
	"CP883.pf",
	"CP884.pf",
	"CP885.pf",
	"CRAZY8.pf",
	"DEF_8X8.pf",
	"VGA-ROM.pf",
	"EVGA-ALT.pf",
	"FE_8X8.pf",
	"GRCKSSRF.pf",
	"HERCITAL.pf",
	"HERCULES.pf",
	"MAC.pf",
	"MARCIO08.pf",
	"READABLE.pf",
	"ROM8PIX.pf",
	"RUSSIAN.pf",
	"CYRILL1.pf",
	"CYRILL2.pf",
	"CYRILL3.pf",
	"CYRIL_B.pf",
	"ARMENIAN.pf",
	"GREEK.pf",	
};

char** font_list(void) {
	return (char**)available_fonts;
}

font_Data* font_data_pointer(void) {
	return (font_Data*)&font;
}

int font_load(vsh_Menu *vsh) {
	
	int ret, value;
	// get device language
	ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &value);
	
	// if language not found, default to english
	if (ret < 0)
		value = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

	switch (value) {
		case PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN:
			// make sure we use a russian font
			if (vsh->config.ark_menu.vsh_font != 49){
				vsh->config.ark_menu.vsh_font = 49;
			}
			break;
		/*
		// use CP881 font for French
		case PSP_SYSTEMPARAM_LANGUAGE_FRENCH:
			load_external_font("CP881.pf");
			vsh->config.ark_menu.vsh_font = 32;
			break;
		*/
		default:
			break;
	}

	// if a font is needed (ie not 0)
	if (vsh->config.ark_menu.vsh_font) {
		// load external font
		load_external_font(available_fonts[vsh->config.ark_menu.vsh_font - 1]);
	}

	return 0;
}


int load_external_font(const char *file) {
	SceUID fd;
	int ret;
	void *buf;
	unsigned int size = 0;
	
	vsh_Menu *vsh = vsh_menu_pointer();

	if (file == NULL || file[0] == 0) return -1;

	static char pkgpath[ARK_PATH_SIZE];
	scePaf_strcpy(pkgpath, vsh->config.p_ark->arkpath);
	strcat(pkgpath, "LANG.ARK");

	SceOff offset = findPkgOffset(file, &size, pkgpath);

	if (offset == 0) return -1;

	fd = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);

	if(fd < 0) {
		return fd;
	}

	font.mem_id = sceKernelAllocPartitionMemory(2, "proDebugScreenFontBuffer", PSP_SMEM_High, size, NULL);

	if(font.mem_id < 0) {
		sceIoClose(fd);
		return font.mem_id;
	}

	buf = sceKernelGetBlockHeadAddr(font.mem_id);

	if(buf == NULL) {
		sceKernelFreePartitionMemory(font.mem_id);
		sceIoClose(fd);
		return -2;
	}

	sceIoLseek(fd, offset, PSP_SEEK_SET);
	ret = sceIoRead(fd, buf, size);

	if(ret != size) {
		sceKernelFreePartitionMemory(font.mem_id);
		sceIoClose(fd);
		return -3;
	}

	sceIoClose(fd);
	font.bitmap = (u8*)buf;
	return 0;
}

void release_font(void) {
	if (font.mem_id >= 0) {
		sceKernelFreePartitionMemory(font.mem_id);
		font.mem_id = -1;
	}

	font.bitmap = msx;
}