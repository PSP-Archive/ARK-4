#ifndef VSHMENU_H
#define VSHMENU_H

#include "common.h"
#include "entry.h"
#include "system_entry.h"

class VSHMenu : public SystemEntry{

    private:
    
        int animation;
        int w, h, x, y;
        int index;
        
        string* customText;
        int ntext;
        
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
        
        string getInfo(){
            return "System Settings";
        }
        
        Image* getIcon(){
            return common::getImage(IMAGE_SETTINGS);
        }
        
        char* getName(){
            return "Settings";
        }
        
};

#endif
