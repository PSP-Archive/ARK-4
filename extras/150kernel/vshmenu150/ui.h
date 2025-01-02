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

#ifndef __UI_H__
#define __UI_H__

// key_wait()
#define SENSE_KEY (PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|PSP_CTRL_START|PSP_CTRL_SELECT)

#define ALL_ALLOW    (PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT)
#define ALL_BUTTON   (PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE)
#define ALL_TRIGGER  (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)
#define ALL_FUNCTION (PSP_CTRL_SELECT|PSP_CTRL_START|PSP_CTRL_HOME|PSP_CTRL_HOLD|PSP_CTRL_NOTE)
#define ALL_CTRL     (ALL_ALLOW|ALL_BUTTON|ALL_TRIGGER|ALL_FUNCTION)

#define IGR_BASE   (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)
#define IGR_RESET  (IGR_BASE|PSP_CTRL_HOME)
#define IGR_RETURN (IGR_BASE|PSP_CTRL_START)
#define IGR_LOGON  (IGR_BASE|PSP_CTRL_SELECT)

int limit(int val,int min,int max);
int get_max_len(char **str_list,int nums);

#endif
