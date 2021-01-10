/*
 * endian.h
 *
 *  Created on: Jan 4, 2011
 */

#ifndef ENDIAN_H_
#define ENDIAN_H_

static inline u16 Swap16(u16 v)
{
	return (((v & 0x00FF) << 8) | (v & 0xFF00) >> 8);
}

static inline u32 Swap32(u32 v)
{
	return (((v & 0x000000FF) << 24) | ((v & 0x0000FF00) << 8) |
		((v & 0xFF000000) >> 24) | ((v & 0x00FF0000) >> 8));
}

static inline u64 Swap64(u64 v)
{
	return (((v & 0x00000000000000FFULL) << 56) |
		((v & 0x000000000000FF00ULL) << 40) |
		((v & 0x0000000000FF0000ULL) << 24) |
		((v & 0x00000000FF000000ULL) <<  8) |
		((v & 0x000000FF00000000ULL) >>  8) |
		((v & 0x0000FF0000000000ULL) >> 24) |
		((v & 0x00FF000000000000ULL) >> 40) |
		((v & 0xFF00000000000000ULL) >> 56));
}


#endif /* ENDIAN_H_ */
