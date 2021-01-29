/*
 * libccc.c
 * Character Code Conversion Library
 * Version 0.31 by BenHur - http://www.psp-programming.com/benhur
 *
 * This work is licensed under the Creative Commons Attribution-Share Alike 3.0 License.
 * See LICENSE for more details.
 *
 */

#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "libccc.h"

static unsigned char cccInitialized = 0;
static void* __table_ptr__[CCC_N_CP];
static void* __table_end__[CCC_N_CP];
static unsigned char __table_dyn__[CCC_N_CP];
static cccUCS2 __error_char_ucs2__ = 0x0000U;

/* the following code is adapted from libLZR 0.11 (see http://www.psp-programming.com/benhur) */

void cccLZRFillBuffer(unsigned int *test_mask, unsigned int *mask, unsigned int *buffer, unsigned char **next_in) {
	/* if necessary: fill up in buffer and shift mask */
	if (*test_mask <= 0x00FFFFFFu) {
		(*buffer) = ((*buffer) << 8) + *(*next_in)++;
		*mask = *test_mask << 8;
	}
}

char cccLZRNextBit(unsigned char *buf_ptr1, int *number, unsigned int *test_mask, unsigned int *mask, unsigned int *buffer, unsigned char **next_in) {
	/* extract and return next bit of information from in stream, update buffer and mask */
	cccLZRFillBuffer(test_mask, mask, buffer, next_in);
	unsigned int value = (*mask >> 8) * (*buf_ptr1);
	if (test_mask != mask) *test_mask = value;
	*buf_ptr1 -= *buf_ptr1 >> 3;
	if (number) (*number) <<= 1;
	if (*buffer < value) {
		*mask = value;		
		*buf_ptr1 += 31;		
		if (number) (*number)++;
		return 1;
	} else {
		*buffer -= value;
		*mask -= value;
		return 0;
	}
}

int cccLZRGetNumber(signed char n_bits, unsigned char *buf_ptr, char inc, char *flag, unsigned int *mask, unsigned int *buffer, unsigned char **next_in) {
	/* extract and return a number (consisting of n_bits bits) from in stream */
	int number = 1;
	if (n_bits >= 3) {
		cccLZRNextBit(buf_ptr+3*inc, &number, mask, mask, buffer, next_in);
		if (n_bits >= 4) {
			cccLZRNextBit(buf_ptr+3*inc, &number, mask, mask, buffer, next_in);
			if (n_bits >= 5) {
				cccLZRFillBuffer(mask, mask, buffer, next_in);
				for (; n_bits >= 5; n_bits--) {
					number <<= 1;
					(*mask) >>= 1;
					if (*buffer < *mask) number++; else (*buffer) -= *mask;
				}
			}
		}
	}
	*flag = cccLZRNextBit(buf_ptr, &number, mask, mask, buffer, next_in);
	if (n_bits >= 1) {
		cccLZRNextBit(buf_ptr+inc, &number, mask, mask, buffer, next_in);
		if (n_bits >= 2) {
			cccLZRNextBit(buf_ptr+2*inc, &number, mask, mask, buffer, next_in);
		}
	}	
	return number;
}

int cccLZRDecompress(void *out, unsigned int out_capacity, void *in, void *in_end) { 
	unsigned char **next_in, *tmp, *next_out, *out_end, *next_seq, *seq_end, *buf_ptr1, *buf_ptr2;
	unsigned char last_char = 0;
	int seq_len, seq_off, n_bits, buf_off = 0, i, j;	
	unsigned int mask = 0xFFFFFFFF, test_mask;
	char flag;
	
	signed char type = *(signed char*)in;
	unsigned int buffer = ((unsigned int)*(unsigned char*)(in+1) << 24) + 
	                      ((unsigned int)*(unsigned char*)(in+2) << 16) + 
	                      ((unsigned int)*(unsigned char*)(in+3) <<  8) + 
	                      ((unsigned int)*(unsigned char*)(in+4)      );	
	next_in = (in_end) ? in_end : &tmp; //use user provided counter if available
	*next_in = in + 5;
	next_out = out;
	out_end = out + out_capacity;

	if (type < 0) { 
		
		/* copy from stream without decompression */

		seq_end = next_out + buffer;
		if (seq_end > out_end) return CCC_ERROR_BUFFER_SIZE;
		while (next_out < seq_end) {
			*next_out++ = *(*next_in)++;
		} 
		(*next_in)++; //skip 1 byte padding
		return next_out - (unsigned char*)out; 

	}

	/* create and init buffer */
	unsigned char *buf = (unsigned char*)malloc(2800);
	if (!buf) return CCC_ERROR_MEM_ALLOC;
	for (i = 0; i < 2800; i++) buf[i] = 0x80;

	while (1) {

		buf_ptr1 = buf + buf_off + 2488;
		if (!cccLZRNextBit(buf_ptr1, 0, &mask, &mask, &buffer, next_in)) {

			/* single new char */

			if (buf_off > 0) buf_off--;
			if (next_out == out_end) return CCC_ERROR_BUFFER_SIZE;
			buf_ptr1 = buf + (((((((int)(next_out - (unsigned char*)out)) & 0x07) << 8) + last_char) >> type) & 0x07) * 0xFF - 0x01;
			for (j = 1; j <= 0xFF; ) {
				cccLZRNextBit(buf_ptr1+j, &j, &mask, &mask, &buffer, next_in);
			}
			*next_out++ = j;

		} else {                       

			/* sequence of chars that exists in out stream */

			/* find number of bits of sequence length */			
			test_mask = mask;
			n_bits = -1;
			do {
				buf_ptr1 += 8;
				flag = cccLZRNextBit(buf_ptr1, 0, &test_mask, &mask, &buffer, next_in);
				n_bits += flag;
			} while ((flag != 0) && (n_bits < 6));
			
			/* find sequence length */
			buf_ptr2 = buf + n_bits + 2033;
			j = 64;
			if ((flag != 0) || (n_bits >= 0)) {
				buf_ptr1 = buf + (n_bits << 5) + (((((int)(next_out - (unsigned char*)out)) << n_bits) & 0x03) << 3) + buf_off + 2552;
				seq_len = cccLZRGetNumber(n_bits, buf_ptr1, 8, &flag, &mask, &buffer, next_in);
				if (seq_len == 0xFF) return next_out - (unsigned char*)out; //end of data stream
				if ((flag != 0) || (n_bits > 0)) {
					buf_ptr2 += 56;
					j = 352;
				}
			} else {
				seq_len = 1;
			}

			/* find number of bits of sequence offset */			
			i = 1;
			do {
				n_bits = (i << 4) - j;
				flag = cccLZRNextBit(buf_ptr2 + (i << 3), &i, &mask, &mask, &buffer, next_in);
			} while (n_bits < 0);

			/* find sequence offset */
			if (flag || (n_bits > 0)) {
				if (!flag) n_bits -= 8;
				seq_off = cccLZRGetNumber(n_bits/8, buf+n_bits+2344, 1, &flag, &mask, &buffer, next_in);
			} else {
				seq_off = 1;
			}

			/* copy sequence */
			next_seq = next_out - seq_off;
			if (next_seq < (unsigned char*)out) return CCC_ERROR_INPUT_STREAM;
			seq_end = next_out + seq_len + 1;
			if (seq_end > out_end) return CCC_ERROR_BUFFER_SIZE;
			buf_off = ((((int)(seq_end - (unsigned char*)out))+1) & 0x01) + 0x06;
			do {
				*next_out++ = *next_seq++;
			} while (next_out < seq_end);

		}
		last_char = *(next_out-1);		
	}
}

/* end of code adapted from libLZR 0.11 (see http://www.psp-programming.com/benhur) */

int cccSetTable(void* table, unsigned int bytesize, unsigned char cp, unsigned char dyn) {
	if (cp < CCC_N_CP) {
		if (__table_dyn__[cp] && __table_ptr__[cp]) 
			free(__table_ptr__[cp]);
		__table_ptr__[cp] = table;
		__table_end__[cp] = table+bytesize;
		__table_dyn__[cp] = dyn;
		return CCC_SUCCESS;
	} else 
		return CCC_ERROR_UNSUPPORTED;
}

int cccLoadTable(const char *filename, unsigned char cp) {
	if (cp >= CCC_N_CP) return CCC_ERROR_UNSUPPORTED;
    
	/* read in (compressed) table_data */
	SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
    if (fd < 0) return CCC_ERROR_FILE_READ;
    unsigned int filesize = sceIoLseek(fd, 0, SEEK_END);
    sceIoLseek(fd, 0, SEEK_SET);
    void* table_data = (void*)malloc(filesize);
	if (!table_data) {
		sceIoClose(fd);
		return CCC_ERROR_MEM_ALLOC;
	}
    if (sceIoRead(fd, table_data, filesize) != filesize) {
		sceIoClose(fd);
		free(table_data);
		return CCC_ERROR_FILE_READ;
	}
	sceIoClose(fd);

	/* decompress requested tables */
	unsigned int *header = (unsigned int*)table_data;
	while (header[0]) {
		if ((cp == 0) || (cp == header[0])) {
			void* table = (void*)malloc(header[4]);
			if (!table) {
				free(table_data);
				return CCC_ERROR_MEM_ALLOC;
			}
			int ret = cccLZRDecompress(table, header[4], table_data+header[2], NULL);
			if (ret < 0) {
				free(table_data);
				free(table);
				return ret;
			}
			cccSetTable(table, header[4], header[0], 1);
		}
		header += 8;
	}
	free(table_data);
	return CCC_SUCCESS;    
}

void cccInit(void) {
	if (!cccInitialized) {
		int cp;
		for (cp = 0; cp < CCC_N_CP; cp++) {
			__table_ptr__[cp] = NULL;
			__table_end__[cp] = NULL;
			__table_dyn__[cp] = 0;
		}
		//cccLoadTable("flash0:/vsh/etc/cptbl.dat", 0); //this would load all tables available, but it's done on demand
		cccInitialized = 1;
	}
}

void cccShutDown(void) {
	if (cccInitialized) {
		//free dynamically loaded tables
		int cp;
		for (cp = 0; cp < CCC_N_CP; cp++) 
			cccSetTable(NULL, 0, cp, 0);
		cccInitialized = 0;
	}
}

cccUCS2 cccSetErrorCharUCS2(cccUCS2 code) {
	cccUCS2 old = __error_char_ucs2__;
	__error_char_ucs2__ = code;
	return old;
}

int cccStrlen(cccCode const * str) {
	if (!str) return 0;

	return strlen((char*)str);
}

int cccStrlenSJIS(cccCode const * str) {
	if (!str) return 0;

	int i = 0, length = 0;
	while (str[i]) {
		length++;
		i += (str[i] <= 0x80 || (str[i] >= 0xA0 && str[i] <= 0xDF) || str[i] >= 0xFD) ? 1 : 2; //single or double byte
	}
	return length;
}

int cccStrlenGBK(cccCode const * str) {
	if (!str) return 0;

	int i = 0, length = 0;
	while (str[i]) {
		length++;
		i += (str[i] <= 0x80 || str[i] == 0xFF) ? 1 : 2; //single or double byte
	}
	return length;
}

int cccStrlenKOR(cccCode const * str) {
	return cccStrlenGBK(str);
}

int cccStrlenBIG5(cccCode const * str) {
	return cccStrlenGBK(str);
}

int cccStrlenUTF8(cccCode const * str) {
	if (!str) return 0;

	int i = 0, length = 0;
	while (str[i]) {
		if      (str[i] <= 0x7F) { i++;    length++; } //ASCII
		else if (str[i] <= 0xC1) { i++;              } //part of multi-byte or overlong encoding ->ignore
		else if (str[i] <= 0xDF) { i += 2; length++; } //2-byte
		else if (str[i] <= 0xEF) { i += 3; length++; } //3-byte
		else                     { i++;              } //4-byte, restricted or invalid range ->ignore
	}
	return length;
}

int cccStrlenCode(cccCode const * str, unsigned char cp) {
	if (!str) return 0;
	
	int length = 0;
    switch (cp) {
		/* multi byte character sets */
		case CCC_CP932:  length = cccStrlenSJIS(str); break;
		case CCC_CP936:  length = cccStrlenGBK (str); break;
		case CCC_CP949:  length = cccStrlenKOR (str); break;
		case CCC_CP950:  length = cccStrlenBIG5(str); break;
		case CCC_CPUTF8: length = cccStrlenUTF8(str); break;
		/* single byte character sets */
		default:        length = cccStrlen(str); break;
	}
	return length;
}

int cccStrlenUCS2(cccUCS2 const * str) {
	if (!str) return 0;

	int length = 0;
	while (str[length]) length++;
	return length;
}


int cccSJIStoUCS2(cccUCS2 * dst, size_t count, cccCode const * str) {
	if (!str || !dst) return 0;
	if (!cccInitialized) cccInit();
	if (!(__table_ptr__[CCC_CP932])) cccLoadTable("flash0:/vsh/etc/cptbl.dat", CCC_CP932);
	
	int i = 0, length = 0, j, code, id;
	if (__table_ptr__[CCC_CP932]) { //table is present
		unsigned short *header = (unsigned short*)(__table_ptr__[CCC_CP932]);
		cccUCS2 *SJIStoUCS2 = (cccUCS2*)header+header[2]*3+3;		
		while (str[i] && length < count) {
			code = str[i];
			id = -1;
			for (j = 1; (j <= header[2]) && (id < 0); j++) {
				if ((code >= header[j*3]) && (code <= header[j*3+1])) {
					id = header[j*3+2] + code - header[j*3]; 
				} else {
					if (j == 2) code = 0x0200 * str[i] - 0xE100 - ((str[i] >= 0xE0) ? 0x8000 : 0) + str[i+1] + ((str[i+1] <= 0x7E) ? -0x1F : ((str[i+1] >= 0x9F) ? 0x82 : -0x20) );
				}
			}
			dst[length++] = (id < 0) ? __error_char_ucs2__ : SJIStoUCS2[id];
			i += (str[i] <= 0x80 || (str[i] >= 0xA0 && str[i] <= 0xDF) || str[i] >= 0xFD) ? 1 : 2; //single or double byte
		}
	} else { //table not present
		while (str[i] && length < count) {
			dst[length++] = __error_char_ucs2__;
			i += (str[i] <= 0x80 || (str[i] >= 0xA0 && str[i] <= 0xDF) || str[i] >= 0xFD) ? 1 : 2; //single or double byte
		}
	}
	return length;
}

int cccGBKtoUCS2(cccUCS2 * dst, size_t count, cccCode const * str) {
	if (!str || !dst) return 0;
	if (!cccInitialized) cccInit();
	if (!(__table_ptr__[CCC_CP936])) cccLoadTable("flash0:/vsh/etc/cptbl.dat", CCC_CP936);

	unsigned char* entry;
	unsigned short code;
	int i = 0, length = 0;
	while (str[i] && length < count) {
		if (str[i] <= 0x7f) {
			dst[length] = (cccUCS2)str[i]; 
		} else if (str[i] <= 0x80) {
			dst[length] = 0x20ac;
        } else if (str[i] <= 0xfe) {
			if (__table_ptr__[CCC_CP936]) { //table is present
				code = 0x0100 * str[i] + str[i+1];			
				for (entry = (unsigned char*)(__table_ptr__[CCC_CP936]); (entry < (unsigned char*)(__table_end__[CCC_CP936])) && ((entry[0]+0x100*entry[1] + entry[4]) <= code); entry += 5);			
				if ((entry >= (unsigned char*)(__table_end__[CCC_CP936])) || (code < entry[0]+0x100*entry[1])) {
					dst[length] = __error_char_ucs2__;
				} else {
					dst[length] = entry[2]+0x100*entry[3] + (code - entry[0]-0x100*entry[1]);
				}
			} else {
				dst[length] = __error_char_ucs2__;
			}
		} else {
			dst[length] = __error_char_ucs2__;
		}
        length++;
        i += (str[i] <= 0x80 || str[i] == 0xFF) ? 1 : 2; //single or double byte
	}
	return length;
}

int cccKORtoUCS2(cccUCS2 * dst, size_t count, cccCode const * str) {
	if (!str || !dst) return 0;
	if (!cccInitialized) cccInit();
	if (!(__table_ptr__[CCC_CP949])) cccLoadTable("flash0:/vsh/etc/cptbl.dat", CCC_CP949);

	unsigned char* entry;
	unsigned short code;
	int i = 0, length = 0;
	while (str[i] && length < count) {
		if (str[i] <= 0x7f) {
			dst[length] = (cccUCS2)str[i]; 
		} else if (str[i] <= 0x80) {
			dst[length] = __error_char_ucs2__;
		} else if (str[i] <= 0xfd) {
			if (__table_ptr__[CCC_CP949]) { //table is present
				code = 0x0100 * str[i] + str[i+1];			
				for (entry = (unsigned char*)(__table_ptr__[CCC_CP949]); (entry < (unsigned char*)(__table_end__[CCC_CP949])) && ((entry[0]+0x100*entry[1] + entry[4]) <= code); entry += 5);			
				if ((entry >= (unsigned char*)(__table_end__[CCC_CP949])) || (code < entry[0]+0x100*entry[1])) {
					dst[length] = __error_char_ucs2__;
				} else {
					dst[length] = entry[2]+0x100*entry[3] + (code - entry[0]-0x100*entry[1]);
				}
			} else {
				dst[length] = __error_char_ucs2__;
			}
		} else {
			dst[length] = __error_char_ucs2__;
		}
        length++;
        i += (str[i] <= 0x80 || str[i] == 0xFF) ? 1 : 2; //single or double byte
	}
	return length;
}

int cccBIG5toUCS2(cccUCS2 * dst, size_t count, cccCode const * str) {
	if (!str || !dst) return 0;
	if (!cccInitialized) cccInit();
	if (!(__table_ptr__[CCC_CP950])) cccLoadTable("flash0:/vsh/etc/cptbl.dat", CCC_CP950);

	typedef struct {
		unsigned short code;
		cccUCS2 ucs2;
	} cccTblEntry;
	cccTblEntry* entry;
	unsigned short code;
	int i = 0, length = 0;
	while (str[i] && length < count) {
		if (str[i] <= 0x7f) {
			dst[length] = (cccUCS2)str[i]; 
        } else if (str[i] <= 0xa0) {
			dst[length] = __error_char_ucs2__;
		} else if (str[i] <= 0xf9) {
			if (__table_ptr__[CCC_CP950]) { //table is present
				code = 0x0100 * str[i] + str[i+1];			
				for (entry = (cccTblEntry*)(__table_ptr__[CCC_CP950]); (entry < (cccTblEntry*)(__table_end__[CCC_CP950])) && (entry->code < code); entry++);
				if ((entry >= (cccTblEntry*)(__table_end__[CCC_CP950])) || (code < entry->code)) {
					dst[length] = __error_char_ucs2__;
				} else {
					dst[length] = entry->ucs2;
				}		
			} else {
				dst[length] = __error_char_ucs2__;
			}
		} else {
			dst[length] = __error_char_ucs2__;
		}
        length++;
        i += (str[i] <= 0x80 || str[i] == 0xFF) ? 1 : 2; //single or double byte
	}
	return length;
}

int cccUTF8toUCS2(cccUCS2 * dst, size_t count, cccCode const * str) {
	if (!str || !dst) return 0;

    int i = 0, length = 0;
    while (str[i] && length < count) {
		if  (str[i] <= 0x7FU) {       //ASCII
			dst[length] = (cccUCS2)str[i]; 
			i++;    length++; 
		} else if (str[i] <= 0xC1U) { //part of multi-byte or overlong encoding ->ignore
			i++;          
		} else if (str[i] <= 0xDFU) { //2-byte
			dst[length] = ((str[i]&0x001fu)<<6) | (str[i+1]&0x003fu); 
			i += 2; length++; 
		} else if (str[i] <= 0xEFU) { //3-byte
			dst[length] = ((str[i]&0x001fu)<<12) | ((str[i+1]&0x003fu)<<6) | (str[i+2]&0x003fu); 
			i += 3; length++; 
		} else i++;                    //4-byte, restricted or invalid range ->ignore
	}
    return length;
}

int cccCodetoUCS2(cccUCS2 * dst, size_t count, cccCode const * str, unsigned char cp) {
	if (!str || !dst) return 0;

	int length = 0;
	switch (cp) {
		//multi-byte character sets
		case CCC_CP932: length = cccSJIStoUCS2(dst, count, str); break;
		case CCC_CP936: length = cccGBKtoUCS2(dst, count, str); break;
		case CCC_CP949: length = cccKORtoUCS2(dst, count, str); break;
		case CCC_CP950: length = cccBIG5toUCS2(dst, count, str); break;
		case CCC_CPUTF8: length = cccUTF8toUCS2(dst, count, str); break;
		//single-byte character sets
		default: 
			if (cp < CCC_N_CP) { //codepage in range?
				if (!cccInitialized) cccInit();
				if (!(__table_ptr__[cp]) && (cp > 0)) cccLoadTable("flash0:/vsh/etc/cptbl.dat", cp);
				while (str[length] && length < count) { //conversion: ASCII (if ASCII) or LUT-value (if LUT exists) or error_char (if LUT doesn't exist)
					if (str[length] < 0x80) {
						dst[length] = (cccUCS2)str[length];
					} else {
						dst[length] =  ((__table_ptr__[cp]) ? ((cccUCS2*)(__table_ptr__[cp]))[str[length]-0x80] : __error_char_ucs2__); 
						if (!dst[length]) dst[length] = __error_char_ucs2__;
					}
					length++; 
				}
			}
			break;
	} 	
	return length;
}

