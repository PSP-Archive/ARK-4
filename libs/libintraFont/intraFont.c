/*
 * intraFont.c
 * This file is used to display the PSP's internal font (pgf and bwfon firmware files)
 * intraFont Version 0.31 by BenHur - http://www.psp-programming.com/benhur
 *
 * Uses parts of pgeFont by InsertWittyName - http://insomniac.0x89.org
 *
 * This work is licensed under the Creative Commons Attribution-Share Alike 3.0 License.
 * See LICENSE for more details.
 *
 */

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "intraFont.h"


static unsigned int __attribute__((aligned(16))) clut[16];


unsigned long intraFontGetV(unsigned long n, unsigned char *p, unsigned long *b) {
	unsigned long i,v=0;
	for(i=0;i<n;i++) {
	    v += ( ( p[(*b)/8] >> ((*b)%8) ) & 1) << i;
		(*b)++;
	}
	return v;
}

unsigned long* intraFontGetTable(FILE *file, unsigned long n_elements, unsigned long bp_element) {
	unsigned long len_table = ((n_elements*bp_element+31)/32)*4;
	unsigned char *raw_table = (unsigned char*)malloc(len_table*sizeof(unsigned char));
	if (raw_table == NULL) return NULL;
	if (fread(raw_table, len_table*sizeof(unsigned char), 1, file) != 1) {
		free(raw_table);
		return NULL;
	}
	unsigned long *table = (unsigned long*)malloc(n_elements*sizeof(unsigned long));
	if (table == NULL) {
		free(raw_table);
		return NULL;
	}
	unsigned long i,j=0;
	for (i=0;i<n_elements;i++) {
		table[i] = intraFontGetV(bp_element,raw_table,&j);
	}
	free(raw_table);
	return table;
}

int intraFontGetBMP(intraFont *font, unsigned short id, unsigned char glyphtype) {
	if (!font) return 0;
	if (font->options & INTRAFONT_CACHE_ASCII) return 0; //swizzeled texture
	
	Glyph *glyph;
	if (font->fileType == FILETYPE_PGF) {
		if (glyphtype & PGF_CHARGLYPH) {
			glyph = &(font->glyph[id]);
		} else {
			glyph = &(font->shadowGlyph[id]);
		}
	} else { //FILETYPE_BWFON
		if (glyphtype & PGF_CHARGLYPH) {
			glyph = &(font->glyph[0]);
			glyph->flags = font->glyphBW[id].flags | PGF_BMP_H_ROWS;
		} else {
			glyph = &(font->shadowGlyph[0]);
		}
		glyph->ptr = ((unsigned long)id)*36; //36 bytes/char		
	}
		
	if (glyph->flags & PGF_CACHED) return 1;
	unsigned long b = glyph->ptr*8;
	
	if (glyph->width > 0 && glyph->height > 0) {
		if (!(glyph->flags & PGF_BMP_H_ROWS) != !(glyph->flags & PGF_BMP_V_ROWS)) { //H_ROWS xor V_ROWS (real glyph, no overlay)
			if ((font->texX + glyph->width + 1) > font->texWidth) {
				font->texY += font->texYSize + 1;
				font->texX = 1;
			}
			if ((font->texY + glyph->height + 1) > font->texHeight) {
				font->texY = 1;
				font->texX = 1;
			}
			glyph->x=font->texX;
			glyph->y=font->texY;
            
			//draw bmp
			int i=0,j,xx,yy;
			unsigned char nibble, value = 0;
			if (font->fileType == FILETYPE_PGF) { //for compressed pgf format
				while (i<(glyph->width*glyph->height)) {
					nibble = intraFontGetV(4,font->fontdata,&b);
					if (nibble < 8) value = intraFontGetV(4,font->fontdata,&b);
					for (j=0; (j<=((nibble<8)?(nibble):(15-nibble))) && (i<(glyph->width*glyph->height)); j++) {
						if (nibble >= 8) value = intraFontGetV(4,font->fontdata,&b);
						if (glyph->flags & PGF_BMP_H_ROWS) {
							xx = i%glyph->width;
							yy = i/glyph->width;
						} else {
							xx = i/glyph->height;
							yy = i%glyph->height;
						}
						if ((font->texX + xx) & 1) {
							font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] &= 0x0F;
							font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] |= (value<<4);
						} else {
							font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] &= 0xF0;
							font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] |= (value);
						}
						i++;
					}				
				}
			} else {                                  //for uncompressed bwfon format (could use some optimizations...)
				for (yy = 0; yy < glyph->height; yy++) {
					for (xx = 0; xx < glyph->width; xx++) {
						if (glyphtype & PGF_CHARGLYPH) {
							value = intraFontGetV(1,font->fontdata,&b) * 0x0f; //scale 1 bit/pix to 4 bit/pix
							/* Simple anti-aliasing/blur for black pixels. Unfortunately, does not improve the result...
							if ((value == 0) && (xx > 0) && (yy > 0) && (xx < (glyph->width-1)) && (yy < (glyph->height-1))) {
								b -= 19;
								value += intraFontGetV(1,font->fontdata,&b);
								value += intraFontGetV(1,font->fontdata,&b);
								value += intraFontGetV(1,font->fontdata,&b);
								b += 13;
								value += intraFontGetV(1,font->fontdata,&b);
								value += intraFontGetV(1,font->fontdata,&b);
								value += intraFontGetV(1,font->fontdata,&b);
								b += 13;
								value += intraFontGetV(1,font->fontdata,&b);
								value += intraFontGetV(1,font->fontdata,&b);
								value += intraFontGetV(1,font->fontdata,&b);
								b -= 16;								
							} */
							if ((font->texX + (7-(xx&7)+(xx&248))) & 1) {
								font->texture[((font->texX + (7-(xx&7)+(xx&248))) + (font->texY + yy) * font->texWidth)>>1] &= 0x0F;
								font->texture[((font->texX + (7-(xx&7)+(xx&248))) + (font->texY + yy) * font->texWidth)>>1] |= (value<<4);
							} else {
								font->texture[((font->texX + (7-(xx&7)+(xx&248))) + (font->texY + yy) * font->texWidth)>>1] &= 0xF0;
								font->texture[((font->texX + (7-(xx&7)+(xx&248))) + (font->texY + yy) * font->texWidth)>>1] |= (value);
							}
						} else { //PGF_SHADOWGLYPH
							value = intraFontGetV(4,font->fontdata,&b);
							if ((font->texX + xx) & 1) {
								font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] &= 0x0F;
								font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] |= (value<<4);
							} else {
								font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] &= 0xF0;
								font->texture[((font->texX + xx) + (font->texY + yy) * font->texWidth)>>1] |= (value);
							}
						}
						
					}
				}
			}						

            //erase border around glyph
            for (i = font->texX/2; i < (font->texX+glyph->width+1)/2; i++) {
                font->texture[i + (font->texY-1)*font->texWidth/2] = 0;
                font->texture[i + (font->texY+glyph->height)*font->texWidth/2] = 0;
            }
            for (i = font->texY-1; i < (font->texY+glyph->height+1); i++) {
                font->texture[((font->texX-1) + (i*font->texWidth))>>1] &= (font->texX & 1) ? 0xF0 : 0x0F;
                font->texture[((font->texX+glyph->width) + (i*font->texWidth))>>1] &= ((font->texX+glyph->width) & 1) ? 0x0F : 0xF0;
            }
			font->texX += glyph->width+1; //add empty gap to prevent interpolation artifacts from showing
			
			//mark dirty glyphs as uncached
			if (font->fileType == FILETYPE_PGF) { //...for PGF glyphs
				for (i = 0; i < font->n_chars; i++) {
					if ( (font->glyph[i].flags & PGF_CACHED) && (font->glyph[i].y == glyph->y) ) {
						if ( (font->glyph[i].x+font->glyph[i].width+1) > glyph->x && font->glyph[i].x < (glyph->x+glyph->width+1) ) {
							font->glyph[i].flags -= PGF_CACHED;
						}
					}
				}
			} else {                               //...for BWFON glyphs
				for (i = 0; i < font->n_chars; i++) {
					if ( (font->glyphBW[i].flags & PGF_CACHED) && (font->glyphBW[i].y == glyph->y) ) {
						if ( (font->glyphBW[i].x+font->glyph[0].width+1) > glyph->x && font->glyphBW[i].x < (glyph->x+glyph->width+1) ) {
							font->glyphBW[i].flags -= PGF_CACHED;
						}
					}
				}				
			}
			for (i = 0; i < font->n_shadows; i++) {
				if ( (font->shadowGlyph[i].flags & PGF_CACHED) && (font->shadowGlyph[i].y == glyph->y) ) {
					if ( (font->shadowGlyph[i].x+font->shadowGlyph[i].width+1) > glyph->x && font->shadowGlyph[i].x < (glyph->x+glyph->width+1) ) {
						font->shadowGlyph[i].flags -= PGF_CACHED;
					}
				}
			}

		} else return 0; //transposition=0 or overlay glyph
	} else {
		glyph->x=0;
		glyph->y=0;
	}
	glyph->flags |= PGF_CACHED;

	if ((font->fileType == FILETYPE_BWFON) && (glyphtype & PGF_CHARGLYPH)) { //write properties back to BW-glyphs
		font->glyphBW[id].x = glyph->x;
		font->glyphBW[id].y = glyph->y;
		font->glyphBW[id].flags = glyph->flags;
	}

	return 1; //texture has changed
}

int intraFontGetGlyph(unsigned char *data, unsigned long *b, unsigned char glyphtype, signed long *advancemap, Glyph *glyph) {
    if (glyphtype & PGF_CHARGLYPH) {
        (*b) += 14; //skip offset pos value of shadow
    } else {
        (*b) += intraFontGetV(14,data,b)*8+14; //skip to shadow 
    }
    glyph->width=intraFontGetV(7,data,b);
	glyph->height=intraFontGetV(7,data,b);
	glyph->left=intraFontGetV(7,data,b);
	if (glyph->left >= 64) glyph->left -= 128;
	glyph->top=intraFontGetV(7,data,b);
	if (glyph->top >= 64) glyph->top -= 128;
    glyph->flags = intraFontGetV(6,data,b);
    if (glyph->flags & PGF_CHARGLYPH) {
        (*b) += 7; //skip magic number
		glyph->shadowID = intraFontGetV(9,data,b);
		(*b) += 24 + ((glyph->flags & PGF_NO_EXTRA1)?0:56) + ((glyph->flags & PGF_NO_EXTRA2)?0:56) + ((glyph->flags & PGF_NO_EXTRA3)?0:56); //skip to 6bits before end of header
		glyph->advance = advancemap[intraFontGetV(8,data,b)*2]/16;
    } else {
		glyph->shadowID = 65535;
        glyph->advance = 0;
    }    
	glyph->ptr = (*b)/8;
    return 1;
}

unsigned short intraFontGetID(intraFont* font, cccUCS2 ucs) {
	unsigned short j, id = 0;
	char found = 0;
	for (j = 0; j < font->charmap_compr_len && !found; j++) {
		if ((ucs >= font->charmap_compr[j*2]) && (ucs < (font->charmap_compr[j*2]+font->charmap_compr[j*2+1]))) {
			id += ucs - font->charmap_compr[j*2];
			found = 1;			
		} else {
			id += font->charmap_compr[j*2+1];
		}
	}
	if (!found) return 65535;	//char not in charmap
	if (font->fileType == FILETYPE_PGF) id = font->charmap[id]; //BWFON has right id already
	if (id >= font->n_chars) return 65535; //char not in fontdata or not in ASCII-cache
	//if (font->glyph[id].width == 0 || font->glyph[id].height == 0) return 65535; //char has no valid glyph
	return id;
}

static int intraFontSwizzle(intraFont *font) {
	int height = font->texHeight;
	int byteWidth = font->texWidth>>1;
	int textureSize = byteWidth*height;

	int rowBlocks = (byteWidth>>4);
	int rowBlocksAdd = (rowBlocks - 1)<<7;
	unsigned int blockAddress = 0;
	unsigned int *src = (unsigned int*) font->texture;
	static unsigned char *tData;

	tData = (unsigned char*) memalign(16,textureSize);
	if(!tData) return 0;

	int i,j;
	for(j = 0; j < height; j++, blockAddress += 16) {
		unsigned int *block = ((unsigned int*)&tData[blockAddress]);
		for(i=0; i < rowBlocks; i++) {
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			block += 28;
		}
		if((j & 0x7) == 0x7) blockAddress += rowBlocksAdd;
	}

	free(font->texture);
	font->texture = tData;
	font->options |= INTRAFONT_CACHE_ASCII;

	return 1;
}

int intraFontPreCache(intraFont *font, unsigned int options) {
	if (!font) return 0;
	if (!(options & INTRAFONT_CACHE_ASCII)) return 0; //no precaching requested
	if (font->options & INTRAFONT_CACHE_ASCII) return 0; //already prechached?
		
    //cache all glyphs
	int i,y;
	font->texX = 1;
	font->texY = 1;
	font->texYSize = 0;
	for (i = 0; i < font->n_chars; i++) {
		y = font->texY;
		intraFontGetBMP(font,i,PGF_CHARGLYPH);		
		if (font->texY > y || font->texYSize < font->glyph[i].height) font->texYSize = font->glyph[i].height; //minimize ysize after newline in cache(only valid for precached glyphs)
		if (font->texY < y) return 0;                               //char did not fit into cache -> abort precache (should reset cache and glyph.flags)
	}
	for (i=0;i<font->n_shadows;i++) {
		y = font->texY;
		intraFontGetBMP(font,i,PGF_SHADOWGLYPH);
		if (font->texY > y || font->texYSize < font->shadowGlyph[i].height) font->texYSize = font->shadowGlyph[i].height; //minimize ysize
		if (font->texY < y) return 0;                               //char did not fit into cache -> abort precache (should reset cache and glyph.flags)
	}
	font->texHeight = ((font->texY) + (font->texYSize) + 7)&~7;     
	if (font->texHeight > font->texWidth) font->texHeight = font->texWidth;
	
	//reduce fontdata
	int index = 0, j;
	for (i = 0; i < font->n_chars; i++) {
		if ((font->glyph[i].flags & PGF_BMP_H_ROWS) && (font->glyph[i].flags & PGF_BMP_V_ROWS)) {
			for (j = 0; j < 6; j++, index++) {
				font->fontdata[index] = font->fontdata[(font->glyph[i].ptr)+j];
			}
			font->glyph[i].ptr = index - j;
		}
	}
	if (index == 0)	{
		free(font->fontdata); 
		font->fontdata = NULL;
	} else {
		font->fontdata = (unsigned char*)realloc(font->fontdata,index*sizeof(unsigned char));		
	}

	//swizzle texture
	sceKernelDcacheWritebackAll();
	intraFontSwizzle(font);
	sceKernelDcacheWritebackAll();
	
	return 1;
}

intraFont* intraFontLoad(const char *filename, unsigned int options) {
    unsigned long i,j;
	static Glyph bw_glyph                         = { 0, 0, 16, 18, 0, 15, PGF_BMP_H_ROWS, 0, 64, 0 };
	static Glyph bw_shadowGlyph                   = { 0, 0,  8, 10, 0,  5, PGF_BMP_H_ROWS, 0, 64, 0 };
	static const unsigned short bw_charmap_compr[]= { 0x00a4,  1, 0x00a7,  2, 0x00b0,  2, 0x00b7,  1, 0x00d7,  1, 0x00e0,  2, 0x00e8,  3, 0x00ec,  2,
		                                              0x00f2,  2, 0x00f7,  1, 0x00f9,  2, 0x00fc,  1, 0x0101,  1, 0x0113,  1, 0x011b,  1, 0x012b,  1, 
													  0x0144,  1, 0x0148,  1, 0x014d,  1, 0x016b,  1, 0x01ce,  1, 0x01d0,  1, 0x01d2,  1, 0x01d4,  1, 
													  0x01d6,  1, 0x01d8,  1, 0x01da,  1, 0x01dc,  1, 0x0251,  1, 0x0261,  1, 0x02c7,  1, 0x02c9,  3, 
													  0x02d9,  1, 0x0391, 17, 0x03a3,  7, 0x03b1, 17, 0x03c3,  7, 0x0401,  1, 0x0410, 64, 0x0451,  1,
													  0x2010,  1, 0x2013,  4, 0x2018,  2, 0x201c,  2, 0x2025,  2, 0x2030,  1, 0x2032,  2, 0x2035,  1,
													  0x203b,  1, 0x20ac,  1, 0x2103,  1, 0x2105,  1, 0x2109,  1, 0x2116,  1, 0x2121,  1, 0x2160, 12, 
													  0x2170, 10, 0x2190,  4, 0x2196,  4, 0x2208,  1, 0x220f,  1, 0x2211,  1, 0x2215,  1, 0x221a,  1, 
													  0x221d,  4, 0x2223,  1, 0x2225,  1, 0x2227,  5, 0x222e,  1, 0x2234,  4, 0x223d,  1, 0x2248,  1, 
													  0x224c,  1, 0x2252,  1, 0x2260,  2, 0x2264,  4, 0x226e,  2, 0x2295,  1, 0x2299,  1, 0x22a5,  1, 
													  0x22bf,  1, 0x2312,  1, 0x2460, 10, 0x2474, 40, 0x2500, 76, 0x2550, 36, 0x2581, 15, 0x2593,  3, 
													  0x25a0,  2, 0x25b2,  2, 0x25bc,  2, 0x25c6,  2, 0x25cb,  1, 0x25ce,  2, 0x25e2,  4, 0x2605,  2, 
													  0x2609,  1, 0x2640,  1, 0x2642,  1, 0x2e81,  1, 0x2e84,  1, 0x2e88,  1, 0x2e8b,  2, 0x2e97,  1, 
													  0x2ea7,  1, 0x2eaa,  1, 0x2eae,  1, 0x2eb3,  1, 0x2eb6,  2, 0x2ebb,  1, 0x2eca,  1, 0x2ff0, 12, 
													  0x3000,  4, 0x3005, 19, 0x301d,  2, 0x3021,  9, 0x303e,  1, 0x3041, 83, 0x309b,  4, 0x30a1, 86,
													  0x30fc,  3, 0x3105, 37, 0x3220, 10, 0x3231,  1, 0x32a3,  1, 0x338e,  2, 0x339c,  3, 0x33a1,  1, 
													  0x33c4,  1, 0x33ce,  1, 0x33d1,  2, 0x33d5,  1,0x3400,6582,0x4e00,20902,0xe78d, 10, 0xe7c7,  2,
													  0xe816,  3, 0xe81e,  1, 0xe826,  1, 0xe82b,  2, 0xe831,  2, 0xe83b,  1, 0xe843,  1, 0xe854,  2, 
													  0xe864,  1, 0xf92c,  1, 0xf979,  1, 0xf995,  1, 0xf9e7,  1, 0xf9f1,  1, 0xfa0c,  4, 0xfa11,  1, 
													  0xfa13,  2, 0xfa18,  1, 0xfa1f,  3, 0xfa23,  2, 0xfa27,  3, 0xfe30,  2, 0xfe33, 18, 0xfe49, 10, 
													  0xfe54,  4, 0xfe59, 14, 0xfe68,  4, 0xff01, 94, 0xffe0,  6 };
	static const unsigned char bw_shadow[]        = { 0x10, 0x11, 0x11, 0x01, 0x10, 0x22, 0x22, 0x01, 0x21, 0x43, 0x34, 0x12, 0x31, 0x75, 0x57, 0x13, 
		                                              0x31, 0x86, 0x68, 0x13, 0x31, 0x86, 0x68, 0x13, 0x31, 0x75, 0x57, 0x13, 0x21, 0x43, 0x34, 0x12, 
													  0x10, 0x22, 0x22, 0x01, 0x10, 0x11, 0x11, 0x01 };
	
	//create font structure
	intraFont* font = (intraFont*)malloc(sizeof(intraFont));
	if (!font) return NULL;
	
	//open pgf file and get file size
    FILE *file = fopen(filename, "rb"); /* read from the file in binary mode */
	if (!file) return NULL;
	fseek(file, 0, SEEK_END);
    unsigned long filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	//read pgf header
	static PGF_Header header;
	if (fread(&header, sizeof(PGF_Header), 1, file) != 1) {
		fclose(file);
		return NULL;
	}
	
	//pgf header tests: valid and known pgf file?
	if ((strncmp(header.pgf_id,"PGF0",4) == 0) && (header.version == 6) && (header.revision == 2 || header.revision == 3) &&
		(header.charmap_len <= 65535) && (header.charptr_len <= 65535) && (header.charmap_bpe <= 32) && (header.charptr_bpe <= 32) &&
		(header.charmap_min <= header.charmap_max) && (header.shadowmap_len <= 511) && (header.shadowmap_bpe <= 16)) {
		font->fileType = FILETYPE_PGF;
	} else if (filesize == 1023372) { //only known size for bwfon -> refuse to work for modified sizes
		font->fileType = FILETYPE_BWFON;
	} else {
	    fclose(file);
		return NULL;
	}
	
	//intitialize font structure
	if (font->fileType == FILETYPE_PGF) {
		font->n_chars = (unsigned short)header.charptr_len;
		font->charmap_compr_len = (header.revision == 3)?7:1;
		font->texYSize = 0;	
		font->advancex = header.fixedsize[0]/16;
		font->advancey = header.fixedsize[1]/16;
		font->n_shadows = (unsigned short)header.shadowmap_len;
		font->shadowscale = (unsigned char)header.shadowscale[0];
		font->glyph = (Glyph*)malloc(font->n_chars*sizeof(Glyph));
		font->glyphBW = NULL;
		font->shadowGlyph = (Glyph*)malloc(font->n_shadows*sizeof(Glyph));
		font->charmap_compr = (unsigned short*)malloc(font->charmap_compr_len*sizeof(unsigned short)*2);
		font->charmap = (unsigned short*)malloc(header.charmap_len*sizeof(unsigned short));
		if (!font->glyph || !font->shadowGlyph || !font->charmap_compr || !font->charmap) {
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}
		memset(font->glyph, 0, font->n_chars*sizeof(Glyph)); //make sure glyphs are initialized with 0
		memset(font->shadowGlyph, 0, font->n_shadows*sizeof(Glyph));
	} else { //FILETYPE_BWFON
		font->n_chars = filesize/36; //36 bytes/char
		font->charmap_compr_len = 165; //entries in the compressed charmap
		font->texYSize = 18;	
		font->advancex = 16*4;
		font->advancey = 18*4;
		font->n_shadows = 1;
		font->shadowscale = 24;
		font->glyph = &bw_glyph;
		font->glyph[0].shadowID = font->n_chars; //shadow is appended
		font->glyphBW = (GlyphBW*)malloc(font->n_chars*sizeof(GlyphBW));
		font->shadowGlyph = &bw_shadowGlyph;
		font->charmap_compr = (unsigned short*)bw_charmap_compr; //static for bwfon
		font->charmap = NULL;                //not needed for bwfon
		if (!font->glyphBW) {
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}	
		memset(font->glyphBW, 0, font->n_chars*sizeof(GlyphBW)); //make sure glyphBWs are initialized with 0
	}	
	font->texWidth = (options & INTRAFONT_CACHE_LARGE) ? (512) : (256);
	font->texHeight = font->texWidth;
	font->texX = 1;
	font->texY = 1;
    font->options = options; 
	if ((options & INTRAFONT_CACHE_ASCII) ) font->options -= INTRAFONT_CACHE_ASCII; //pre-cached texture enabled at the end of font-load
    font->size = 1.0f;               //default size
    font->color = 0xFFFFFFFF;        //non-transparent white
    font->shadowColor = 0xFF000000;  //non-transparent black
	font->altFont = NULL;            //no alternative font
	font->filename = (char*)malloc((strlen(filename)+1)*sizeof(char));
	font->texture = (unsigned char*)memalign(16,font->texWidth*font->texHeight>>1);
	if (!font->filename || !font->texture) {
		fclose(file);
		intraFontUnload(font);
		return NULL;
	}
	strcpy(font->filename,filename);
	
	//read in file (tables / fontdata) and populate font structure
	if (font->fileType == FILETYPE_PGF) {

		//read advance table
		fseek(file, header.header_len+(header.table1_len+header.table2_len+header.table3_len)*8, SEEK_SET);
		signed long *advancemap = (signed long*)malloc(header.advance_len*sizeof(signed long)*2);
		if (!advancemap) {
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}	
		if (fread(advancemap, header.advance_len*sizeof(signed long)*2, 1, file) != 1) {
			free(advancemap);
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}	
	
		//read shadowmap
		unsigned long *ucs_shadowmap = intraFontGetTable(file, header.shadowmap_len, header.shadowmap_bpe);
		if (ucs_shadowmap == NULL) {
			free(advancemap);	
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}
	
		//version 6.3 charmap compression
		if (header.revision == 3) {
			if (fread(font->charmap_compr, font->charmap_compr_len*sizeof(unsigned short)*2, 1, file) != 1) {
				free(advancemap);		
				free(ucs_shadowmap);
				fclose(file);
				intraFontUnload(font);
				return NULL;
			}
		} else {
			font->charmap_compr[0] = header.charmap_min;
			font->charmap_compr[1] = header.charmap_len;
		}
	
		//read charmap
		if (header.charmap_bpe == 16) { //read directly from file...
			if (fread(font->charmap, header.charmap_len*sizeof(unsigned short), 1, file) != 1) {
				free(advancemap);	
				free(ucs_shadowmap);
				fclose(file);
				intraFontUnload(font);
				return NULL;
			}
		} else {
			unsigned long *id_charmap = intraFontGetTable(file, header.charmap_len, header.charmap_bpe);
			if (id_charmap == NULL) {
				free(advancemap);	
				free(ucs_shadowmap);
				fclose(file);
				intraFontUnload(font);
				return NULL;
			}
			for (i=0;i<header.charmap_len;i++) {
				font->charmap[i]=(unsigned short)((id_charmap[i] < font->n_chars)?id_charmap[i]:65535);
			}
			free(id_charmap);
		}
	
		//read charptr
		unsigned long *charptr = intraFontGetTable(file, header.charptr_len, header.charptr_bpe);
		if (charptr == NULL) {
			free(advancemap);	
			free(ucs_shadowmap);
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}

		//read raw fontdata
		unsigned long start_fontdata = ftell(file);
		unsigned long len_fontdata = filesize-start_fontdata;
		font->fontdata = (unsigned char*)malloc(len_fontdata*sizeof(unsigned char));
		if (font->fontdata == NULL) {
			free(advancemap);	
		    free(ucs_shadowmap);
			free(charptr);
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}
		if (fread(font->fontdata, len_fontdata*sizeof(unsigned char), 1, file) != 1) {
			free(advancemap);	
			free(ucs_shadowmap);
			free(charptr);
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}

		//close file	
		fclose(file);
	
		//count ascii chars and reduce mem required
		if ((options & PGF_CACHE_MASK) == INTRAFONT_CACHE_ASCII) {
			font->n_chars = 1; //assume there's at least one char (needed for intraFontGetID to work properly)
			for (i=0; i<128; i++) {
				if (intraFontGetID(font,i) < 65535) (font->n_chars)++;
			}
			(font->n_chars)--; //correct the above assumption
			if (font->n_chars == 0) { //no ASCII chars in fontfile -> abort
				free(advancemap);			
				free(ucs_shadowmap);
				free(charptr);
				intraFontUnload(font);
				return NULL;
			}
			font->glyph = (Glyph*)realloc(font->glyph,font->n_chars*sizeof(Glyph));
			font->charmap_compr[1] = 128 - font->charmap_compr[0];
			font->charmap = (unsigned short*)realloc(font->charmap,font->charmap_compr[1]*sizeof(unsigned short)); 
		}
	
		//populate chars and count space used in cache to prebuffer all chars
		int x=1,y=1,ysize=0;
		for (i = 0; i < font->n_chars; i++) {
			j = charptr[i]*4*8;
			if (!intraFontGetGlyph(font->fontdata, &j, PGF_CHARGLYPH, advancemap, &(font->glyph[i]))) {
				free(advancemap);			
				free(ucs_shadowmap);
				free(charptr);
				intraFontUnload(font);
				return NULL;
			}
			if (!(font->glyph[i].flags & PGF_BMP_H_ROWS) != !(font->glyph[i].flags & PGF_BMP_V_ROWS)) { //H_ROWS xor V_ROWS (real glyph, not overlay)
				if (font->glyph[i].height > font->texYSize) font->texYSize = font->glyph[i].height;     //find max glyph height			
				if ((x + font->glyph[i].width) > font->texWidth) {                             
					y += ysize+1;
					x = 1;
					ysize = 0;
				} 
				if (font->glyph[i].height > ysize) ysize = font->glyph[i].height;
				x += font->glyph[i].width+1;
			}		
		} 
	
		//populate shadows and count space (only for chars needed)
		unsigned short char_id, shadow_id = 0;
		for (i = 0; i < font->n_chars; i++) {
			shadow_id = font->glyph[i].shadowID;
			char_id = intraFontGetID(font,ucs_shadowmap[shadow_id]);
			if (char_id < font->n_chars && font->shadowGlyph[shadow_id].shadowID == 0) { //valid char and shadow glyph not yet loaded
				j = charptr[char_id]*4*8;
				if (!intraFontGetGlyph(font->fontdata, &j, PGF_SHADOWGLYPH, NULL, &(font->shadowGlyph[shadow_id]))) {
					free(advancemap);			
					free(ucs_shadowmap);
					free(charptr);
					intraFontUnload(font);
					return NULL;
				}			
				if (!(font->shadowGlyph[shadow_id].flags & PGF_BMP_H_ROWS) != !(font->shadowGlyph[shadow_id].flags & PGF_BMP_V_ROWS)) { //H_ROWS xor V_ROWS (real glyph, not overlay)
					if (font->shadowGlyph[shadow_id].height > font->texYSize) font->texYSize = font->shadowGlyph[shadow_id].height;     //find max glyph height
					if ((x + font->shadowGlyph[shadow_id].width) > font->texWidth) {                          
						y += ysize+1;
						x = 1;
						ysize = 0;
					} 
					if (font->shadowGlyph[shadow_id].height > ysize) ysize = font->shadowGlyph[shadow_id].height;
					x += font->shadowGlyph[shadow_id].width+1;
				}
			}		              
		}
	
		//free temp stuff
		free(advancemap);
		free(ucs_shadowmap);
		free(charptr);
	
		//cache chars, swizzle texture and free unneeded stuff (if INTRAFONT_CACHE_ALL or _ASCII)
		sceKernelDcacheWritebackAll();
		if ( (options & INTRAFONT_CACHE_ASCII) && ((y + ysize + 1) <= font->texHeight) ) { //cache all and does it fit into cache?
			if (!intraFontPreCache(font,options)) {
				intraFontUnload(font);
				return NULL;			
			}
		}
		
	} else { //FILETYPE_BWFON

		//read raw fontdata
		fseek(file, 0, SEEK_SET);
		font->fontdata = (unsigned char*)malloc((filesize+40)*sizeof(unsigned char));
		if (font->fontdata == NULL) {
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}
		if (fread(font->fontdata, filesize*sizeof(unsigned char), 1, file) != 1) {
			fclose(file);
			intraFontUnload(font);
			return NULL;
		}
		memcpy(font->fontdata+filesize,bw_shadow,40);

		//close file	
		fclose(file);

		//count ascii chars and reduce mem required: no ascii chars in bwfon -> abort
		if ((options & PGF_CACHE_MASK) == INTRAFONT_CACHE_ASCII) {
			intraFontUnload(font);
			return NULL;
		}	
	
		//populate chars and count space used in cache to prebuffer all chars: not needed/available -> skip
		//populate shadows and count space (only for chars needed): not needed/available -> skip
		//free temp stuff: not needed -> skip
		//cache chars, swizzle texture and free unneeded stuff (if INTRAFONT_CACHE_ALL or _ASCII): not available ->skip

	}

	sceKernelDcacheWritebackAll();

	return font;
}

void intraFontUnload(intraFont *font) {
	if (!font) return;
    if (font->filename) free(font->filename);
    if (font->fontdata) free(font->fontdata);
	if (font->texture) free(font->texture);
	if (font->fileType == FILETYPE_PGF) {
		if (font->charmap_compr) free(font->charmap_compr);
		if (font->charmap) free(font->charmap);
		if (font->glyph) free(font->glyph);
		if (font->shadowGlyph) free(font->shadowGlyph);
	} else { //FILETYPE_BWFON
		if (font->glyphBW) free(font->glyphBW);
	}
	if (font) free(font);
	font = NULL;
}

int intraFontInit(void) {
	int n;
	for(n = 0; n < 16; n++)
		clut[n] = ((n * 17) << 24) | 0xffffff;
	return 1;
}

void intraFontShutdown(void) {
	//Nothing yet
}

void intraFontActivate(intraFont *font) {
	if(!font) return;
	if(!font->texture) return;

	sceGuClutMode(GU_PSM_8888, 0, 255, 0);
	sceGuClutLoad(2, clut);

	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_T4, 0, 0, (font->options & INTRAFONT_CACHE_ASCII) ? 1 : 0);
	sceGuTexImage(0, font->texWidth, font->texWidth, font->texWidth, font->texture);
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
	sceGuTexEnvColor(0x0);
	sceGuTexOffset(0.0f, 0.0f);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
}

void intraFontSetStyle(intraFont *font, float size, unsigned int color, unsigned int shadowColor, unsigned int options) {
	if (!font) return;
	font->size = size;
	font->color = color;
	font->shadowColor = shadowColor;
	font->options = (options & PGF_OPTIONS_MASK) | (font->options & PGF_STRING_MASK) | (font->options & PGF_CACHE_MASK);
	if ((font->options & PGF_WIDTH_MASK) == 0) font->options |= ((font->advancex / 8) & PGF_WIDTH_MASK);
}

void intraFontSetEncoding(intraFont *font, unsigned int options) {
    if (!font) return;
	font->options = (font->options & PGF_OPTIONS_MASK) | (options & PGF_STRING_MASK) | (font->options & PGF_CACHE_MASK);
}

void intraFontSetAltFont(intraFont *font, intraFont *altFont) {
	if (!font) return;
	intraFont* nextFont;
	nextFont = altFont;
	while (nextFont) { 
		if ((nextFont->altFont) == font) return; //it must not point at itself
		nextFont = nextFont->altFont;
	}
	font->altFont = altFont; 
}	

float intraFontPrintf(intraFont *font, float x, float y, const char *text, ...) {
	if(!font) return x;

	char buffer[256];
	va_list ap;
	
	va_start(ap, text);
	vsnprintf(buffer, 256, text, ap);
	va_end(ap);
	buffer[255] = 0;
	
	return intraFontPrint(font, x, y, buffer);
}

float intraFontPrint(intraFont *font, float x, float y, const char *text) {
	if (!font) return x;
	int length = cccStrlenCode((cccCode*)text, font->options/0x00010000);
	return intraFontPrintColumnEx(font, x, y, 0.0f, text, length);
}

float intraFontPrintEx(intraFont *font, float x, float y, const char *text, int length) {
	return intraFontPrintColumnEx(font, x, y, 0.0f, text, length);
}

float intraFontPrintColumn(intraFont *font, float x, float y, float column, const char *text) {
	if (!font) return x;
	int length = cccStrlenCode((cccCode*)text, font->options/0x00010000);
	return intraFontPrintColumnEx(font, x, y, column, text, length);
}

float intraFontPrintColumnEx(intraFont *font, float x, float y, float column, const char *text, int length) {
    if (!text || length <= 0 || !font) return x;
	
#define LOCAL_BUFFER_LENGTH 256
	cccUCS2 buffer[ LOCAL_BUFFER_LENGTH ];
    cccUCS2* ucs2_text = buffer;
	
	// Allocate a temporary buffer if too long
	if (length > LOCAL_BUFFER_LENGTH) ucs2_text = (cccUCS2*)malloc(length*sizeof(cccUCS2));
    if (!ucs2_text) return x;
	
	//->UCS2 conversion
	length = cccCodetoUCS2(ucs2_text, length, (cccCode*)text, font->options/0x00010000);

	//for scrolling: if text contains '\n', replace with spaces 
	int i;
	if (font->options & INTRAFONT_SCROLL_LEFT) {
		for (i = 0; i < length; i++) {
			if (ucs2_text[i] == '\n') ucs2_text[i] = ' ';
		}
	}

	if (column >= 0) {
		x = intraFontPrintColumnUCS2Ex(font, x, y, column, ucs2_text, length);
	} else {                                                      //negative column prevents from drawing -> measure text
		x = intraFontMeasureTextUCS2Ex(font, ucs2_text, length);  //(hack to share local buffer between intraFontPrint and intraFontMeasure)
	}
	
	// Free temporary buffer if allocated
	if (ucs2_text != buffer) free(ucs2_text);
	
    return x;
}

float intraFontPrintUCS2(intraFont *font, float x, float y, const cccUCS2 *text) {
	return intraFontPrintColumnUCS2Ex(font, x, y, 0.0f, text, cccStrlenUCS2(text));
}

float intraFontPrintUCS2Ex(intraFont *font, float x, float y, const cccUCS2 *text, int length) {
	return intraFontPrintColumnUCS2Ex(font, x, y, 0.0f, text, length);
}

float intraFontPrintColumnUCS2(intraFont *font, float x, float y, float column, const cccUCS2 *text) {
	return intraFontPrintColumnUCS2Ex(font, x, y, 0.0f, text, cccStrlenUCS2(text));
}

float intraFontPrintColumnUCS2Ex(intraFont *font, float x, float y, float column, const cccUCS2 *text, int length) {
	if (!text || length <= 0 || !font) return x;

	//for scrolling: if text contains '\n', replace with spaces and call intraFontColumnUCS2Ex again
	int i;
	if (font->options & INTRAFONT_SCROLL_LEFT) {
		for (i = 0; i < length; i++) {
			if (text[i] == '\n') {
				cccUCS2* ucs2_text = (cccUCS2*)malloc(length*sizeof(cccUCS2));
				if (!ucs2_text) return x;
				for (i = 0; i < length; i++) ucs2_text[i] = (text[i] == '\n') ? ' ' : text[i];
				x = intraFontPrintColumnUCS2Ex(font, x, y, column, ucs2_text, length);
				free(ucs2_text);
				return x;
			}
		}
	}
	
	unsigned int color = font->color, shadowColor = font->shadowColor;
	float glyphscale = font->size;
	float width = 0.0f, height = font->advancey * glyphscale / 4.0;
	float left = x, top = y - 2*height;
	int eol = -1, n_spaces = -1, scroll = 0, textwidth;
	float fill = 0.0f;
	
	typedef struct {
		float u, v;
		unsigned int c;
		float x, y, z;
	} fontVertex;
	fontVertex *v, *v0, *v1, *s0, *s1;
	
	//count number of glyphs to draw and cache BMPs
	int j, n_glyphs, last_n_glyphs, n_sglyphs, changed, count = 0;
	unsigned short char_id, subucs2, glyph_id, glyph_ptr, shadowGlyph_ptr;
	do {
		changed = 0;
		n_glyphs = 0;
		n_sglyphs = 0;
		last_n_glyphs = 0;
		for(i = 0; i < length; i++) {
		
			char_id = intraFontGetID(font,text[i]); //char
			if (char_id < font->n_chars) {
				if (font->fileType == FILETYPE_PGF) { //PGF-file
					if ((font->glyph[char_id].flags & PGF_BMP_OVERLAY) == PGF_BMP_OVERLAY) { //overlay glyph?
						for (j = 0; j < 3; j++) {
							subucs2 = font->fontdata[(font->glyph[char_id].ptr)+j*2] + font->fontdata[(font->glyph[char_id].ptr)+j*2+1] * 256;				
							if (subucs2) {
								glyph_id = intraFontGetID(font, subucs2);
								if (glyph_id < font->n_chars) {
									n_glyphs++;
									if (!(font->glyph[glyph_id].flags & PGF_CACHED)) {
										if (intraFontGetBMP(font,glyph_id,PGF_CHARGLYPH)) changed = 1; 
									}								
								}
							}
						}
					} else {
						n_glyphs++; 
						if (!(font->glyph[char_id].flags & PGF_CACHED)) {
							if (intraFontGetBMP(font,char_id,PGF_CHARGLYPH)) changed = 1; 
						}
					}
			
					if (n_glyphs > last_n_glyphs) {
						n_sglyphs++; //shadow
						if (!(font->shadowGlyph[font->glyph[char_id].shadowID].flags & PGF_CACHED)) {
							if (intraFontGetBMP(font,font->glyph[char_id].shadowID,PGF_SHADOWGLYPH)) changed = 1; 
						}
						last_n_glyphs = n_glyphs;
					}
				
				} else {                            //BWFON-file
					n_glyphs++; 
					if (!(font->glyphBW[char_id].flags & PGF_CACHED)) {
						if (intraFontGetBMP(font,char_id,PGF_CHARGLYPH)) changed = 1; 
					}
					n_sglyphs++; //shadow
					if (!(font->shadowGlyph[0].flags & PGF_CACHED)) {
						if (intraFontGetBMP(font,font->glyph[0].shadowID,PGF_SHADOWGLYPH)) changed = 1; 
					}
				}
			}
			
		}
		count++;
	} while (changed && count <= length);
	if (changed) return x; //not all chars fit into texture -> abort (better solution: split up string and call intraFontPrintUCS2 twice)
	
	//reserve memory in displaylist
	v = sceGuGetMemory(((n_glyphs+n_sglyphs)<<1) * sizeof(fontVertex));

	int s_index = 0, c_index = n_sglyphs, last_c_index = n_sglyphs; // index for shadow and character/overlay glyphs	
	for (i = 0; i < length; i++) {

		//calculate left, height and possibly fill for character placement 
		if ((i == 0) || (text[i] == '\n') || ((column > 0.0f) && (i >= eol) && (text[i] != 32))) { //newline

			if (column > 0.0f) {                                                
				if (font->options & INTRAFONT_SCROLL_LEFT) { //scrolling (any direction) requested
					eol = length;
					scroll = 1;
					left = ((float)((int)x));
					union { int i; float f; } ux, uleft; 
					ux.f = x;
					uleft.f = left;
					count = ux.i - uleft.i;
					textwidth = intraFontMeasureTextUCS2Ex(font, text+i, length-i)+1;
					if (textwidth > column) {  //scrolling really required
		
						switch (font->options & PGF_SCROLL_MASK) {

							case INTRAFONT_SCROLL_LEFT:                                  //scroll left
								sceGuScissor(left-2, 0, left+column+4, 272); //limit to column width
								if (count < 60) {
									//show initial text for 1s
								} else if (count < (textwidth+90)) {
									left -= (count-60); //scroll left for (textwidth/60) s (speed = 1 pix/frame) and disappear for 0.5s
								} else if (count < (textwidth+120)) {
									color       = (color       & 0x00FFFFFF) | ((((color      >>24)*(count-textwidth-90))/30)<<24); //fade-in in 0.5s
									shadowColor = (shadowColor & 0x00FFFFFF) | ((((shadowColor>>24)*(count-textwidth-90))/30)<<24);
								} else {
									ux.f = left;     //reset counter
								}
								break;

							case INTRAFONT_SCROLL_SEESAW:  //scroll left and right
								sceGuScissor(left-column/2-2, 0, left+column/2+4, 272); //limit to column width
								textwidth -= column;
								if (count < 60) {
									left -= column/2; //show initial text (left side) for 1s
								} else if (count < (textwidth+60)) {
									left -= column/2+(count-60); //scroll left 
								} else if (count < (textwidth+120)) {
									left -= column/2+textwidth; //show right side for 1s
								} else if (count < (2*textwidth+120)) {
									left -= column/2+2*textwidth-count+120; //scroll right
								} else {
									ux.f = left;     //reset counter
									left -= column/2;
								}
								break;

							case INTRAFONT_SCROLL_RIGHT:  //scroll right
								sceGuScissor(left-column-2, 0, left+4, 272); //limit to column width
								if (count < 60) {
									left -= textwidth; //show initial text for 1s
								} else if (count < (textwidth+90)) {
									left -= textwidth-count+60; //scroll right for (textwidth/60) s (speed = 1 pix/frame) and disappear for 0.5s
								} else if (count < (textwidth+120)) {
									left -= textwidth;
									color       = (color       & 0x00FFFFFF) | ((((color      >>24)*(count-textwidth-90))/30)<<24); //fade-in in 0.5s
									shadowColor = (shadowColor & 0x00FFFFFF) | ((((shadowColor>>24)*(count-textwidth-90))/30)<<24);
								} else {
									ux.f = left;     //reset counter
									left -= textwidth;
								}
								break;

							case INTRAFONT_SCROLL_THROUGH:  //scroll through
								sceGuScissor(left-2, 0, left+column+4, 272); //limit to column width
								if (count < (textwidth+column+30)) {
									left += column+4-count; //scroll through
								} else {
									ux.f = left;     //reset counter
									left += column+4;
								}
								break;

						}
						ux.i++;             //increase counter
						x = ux.f;           //copy back to original var 
						sceGuEnable(GU_SCISSOR_TEST);
					}
				} else {					//automatic line-break required
					n_spaces = -1;
					eol = -1;
					fill = 0.0f;
					for (j = i; j < length; j++) {                                 
						if (text[j] == '\n') {                                           //newline reached -> no auto-line-break
							eol = j;
							break;                                   
						}
						if (text[j] == ' ') {                                        //space found for padding or eol
							n_spaces++;
							eol = j;
						}
						if (intraFontMeasureTextUCS2Ex(font, text+i, j+1-i) > column) { //line too long -> line-break
							if (eol < 0) eol = j;                                       //break in the middle of the word
							if (n_spaces > 0) fill = (column-intraFontMeasureTextUCS2Ex(font, text+i, eol-i))/((float)n_spaces);
							break;
						}
					}
					if (j == length) {
						eol = length;                                                 //last line
						while ((text[eol-1] == ' ') && (eol > 1)) eol--;                  //do not display trailing spaces
					}
					
					left = x;
					if ((font->options & PGF_ALIGN_MASK) == INTRAFONT_ALIGN_RIGHT)  left -= intraFontMeasureTextUCS2Ex(font, text+i,eol-i);
					if ((font->options & PGF_ALIGN_MASK) == INTRAFONT_ALIGN_CENTER) left -= intraFontMeasureTextUCS2Ex(font, text+i,eol-i)/2.0;

				}
			} else {  				//no column boundary -> display everything
				left = x;
				if (text[i] == '\n') {
					if ((font->options & PGF_ALIGN_MASK) == INTRAFONT_ALIGN_RIGHT)  left -= intraFontMeasureTextUCS2Ex(font, text+i+1,length-i-1);
					if ((font->options & PGF_ALIGN_MASK) == INTRAFONT_ALIGN_CENTER) left -= intraFontMeasureTextUCS2Ex(font, text+i+1,length-i-1)/2.0;
				} else {
					if ((font->options & PGF_ALIGN_MASK) == INTRAFONT_ALIGN_RIGHT)  left -= intraFontMeasureTextUCS2Ex(font, text+i,length-i);
					if ((font->options & PGF_ALIGN_MASK) == INTRAFONT_ALIGN_CENTER) left -= intraFontMeasureTextUCS2Ex(font, text+i,length-i)/2.0;
				}
			}

			width = 0.0f;
			height += font->advancey * glyphscale * 0.25;
			
		}

		char_id = intraFontGetID(font,text[i]);
		if (char_id < font->n_chars) {

			glyph_ptr = (font->fileType == FILETYPE_PGF) ? char_id : 0; //for fields not covered in GlyphBW (advance,...)
			shadowGlyph_ptr = (font->fileType == FILETYPE_PGF) ? font->glyph[glyph_ptr].shadowID : 0; 
		
			//center glyphs for monospace
			if (font->options & INTRAFONT_WIDTH_FIX) {
				width += ( ((float)(font->options & PGF_WIDTH_MASK))/2.0f - ((float)font->glyph[glyph_ptr].advance)/8.0f ) * glyphscale ;
			}
		
			//add vertices for subglyphs				
			for (j = 0; j < 3; j++) {
				if (font->fileType == FILETYPE_PGF) {
					if ((font->glyph[char_id].flags & PGF_BMP_OVERLAY) == PGF_BMP_OVERLAY) {
						subucs2 = font->fontdata[(font->glyph[char_id].ptr)+j*2] + font->fontdata[(font->glyph[char_id].ptr)+j*2+1] * 256;				
						glyph_id = intraFontGetID(font, subucs2);
					} else {
						glyph_id = char_id;
						j = 2;
					}
				} else { //FILETYPE_BWFON
					glyph_id = 0;
					j = 2;
				}
		
				if (glyph_id < font->n_chars) {
					Glyph *glyph = &(font->glyph[glyph_id]);
					if (font->fileType == FILETYPE_BWFON) {
						glyph->x = font->glyphBW[char_id].x;
						glyph->y = font->glyphBW[char_id].y;
					}

					v0 = &v[(c_index<<1) + 0];
					v1 = &v[(c_index<<1) + 1];

					v0->u = glyph->x-0.25f;
					v0->v = glyph->y-0.25f;
					v0->c = color;
					v0->x = left + width + glyph->left*glyphscale;
					v0->y = top + height - glyph->top *glyphscale;
					v0->z = 0.0f;

					v1->u = (glyph->x + glyph->width)+0.25f;
					v1->v = (glyph->y + glyph->height)+0.25f;
					v1->c = color;
					v1->x = left + width + (glyph->width+glyph->left)*glyphscale;
					v1->y = top + height + (glyph->height-glyph->top)*glyphscale;
					v1->z = 0.0f;
					
					c_index++;
				}
			}
				
			//add vertices for shadow
			if (c_index > last_c_index) {
				Glyph *shadowGlyph = &(font->shadowGlyph[shadowGlyph_ptr]);
		
				s0 = &v[(s_index<<1) + 0];
				s1 = &v[(s_index<<1) + 1];

				s0->u = shadowGlyph->x-0.25f;
				s0->v = shadowGlyph->y-0.25f;
				s0->c = shadowColor;
				s0->x = left + width + shadowGlyph->left*glyphscale*64.0f/((float)font->shadowscale);
				s0->y = top + height - shadowGlyph->top *glyphscale*64.0f/((float)font->shadowscale);
				s0->z = 0.0f;

				s1->u = (shadowGlyph->x + shadowGlyph->width)+0.25f;
				s1->v = (shadowGlyph->y + shadowGlyph->height)+0.25f;
				s1->c = shadowColor;
				s1->x = left + width + (shadowGlyph->width+shadowGlyph->left)*glyphscale*64.0f/((float)font->shadowscale);
				s1->y = top + height + (shadowGlyph->height-shadowGlyph->top)*glyphscale*64.0f/((float)font->shadowscale);
				s1->z = 0.0f;
			
				s_index++;
				last_c_index = c_index;
			}

			// advance
			if (font->options & INTRAFONT_WIDTH_FIX) {
				width += ( ((float)(font->options & PGF_WIDTH_MASK))/2.0f + ((float)font->glyph[glyph_ptr].advance)/8.0f ) * glyphscale; 
			} else {
				width += font->glyph[glyph_ptr].advance * glyphscale * 0.25;
			}
			
			if ((text[i] == 32) && ((font->options & INTRAFONT_ALIGN_FULL) == INTRAFONT_ALIGN_FULL)) width += fill;
			
		} else {
			if (font->altFont) {
				unsigned int altOptions = (font->altFont)->options;
				(font->altFont)->options = altOptions & (PGF_WIDTH_MASK+PGF_CACHE_MASK);
				width += intraFontPrintColumnUCS2Ex(font->altFont, left+width, top+height, 0.0f, text+i, 1) - (left+width);
				(font->altFont)->options = altOptions;
			}
		}
	}
		
	//finalize and activate texture (if not already active or has been changed)
	sceKernelDcacheWritebackAll();
	if (!(font->options & INTRAFONT_ACTIVE)) intraFontActivate(font);
	
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, (n_glyphs+n_sglyphs)<<1, 0, v);
	if (font->fileType == FILETYPE_BWFON) //draw chars again without shadows for improved readability
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, n_glyphs<<1, 0, v+(n_sglyphs<<1));
	sceGuEnable(GU_DEPTH_TEST);
	
	if (scroll == 1) {
		sceGuScissor(0, 0, 480, 272); //reset window to whole screen (test was previously enabled)
		return x;             //for scrolling return x-pos (including updated counter)
	} else {
		return left+width;    //otherwise return x-pos at end of text
	}
}

float intraFontMeasureText(intraFont *font, const char *text) {
	if (!font) return 0;
	int length = cccStrlenCode((cccCode*)text, font->options/0x00010000);
	return intraFontMeasureTextEx(font, text, length);
}

float intraFontMeasureTextEx(intraFont *font, const char *text, int length) { 
	return intraFontPrintColumnEx(font, 0.0f, 0.0f, -1.0f, text, length); //explanation: intraFontPrintColumnEx does the String -> UCS2 conversation,
		                                                                  //but a negative column width triggers measurement without drawing
} 

float intraFontMeasureTextUCS2(intraFont *font, const cccUCS2 *text) { 
	return intraFontMeasureTextUCS2Ex(font, text, cccStrlenUCS2(text));
}

float intraFontMeasureTextUCS2Ex(intraFont *font, const cccUCS2 *text, int length) { 
   if (!text || length <= 0 || !font) return 0.0f; 

   int i; 
   float x = 0.0f; 

   for (i = 0; (i < length) && (text[i] != '\n'); i++) { //measure until end of string or first newline char
		unsigned short char_id = intraFontGetID(font,text[i]); 
		if (char_id < font->n_chars) {
			unsigned short glyph_ptr = (font->fileType == FILETYPE_PGF) ? char_id : 0; //for fields not covered in GlyphBW (advance,...)
			x += (font->options & INTRAFONT_WIDTH_FIX) ? (font->options & PGF_WIDTH_MASK)*font->size : (font->glyph[glyph_ptr].advance)*font->size*0.25f; 
		} else {
			x += intraFontMeasureTextUCS2Ex(font->altFont, text+i, 1); //try alternative font if char does not exist in current font
		}
	} 

   return x; 
}

