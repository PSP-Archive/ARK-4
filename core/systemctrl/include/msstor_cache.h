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

#ifndef _MSSTOR_CACHE_H_
#define _MSSTOR_CACHE_H_

// Cache Buffer Size
#define CACHE_BUFSIZE (16 * 1024)
#define CACHE_BUFSIZE_GO (8 * 1024)

// Initialize "ms" Driver Cache
int msstorCacheInit(const char* driver, int bufSize);

#endif

