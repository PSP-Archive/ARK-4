	.set noreorder

#include "pspimport.s"

// Build files
// intrafont_XXXX.o

#ifdef F_intrafont_0000
	IMPORT_START "intrafont",0x40090000
#endif
#ifdef F_intrafont_0001
	IMPORT_FUNC  "intrafont",0x43222295,intraFontInit
#endif
#ifdef F_intrafont_0002
	IMPORT_FUNC  "intrafont",0x5490AF57,intraFontShutdown
#endif
#ifdef F_intrafont_0003
	IMPORT_FUNC  "intrafont",0xC1DDA431,intraFontLoad
#endif
#ifdef F_intrafont_0004
	IMPORT_FUNC  "intrafont",0x49542E71,intraFontLoadEx
#endif
#ifdef F_intrafont_0005
	IMPORT_FUNC  "intrafont",0xAAC3056A,intraFontUnload
#endif
#ifdef F_intrafont_0006
	IMPORT_FUNC  "intrafont",0xE70425F0,intraFontActivate
#endif
#ifdef F_intrafont_0007
	IMPORT_FUNC  "intrafont",0x5E7343C2,intraFontSetStyle
#endif
#ifdef F_intrafont_0008
	IMPORT_FUNC  "intrafont",0x88EA2465,intraFontSetEncoding
#endif
#ifdef F_intrafont_0009
	IMPORT_FUNC  "intrafont",0x4280925C,intraFontSetAltFont
#endif
#ifdef F_intrafont_0010
	IMPORT_FUNC  "intrafont",0x208C3DFF,intraFontPrintUCS2
#endif
#ifdef F_intrafont_0011
	IMPORT_FUNC  "intrafont",0x2B027D36,intraFontPrintUCS2Ex
#endif
#ifdef F_intrafont_0012
	IMPORT_FUNC  "intrafont",0xE32D492E,intraFontPrintColumnUCS2
#endif
#ifdef F_intrafont_0013
	IMPORT_FUNC  "intrafont",0xDF9C6F4D,intraFontPrintColumnUCS2Ex
#endif
#ifdef F_intrafont_0014
	IMPORT_FUNC  "intrafont",0x73A00FD5,intraFontPrint
#endif
#ifdef F_intrafont_0015
	IMPORT_FUNC  "intrafont",0xA623353F,intraFontPrintEx
#endif
#ifdef F_intrafont_0016
	IMPORT_FUNC  "intrafont",0xBA1CEDD8,intraFontPrintColumn
#endif
#ifdef F_intrafont_0017
	IMPORT_FUNC  "intrafont",0x60B74159,intraFontPrintColumnEx
#endif
#ifdef F_intrafont_0018
	IMPORT_FUNC  "intrafont",0x2487F1B9,intraFontPrintf
#endif
#ifdef F_intrafont_0019
	IMPORT_FUNC  "intrafont",0xB67F0E98,intraFontMeasureText
#endif
#ifdef F_intrafont_0020
	IMPORT_FUNC  "intrafont",0xB4CC27B4,intraFontMeasureTextEx
#endif
#ifdef F_intrafont_0021
	IMPORT_FUNC  "intrafont",0x3933584A,intraFontMeasureTextUCS2
#endif
#ifdef F_intrafont_0022
	IMPORT_FUNC  "intrafont",0x3A172562,intraFontMeasureTextUCS2Ex
#endif
