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

#ifndef _GAMEINFO_H_
#define _GAMEINFO_H_

/* Define this to delete vpl */
#undef USE_GAME_KERNEL_DELETE_VPL

/* Define this pointed to vpl address which needed to be freed */
#undef GAME_VPL_UID

/* Define this to delete fpl */
#undef USE_GAME_KERNEL_DELETE_FPL

/* Define this pointed to fpl address which needed to be freed */
#undef GAME_FPL_UID

/* Define this to use sceUtilityLoadNetModule */
#undef USE_GAME_UTILITY_LOAD_NET_MODULE

/* Define this to use sceUtilityUnloadNetModule */
#undef USE_GAME_UTILITY_UNLOAD_NET_MODULE

/* Define this to use sceUtilityLoadModule */
#define USE_GAME_UTILITY_LOAD_MODULE 1

/* Define this to use sceUtilityUnloadModule */
#define USE_GAME_UTILITY_UNLOAD_MODULE 1

/* Define this to use sceKernelFreePartitionMemory */
#undef USE_GAME_KERNEL_FREE_PARTITION_MEMORY

/* Define this pointed to memory partition's UID */
#undef GAME_USERSBRK_UID

/* Define this to use the offset for sceWlanGetEtherAddr from GAME_TEXT, undef this and then ARK will try to find it by itself */
#undef GAME_GET_ETHER_ADDR

/* Define this to the address of libhttp.prx' text segment, undef this and then ARK will try to find it by itself */
#undef GAME_HTTP_TEXT

//#include <game_config.h>

#endif

