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

#ifndef DIRENT_TRACK_H
#define DIRENT_TRACK_H

struct IoDirentEntry {
	char *path;
	SceUID dfd, iso_dfd;
	struct IoDirentEntry *next;
};

int dirent_add(SceUID dfd, SceUID iso_dfd, const char *path);
int dirent_remove(struct IoDirentEntry *p);
struct IoDirentEntry *dirent_search(SceUID magic);

#endif
