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

#include "common.h"
#include "macros.h"

const char *g_messages_en[] = {
    "SHUTDOWN DEVICE",
    "SUSPEND DEVICE",
    "RESET DEVICE",
    "RESET VSH",
    "EXIT",
    "ARK 1.50 VSH MENU",
};

u8 message_test_en[NELEMS(g_messages_en) == MSG_END ? 0 : -1];
