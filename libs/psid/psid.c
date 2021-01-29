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

#include "psid.h"
#include <string.h>

void prxXorKeyMix(unsigned char *dstBuf, unsigned int size, unsigned char *srcBuf, unsigned char *xorKey)
{
	unsigned int i;

	i = 0;

	while (i < size) {
		dstBuf[i] = srcBuf[i] ^ xorKey[i];
		++i;
	}
}

int isPrxEncrypted(unsigned char *prx, unsigned int size)
{
	if (size < 0x160)
		return 0;

	if (0 != memcmp(prx+0xD0, ENCRYPTED_TAG_MAGIC_1, 4))
	{
		return 0;
	}

	if (0 != memcmp(prx+0x130, ENCRYPTED_TAG_MAGIC_2, 4))
	{
		return 0;
	}

	return 1;
}
