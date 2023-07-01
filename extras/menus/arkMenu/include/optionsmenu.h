#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#define OPTIONS_CANCELLED -1

#include "common.h"

typedef struct {
    int value;
    char* name;
} t_options_entry;

class OptionsMenu {

    protected:
        char* description;
        int n_options;
        t_options_entry* entries;
        int index;
        int x, y, w, h;
        TextScroll scroll;
        
        int maxString();
    
    public:
        OptionsMenu(){};
        OptionsMenu(char* description, int n_options, t_options_entry* entries);
        ~OptionsMenu();
        
        virtual void draw();
        
        virtual int control();

};

#endif
