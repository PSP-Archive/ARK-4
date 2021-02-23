#ifndef VSHMENU_H
#define VSHMENU_H

#include "common.h"
#include "entry.h"
#include "system_entry.h"

typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[];
} vsh_entry;

class VSHMenu : public SystemEntry{

    private:
    
        int animation;
        int w, h, x, y;
        int index;
        
        string* customText;
        int ntext;
        
        bool changed;
        
        vsh_entry** vsh_entries;
        int max_options;
        int max_height;
        
        string info;
        string name;
        
        void (*callback)();
        
    public:
    
        VSHMenu(vsh_entry**, int, void (*callback)());
        ~VSHMenu();
    
        void setCustomText(string text[], int n);
        void unsetCustomText();
    
        void draw();
        
        void control(Controller* pad);
        
        void pause();
        
        void resume();
        
        void setInfo(string info){
            this->info = info;
        }
        
        void setName(string name){
            this->name = name;
        }
        
        string getInfo(){
            return this->info;
        }
        
        Image* getIcon(){
            return common::getImage(IMAGE_SETTINGS);
        }
        
        string getName(){
            return this->name;
        }
        
        void applyConf();
        void readConf();
        
};

#endif
