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
#include "common.h"

extern string ark_version;
static string save_status;
static int status_frame_count = 0; // a few seconds
                    			   //

SubMenu::SubMenu(Menu* menu) {
    this->index = 0;
    this->menu = menu;
    this->getItems();
}

void SubMenu::getItems() {
    common::loadConf();
    stringstream memoryStickSpeedup;
    memoryStickSpeedup << "Memory Stick Speedup: " << ((se_config->msspeed)? "Enabled" : "Disabled");
    stringstream sort_entries;
    sort_entries << "Sort Games by Name: " << ((common::getConf()->sort_entries)? "Enabled" : "Disabled");
    stringstream skip_gameboot;
    skip_gameboot << "Fast Gameboot: " << ((common::getConf()->fast_gameboot)? "Enabled" : "Disabled");
    stringstream scan_cat;
    scan_cat << "Scan Categories: " << ((common::getConf()->scan_cat)? "Enabled" : "Disabled");
    stringstream swap_buttons;
    swap_buttons << "Swap X/O Buttons: " << ((common::getConf()->swap_buttons)? "Enabled" : "Disabled");

    options[0] = memoryStickSpeedup.str();
    options[1] = sort_entries.str();
    options[2] = skip_gameboot.str();
    options[3] = scan_cat.str();
    options[4] = swap_buttons.str();
    options[5] = "Restart";
    options[6] = "Exit";
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

    // draw ARK version and info
    {
    int dx = ((w-8*ark_version.size())/2);
    fillScreenRect(0x8000ff00, x+dx, y+5, 8*ark_version.size(), 8);
    common::printText(x + dx, y+5, ark_version.c_str());
    }

    // menu items
    int cur_x;
    int cur_y = y + (h-(10*n))/2;
    for (int i=0; i<n; i++){
        cur_x = x + ((w-(8*options[i].size()))/2);
        fillScreenRect(color&0x00FFFFFF, cur_x, cur_y+4, 8*options[i].size(), 8);
        if(i==0)
            common::printText(cur_x, cur_y+4, options[i].c_str());
        else
            common::printText(cur_x, cur_y+5, options[i].c_str());
        if (i == index) {
            static u32 alpha = 0;
            static u32 delta = 5;
            u32 color = RED_COLOR | (alpha<<24);
            
            int tw = min((int)(options[i].size()*8)+8, w);
            fillScreenRect(color, cur_x-4, cur_y+13, tw, 2); // bottom
            fillScreenRect(color, cur_x-4, cur_y+3, tw, 2); // top
            fillScreenRect(color, cur_x-4, cur_y+5, 2, 8); // left
            fillScreenRect(color, cur_x-6+tw, cur_y+5, 2, 8); // right
            
            if(alpha==0) delta = 5;
            else if (alpha == 255) delta = -5;
            alpha += delta;
        }
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
    control.update();
    while(1) {

        updateScreen();

        control.update();
        if (control.decline() || control.triangle())
            break;
        else if (control.accept() || control.left() || control.right()){
            switch (index){
                case 0:
                    changeMsCacheSetting(); getItems(); break;
                case 1:
                case 2:
                case 3:
                case 4:
                    changeSetting(index); getItems(); 
                    break;
                case 5: 
                    menu->fadeOut(); common::rebootMenu(); break;
                case 6: 
                    menu->fadeOut(); sceKernelExitGame(); break;
            }
        }
        else if (control.up()){
            if (index > 0) index--;
        }
        else if (control.down()){
            if (index < (sizeof(options)/sizeof(options[0])-1)) index++;
        }

    }
    control.update();
}

SubMenu::~SubMenu() {}

void SubMenu::changeSetting(int setting){

    std::stringstream final_str;
    
    if(setting == 1)
        common::getConf()->sort_entries = !common::getConf()->sort_entries;
    else if(setting == 2)
        common::getConf()->fast_gameboot = !common::getConf()->fast_gameboot;
    else if(setting == 3)
        common::getConf()->scan_cat = !common::getConf()->scan_cat;
    else if(setting == 4)
        common::getConf()->swap_buttons = !common::getConf()->swap_buttons;

    
        
    common::saveConf();
    final_str << "Saved Settings!";
    save_status = final_str.str().c_str();
    status_frame_count = 100;

}

void SubMenu::changeMsCacheSetting(){

    se_config->msspeed = !se_config->msspeed;
    std::string arkSettingsPath = string(ark_config->arkpath)+ARK_SETTINGS;
    std::stringstream final_str;
    std::ifstream fs_in(arkSettingsPath.c_str());
    if (!fs_in) {
        final_str << "Cannot open: " << arkSettingsPath;
        save_status = final_str.str().c_str();
        status_frame_count = 100;
        return;
    }

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

    std::ofstream fs_out(arkSettingsPath.c_str());
    if (!fs_out) {
        final_str << "Cannot open: " << arkSettingsPath;
        save_status = final_str.str().c_str();
        status_frame_count = 100;
        return;
    }

    fs_out << updated_content.str();

    fs_out.close();
}
