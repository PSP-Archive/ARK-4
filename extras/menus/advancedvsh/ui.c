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

/*
	PSP VSH extender for devhook 0.50+
*/
#include "common.h"

int limit(int val,int min,int max)
{
	if(val<min) val = max;
	if(val>max) val = min;
	return val;
}

/*
int get_max_len(char **str_list,int nums)
{
	int max_len = 0;
	int i;
	for(i=0;i<nums;i++)
	{
		int len;

#ifdef CONFIG_639
		if(psp_fw_version == FW_639)
			len = scePaf_strlen(str_list[i]);
#endif

#ifdef CONFIG_635
		if(psp_fw_version == FW_635)
			len = scePaf_strlen(str_list[i]);
#endif

#ifdef CONFIG_620
		if(psp_fw_version == FW_620)
			len = scePaf_strlen_620(str_list[i]);
#endif
	
#if defined(CONFIG_660) || defined(CONFIG_661)
		if((psp_fw_version == FW_660) || (psp_fw_version == FW_661))
			len = scePaf_strlen_660(str_list[i]);
#endif
	
		if(max_len < len) max_len = len;
	}
	return max_len;
}
*/
