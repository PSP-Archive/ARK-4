/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>

int removePath(const char *path) {
	SceUID dfd = sceIoDopen(path);
	if (dfd >= 0) {
		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = sceIoDread(dfd, &dir);
			if (res > 0) {
				if (dir.d_name[0] != '.') {
					char new_path[128];
					sprintf(new_path, "%s/%s", path, dir.d_name);
					removePath(new_path);
				}
			}
		} while (res > 0);

		sceIoDclose(dfd);

		sceIoRmdir(path);
	} else {
		sceIoRemove(path);
	}

	return 0;
}

int sceLflashFatfmtStartFatfmt(int argc, char *argv[]) {
	if (argv) {
		if (strncmp(argv[1], "lflash0:0,", 10) == 0) {
			int num = argv[1][10] - '0';

			char path[128];
			sprintf(path, "ms0:/__ADRENALINE__/flash%d", num);

			removePath(path);
			sceIoMkdir(path, 0777);
		}
	}

	return 0;
}