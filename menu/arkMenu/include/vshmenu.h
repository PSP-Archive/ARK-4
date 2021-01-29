#ifndef VSHMENU_H
#define VSHMENU_H

#include "common.h"
#include "entry.h"
#include "pluginmgr.h"
#include "peops_menu.h"
#include "system_entry.h"

class VSHMenu : public SystemEntry{

    private:
    
        enum {MAIN_MENU, PLUGINS_MENU, PEOPS_MENU};
    
        PluginsManager plugins;
        PeopsMenu peops_menu;
    
        int animation;
        int w, h, x, y;
        int index;
        
        string* customText;
        int ntext;
        
        int state;
        
        bool changed;
    
    public:
    
        VSHMenu();
        ~VSHMenu();
    
        void setCustomText(string text[], int n);
        void unsetCustomText();
    
        void draw();
        
        void control(Controller* pad);
        
        void pause();
        
        void resume();
        
        char* getInfo(){
            return "System Settings";
        }
        
        Image* getIcon(){
            return common::getImage(IMAGE_SETTINGS);
        }
        
        char* getName(){
            return "Settings";
        }
        
        PluginsManager* getPluginsManager();

};

#endif
