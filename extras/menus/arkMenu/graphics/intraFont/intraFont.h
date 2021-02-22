/*
 * intraFont.h
 * This file is used to display the PSP's internal font (pgf and bwfon firmware files)
 * intraFont Version 0.31 by BenHur - http://www.psp-programming.com/benhur
 *
 * Uses parts of pgeFont by InsertWittyName - http://insomniac.0x89.org
 * G-spec code by Geecko
 *
 * This work is licensed under the Creative Commons Attribution-Share Alike 3.0 License.
 * See LICENSE for more details.
 *
 */

#ifndef __INTRAFONT_H__
#define __INTRAFONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "libccc.h"

/** @defgroup intraFont Font Library
 *  @{
 */

#define INTRAFONT_ADVANCE_H        0x00000000 //default: advance horizontaly from one char to the next
#define INTRAFONT_ADVANCE_V        0x00000100
#define INTRAFONT_ALIGN_LEFT       0x00000000 //default: left-align the text
#define INTRAFONT_ALIGN_CENTER     0x00000200
#define INTRAFONT_ALIGN_RIGHT      0x00000400
#define INTRAFONT_ALIGN_FULL       0x00000600 //full justify text to width set by intraFontSetTextWidth()
#define INTRAFONT_SCROLL_LEFT      0x00002000 //in intraFontPrintColumn if text does not fit text is scrolled to the left 
                                            //(requires redrawing at ~60 FPS with x position returned by previous call to intraFontPrintColumn())
#define INTRAFONT_SCROLL_SEESAW    0x00002200 //in intraFontPrintColumn if text does not fit text is scrolled left and right
#define INTRAFONT_SCROLL_RIGHT     0x00002400 //in intraFontPrintColumn if text does not fit text is scrolled to the right
#define INTRAFONT_SCROLL_THROUGH   0x00002600 //in intraFontPrintColumn if text does not fit text is scrolled through (to the left)
#define INTRAFONT_WIDTH_VAR        0x00000000 //default: variable-width
#define INTRAFONT_WIDTH_FIX        0x00000800 //set your custom fixed witdh to 24 pixels: INTRAFONT_WIDTH_FIX | 24 
                                              //(max is 255, set to 0 to use default fixed width, this width will be scaled by size)
#define INTRAFONT_ACTIVE           0x00001000 //assumes the font-texture resides inside sceGuTex already, prevents unecessary reloading -> very small speed-gain                     
#define INTRAFONT_CACHE_MED        0x00000000 //default: 256x256 texture (enough to cache about 100 chars)
#define INTRAFONT_CACHE_LARGE      0x00004000 //512x512 texture(enough to cache all chars of ltn0.pgf or ... or ltn15.pgf or kr0.pgf)
#define INTRAFONT_CACHE_ASCII      0x00008000 //try to cache all ASCII chars during fontload (uses less memory and is faster to draw text, but slower to load font)
                                              //if it fails: (because the cache is too small) it will automatically switch to chache on-the-fly with a medium texture
                              //if it succeeds: (all chars and shadows fit into chache) it will free some now unneeded memory
#define INTRAFONT_CACHE_ALL        0x0000C000 //try to cache all chars during fontload (uses less memory and is faster to draw text, but slower to load font)
                                              //if it fails: (because the cache is too small) it will automatically switch to chache on-the-fly with a large texture
                            //if it succeeds: (all chars and shadows fit into chache) it will free some now unneeded memory
#define INTRAFONT_STRING_ASCII     (0x00010000*CCC_CP000)  //default: interpret strings as ascii text (ISO/IEC 8859-1)
#define INTRAFONT_STRING_CP437     (0x00010000*CCC_CP437)  //interpret strings as ascii text (codepage 437)
#define INTRAFONT_STRING_CP850     (0x00010000*CCC_CP850)  //interpret strings as ascii text (codepage 850)
#define INTRAFONT_STRING_CP866     (0x00010000*CCC_CP866)  //interpret strings as ascii text (codepage 866)
#define INTRAFONT_STRING_SJIS      (0x00010000*CCC_CP932)  //interpret strings as shifted-jis (used for japanese)
#define INTRAFONT_STRING_GBK       (0x00010000*CCC_CP936)  //interpret strings as GBK (used for simplified chinese)
#define INTRAFONT_STRING_KOR       (0x00010000*CCC_CP949)  //interpret strings as Korean codepage 949
#define INTRAFONT_STRING_BIG5      (0x00010000*CCC_CP950)  //interpret strings as BIG5 (used for traditional chinese)
#define INTRAFONT_STRING_CP1251    (0x00010000*CCC_CP1251) //interpret strings as ascii text (codepage windows-1251)
#define INTRAFONT_STRING_CP1252    (0x00010000*CCC_CP1252) //interpret strings as ascii text (codepage windows-1252)
#define INTRAFONT_STRING_UTF8      (0x00010000*CCC_CPUTF8) //interpret strings as UTF-8

/** @note The following definitions are used internally by ::intraFont and have no other relevance.*/
#define FILETYPE_PGF      0x00
#define FILETYPE_BWFON    0x01
#define PGF_BMP_H_ROWS    0x01
#define PGF_BMP_V_ROWS    0x02
#define PGF_BMP_OVERLAY   0x03
#define PGF_NO_EXTRA1     0x04
#define PGF_NO_EXTRA2     0x08
#define PGF_NO_EXTRA3     0x10
#define PGF_CHARGLYPH     0x20
#define PGF_SHADOWGLYPH   0x40 //warning: this flag is not contained in the metric header flags and is only provided for simpler call to intraFontGetGlyph - ONLY check with (flags & PGF_CHARGLYPH)
#define PGF_CACHED        0x80
#define PGF_WIDTH_MASK    0x000000FF
#define PGF_OPTIONS_MASK  0x00003FFF
#define PGF_ALIGN_MASK    0x00000600
#define PGF_SCROLL_MASK   0x00002600
#define PGF_CACHE_MASK    0x0000C000
#define PGF_STRING_MASK   0x00FF0000


/**
 * A Glyph struct
 *
 * @note This is used internally by ::intraFont and has no other relevance.
 */
typedef struct {
  unsigned short x;         //in pixels
  unsigned short y;         //in pixels
  unsigned char width;      //in pixels
  unsigned char height;     //in pixels
  char left;                //in pixels
  char top;                 //in pixels
  unsigned char flags;
  unsigned short shadowID;  //to look up in shadowmap
  char advance;             //in quarterpixels
  unsigned long ptr;        //offset 
} Glyph;

typedef struct {
  unsigned short x;         //in pixels
  unsigned short y;         //in pixels
  unsigned char flags;
} GlyphBW;

/**
 * A PGF_Header struct
 *
 * @note This is used internally by ::intraFont and has no other relevance.
 */
typedef struct {
  unsigned short header_start;
  unsigned short header_len;
  char pgf_id[4];
  unsigned long revision;
  unsigned long version;
  unsigned long charmap_len;
  unsigned long charptr_len;
  unsigned long charmap_bpe;
  unsigned long charptr_bpe;
  unsigned char junk00[21];
  unsigned char family[64];
  unsigned char style[64];
  unsigned char junk01[1];
  unsigned short charmap_min;
  unsigned short charmap_max;
  unsigned char junk02[50];
  unsigned long fixedsize[2];
  unsigned char junk03[14];
  unsigned char table1_len;
  unsigned char table2_len;
  unsigned char table3_len;
  unsigned char advance_len;
  unsigned char junk04[102];
  unsigned long shadowmap_len;
  unsigned long shadowmap_bpe;
  unsigned char junk05[4];
  unsigned long shadowscale[2];
  //currently no need ;
} PGF_Header;

/**
 * A Font struct
 */
typedef struct intraFont {
  char* filename;
  unsigned char fileType;          /**< FILETYPE_PGF or FILETYPE_BWFON */
  unsigned char* fontdata;
  
  unsigned char* texture;          /**< The bitmap data */
  unsigned int texWidth;           /**< Texture size (power2) */
  unsigned int texHeight;          /**< Texture height (power2) */  
  unsigned short texX;
  unsigned short texY;
  unsigned short texYSize;
  
  unsigned short n_chars;
  char advancex;                   /**< in quarterpixels */
  char advancey;                   /**< in quarterpixels */
  unsigned char charmap_compr_len; /**< length of compression info */
  unsigned short* charmap_compr;   /**< Compression info on compressed charmap */  
  unsigned short* charmap;         /**< Character map */  
  Glyph* glyph;                    /**< Character glyphs */
  GlyphBW* glyphBW;
    
  unsigned short n_shadows;
  unsigned char shadowscale;       /**< shadows in pgf file (width, height, left and top properties as well) are scaled by factor of (shadowscale>>6) */  
  Glyph* shadowGlyph;              /**<  Shadow glyph(s) */  
  
  float size;
  unsigned int color;
  unsigned int shadowColor;
  float angle, Rsin, Rcos;                /**< For rotation */
  short isRotated;
  unsigned int options;

  struct intraFont* altFont;
} intraFont;


/**
 * Initialise the Font library
 *
 * @returns 1 on success.
 */
int intraFontInit(void);

/**
 * Shutdown the Font library
 */
void intraFontShutdown(void);

/**
 * Load a pgf font.
 *
 * @param filename - Path to the font
 *
 * @param  options - INTRAFONT_XXX flags as defined above including flags related to CACHE (ored together)
 *
 * @returns A ::intraFont struct
 */
intraFont* intraFontLoad(const char *filename,unsigned int options);

/**
 * Free the specified font.
 *
 * @param font - A valid ::intraFont
 */
void intraFontUnload(intraFont *font);

/**
 * Activate the specified font.
 *
 * @param font - A valid ::intraFont
 */
void intraFontActivate(intraFont *font);

/**
 * Set font style
 *
 * @param font - A valid ::intraFont
 *
 * @param size - Text size
 *
 * @param color - Text color
 *
 * @param angle - Text angle (in degrees)
 *
 * @param shadowColor - Shadow color (use 0 for no shadow)
 *
 * @param options - INTRAFONT_XXX flags as defined above except flags related to CACHE (ored together)
 */
void intraFontSetStyle(intraFont *font, float size, unsigned int color, unsigned int shadowColor, float angle, unsigned int options);

/**
 * Set type of string encoding to be used in intraFontPrint[f]
 *
 * @param font - A valid ::intraFont
 *
 * @param options - INTRAFONT_STRING_XXX flags as defined above except flags related to CACHE (ored together)
 */
void intraFontSetEncoding(intraFont *font, unsigned int options);

/**
 * Set alternative font
 *
 * @param font - A valid ::intraFont
 *
 * @param altFont - A valid ::intraFont that's to be used if font does not contain a character
 */
void intraFontSetAltFont(intraFont *font, intraFont *altFont);

/**
 * Draw UCS-2 encoded text along the baseline starting at x, y.
 *
 * @param font - A valid ::intraFont
 *
 * @param x - X position on screen
 *
 * @param y - Y position on screen
 *
 * @param width - column width for automatic line breaking (intraFontPrintColumn... versions only)
 *
 * @param text - UCS-2 encoded text to draw
 *
 * @param length - char length of text to draw (...Ex versions only)
 *
 * @returns The x position after the last char
 */
float intraFontPrintUCS2        (intraFont *font, float x, float y, const unsigned short *text);
float intraFontPrintUCS2Ex      (intraFont *font, float x, float y, const unsigned short *text, int length);
float intraFontPrintColumnUCS2  (intraFont *font, float x, float y, float width, const unsigned short *text);
float intraFontPrintColumnUCS2Ex(intraFont *font, float x, float y, float width, const unsigned short *text, int length);

/**
 * Draw text along the baseline starting at x, y.
 *
 * @param font - A valid ::intraFont
 *
 * @param x - X position on screen
 *
 * @param y - Y position on screen
 *
 * @param width - column width for automatic line breaking (intraFontPrintColumn... versions only)
 *
 * @param text - Text to draw (ASCII & extended ASCII, S-JIS or UTF-8 encoded)
 *
 * @param length - char length of text to draw (...Ex versions only)
 *
 * @returns The x position after the last char
 */
float intraFontPrint        (intraFont *font, float x, float y, const char *text);
float intraFontPrintEx      (intraFont *font, float x, float y, const char *text, int length);
float intraFontPrintColumn  (intraFont *font, float x, float y, float width, const char *text);
float intraFontPrintColumnEx(intraFont *font, float x, float y, float width, const char *text, int length);

/**
 * Draw text along the baseline starting at x, y (with formatting).
 *
 * @param font - A valid ::intraFont
 *
 * @param x - X position on screen
 *
 * @param y - Y position on screen
 *
 * @param width - column width for automatic line breaking (intraFontPrintfColumn... versions only)
 *
 * @param text - Text to draw (ASCII & extended ASCII, S-JIS or UTF-8 encoded)
 *
 * @param length - char length of text to draw (...Ex versions only)
 *
 * @returns The x position after the last char
 */
float intraFontPrintf        (intraFont *font, float x, float y, const char *text, ...);
//the following functions might be implemented in a future version of intraFont
//float intraFontPrintfEx      (intraFont *font, float x, float y, const char *text, int length, ...);
//float intraFontPrintfColumn  (intraFont *font, float x, float y, float width, const char *text, ...);
//float intraFontPrintfColumnEx(intraFont *font, float x, float y, float width, const char *text, int length, ...);

/**
 * Measure a length of text if it were to be drawn
 *
 * @param font - A valid ::intraFont
 *
 * @param text - Text to measure (ASCII & extended ASCII, S-JIS or UTF-8 encoded)
 *
 * @param length - char length of text to measure (...Ex version only)
 *
 * @returns The total width of the text (until the first newline char)
 */
float intraFontMeasureText  (intraFont *font, const char *text);
float intraFontMeasureTextEx(intraFont *font, const char *text, int length);

/**
 * Measure a length of UCS-2 encoded text if it were to be drawn
 *
 * @param font - A valid ::intraFont
 *
 * @param text - UCS-2 encoded text to measure
 *
 * @param length - char length of text to measure (...Ex version only)
 *
 * @returns The total width of the text (until the first newline char)
 */
float intraFontMeasureTextUCS2  (intraFont *font, const unsigned short *text); 
float intraFontMeasureTextUCS2Ex(intraFont *font, const unsigned short *text, int length); 

/** @} */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __INTRAFONT_H__
