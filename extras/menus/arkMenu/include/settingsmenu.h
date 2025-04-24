#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include "common.h"
#include "entry.h"
#include "system_entry.h"

typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[];
} settings_entry;

typedef struct {
    settings_entry** settings_entries;
    int max_options;
    int changed;
} SettingsTable;

class SettingsMenu : public SystemEntry{

    private:
    
        int animation;
        int w, h, x, y;
        int index, start;
        
        string* customText;
        int ntext;
        bool shorten_paths;
        bool show_all_opts;
        bool show_info;
        
        bool changed;
        
        SettingsTable* table;
        int max_height;
        
        string info;
        string footer;
        string name;
        
        int icon;

        TextScroll scroll;
        TextScroll scroll2;

        void (*save_callback)();
        void (*open_callback)();
        void (*close_callback)();
        void (*reset_callback)();
        
    public:
    
        SettingsMenu(SettingsTable* table, bool shorten_paths, bool show_all_opts, bool show_info);
        ~SettingsMenu();

        void setCallbacks(void (*save_callback)(), void (*open_callback)(), void (*close_callback)(), void (*reset_callback)());
    
        void setCustomText(string text[], int n);
        void unsetCustomText();
    
        void setIcon(int icon){
            this->icon = icon;
        }

        void draw();
        
        void control(Controller* pad);
        
        void pause();
        
        void resume();
        
        void setInfo(string info){
            this->info = info;
        }

        void setFooter(string footer) {
            this->footer = footer;
        }
        
        void setName(string name){
            this->name = name;
        }
        
        string getInfo(){
            return this->info;
        }

        string getFooter() {
            return this->footer;
        }
        
        Image* getIcon(){
            return common::getImage(icon);
        }
        
        string getName(){
            return this->name;
        }

        bool isStillLoading(){ return false; }
        
        void applyConf();
        void readConf();

        int getIndex();
        
};

#endif
