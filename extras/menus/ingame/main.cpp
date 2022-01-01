/*
    TN SaveState Plugin
    Copyright (C) 2014, Total_Noob

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

#include <time.h>

#include "start.h"
#include <pspkernel.h>
#include "gfx.h"
#include "debug.h"
#include "common.h"
#include "system_mgr.h"
#include "exitmgr.h"
//#include "browser.h"

#define MAX_ENTRIES 1
static SystemEntry* entries[MAX_ENTRIES];

int main(int argc, char** argv)
{
    intraFontInit();
    ya2d_init();

    common::loadData(argc, argv);

    /*
    //entries[1] = new Browser();
    entries[0] = new ExitManager();
    
    SystemMgr::initMenu(entries, MAX_ENTRIES);
    SystemMgr::startMenu();
    SystemMgr::endMenu();
    */
    
    Controller control;
    while (1){
        common::clearScreen();
        common::getImage(IMAGE_BG)->draw(0, 0);
        common::flipScreen();
        if (control.decline()){
            break;
        }
    }

    common::deleteData();
    
    intraFontShutdown();
    ya2d_shutdown();

    return 0;
}
