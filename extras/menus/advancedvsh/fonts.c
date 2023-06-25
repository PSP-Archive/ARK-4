#include "fonts.h"

#include <psputility.h>


char* g_available_fonts[] = {
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
	return (char**)g_available_fonts;
}

int font_load(vsh_Menu *vsh) {
	// if a font is needed (ie not 0)
	if (vsh->config.ark_menu.vsh_font) {
		// load external font
		load_external_font(g_available_fonts[vsh->config.ark_menu.vsh_font - 1]);
		return 0;
	}
	
	int ret, value;
	// get device language
	ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &value);
	
	// if language not found, default to english
	if (ret < 0)
		value = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

	switch (value) {
		case PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN:
			load_external_font("RUSSIAN.pf");
			vsh->config.ark_menu.vsh_font = 48;
			break;
		// use CP881 font for French
		case PSP_SYSTEMPARAM_LANGUAGE_FRENCH:
			load_external_font("CP881.pf");
			vsh->config.ark_menu.vsh_font = 32;
			break;
		default:
			break;
	}

	return 0;
}
