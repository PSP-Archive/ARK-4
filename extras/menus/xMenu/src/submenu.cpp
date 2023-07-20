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


#include <systemctrl.h>
#include <systemctrl_se.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "menu.h"

#include "../../arkMenu/include/conf.h"

static t_conf config;

extern SEConfig* se_config;
extern ARKConfig* ark_config;

/*
static char* OPTIONS[] = {
	(char*)"Disabled",
	(char*)"Always",
	(char*)"PS1",
	(char*)"Launcher",
};


struct submenu_elements {
	char** options;
	const char* restart;
	const char* exit;
};
*/

static string save_status;
static int status_frame_count = 0; // a few seconds

SubMenu::SubMenu(Menu* menu) {
    this->index = 0;
    this->menu = menu;
	this->getItems();
}

void SubMenu::getItems() {
    stringstream ver;
    ver << "Memory Stick Speedup: " << ((se_config->msspeed)? "Enabled" : "Disabled");

    options[0] = ver.str();
    options[1] = "Restart";
    options[2] = "Exit";

    /*
	uint16_t memory_stick_speed = se_config->msspeed;
	struct submenu_elements submenu_elem = {
		OPTIONS,
		"Restart",
		"Exit"
	};
    */
}

void SubMenu::updateScreen(){
    clearScreen(CLEAR_COLOR);
    
    // draw main menu first
    menu->draw();

    // now draw our stuff
    int n = sizeof(options)/sizeof(options[0]);
    int w = 260;
    int h = 100;
    int x = (480-w)/2;
    int y = (272-h)/2;
    u32 color = 0xa0808000;

    // menu window
    fillScreenRect(color, x, y, w, h);

    // menu items
    int cur_x;
    int cur_y = y + (h-(10*n))/2;
    for (int i=0; i<n; i++){
        cur_x = x + ((w-(8*options[i].size()))/2);
        common::printText(cur_x, cur_y, options[i].c_str());
        if (i == index)
            fillScreenRect(0x00FFFFFF, cur_x, cur_y+7, min((int)options[i].size()*7, w), 1);
        cur_y += 10;
    }

    // draw save status
	if(save_status.length() > 1){
		printTextScreen(RIGHT, TOP+15, save_status.c_str(), GREEN_COLOR);

        if (status_frame_count) status_frame_count--;
        else save_status = "";
	}

    common::flip();
}

void SubMenu::run() {
	
    save_status = "";

	Controller control;
    control.flush();
	while(1) {

		updateScreen();

		control.update();
		if (control.circle() || control.triangle())
			break;
        else if (control.cross()){
            switch (index){
                case 0: changeMsCacheSetting(); getItems(); break;
                case 1: rebootMenu(); break;
                case 2: sceKernelExitGame(); break;
            }
        }
        else if (control.up()){
            if (index > 0) index--;
        }
        else if (control.down()){
            if (index < sizeof(options)/sizeof(options[0])) index++;
        }

	}
    control.flush();
}

SubMenu::~SubMenu() {}

void SubMenu::rebootMenu(){

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(SceKernelLoadExecVSHParam));

    char path[256];
    strcpy(path, ark_config->arkpath);
	strcat(path, ARK_XMENU);

    int runlevel = 0x141;
    
    param.args = strlen(path) + 1;
    param.argp = path;
    param.key = "game";
    menu->fadeOut();
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}

void SubMenu::changeMsCacheSetting(){

    se_config->msspeed = !se_config->msspeed;
    char arkSettingsPath[ARK_PATH_SIZE];
    strcpy(arkSettingsPath, ark_config->arkpath);
    strcat(arkSettingsPath, "SETTINGS.TXT");
    std::stringstream final_str;
    std::ifstream fs_in(arkSettingsPath);
    if (!fs_in) {
        final_str << "Cannot open: " << "SETTINGS.TXT";
        save_status = final_str.str().c_str();
        status_frame_count = 100;
        return;
    }
//			fs.open(arkSettingsPath);

    std::string line = "";
    std::string replace_str = "";
    std::string search_str = "mscache";
    std::stringstream updated_content;

    while (std::getline(fs_in, line)) {
        if (line.find(search_str) != std::string::npos) {
            int size = line.find(search_str) + search_str.length() + 2;
            std::string status = line.substr(line.find_last_of(" ")+1);

            if (status == "on") {
                line.replace(size, status.length(), "off");
            }
            else if (status == "off"){
                line.replace(size, status.length(), "on");
            }


        
            
            final_str << "Saved Settings!";

            save_status = final_str.str().c_str();
            status_frame_count = 100;

        }
            updated_content << line << std::endl;
        

    }

    fs_in.close();

    std::ofstream fs_out(arkSettingsPath);
    if (!fs_out) {
        final_str << "Cannot open: " << "SETTINGS.TXT";
        save_status = final_str.str().c_str();
        status_frame_count = 100;
        return;
    }

    fs_out << updated_content.str();

    fs_out.close();
    /*fs_out.close();
    std::remove(arkSettingsPath);
    std::rename(tempArkSettingsPath, arkSettingsPath);
    */
}