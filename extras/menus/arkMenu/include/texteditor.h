#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include "optionsmenu.h"
#include "settingsmenu.h"

typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[3];
} text_line_t;

class TextEditor : public OptionsMenu{

    private:
        string path;
        string clipboard;
        int lines_max;

        SettingsTable table;
        SettingsMenu* menu;

        void loadTextFile();
        void saveTextFile();
        void addLine(string line, char* opt1, char* opt2, char* opt3);
        void insertLine(int i, string line, char* opt1, char* opt2, char* opt3);
        void removeLine(int i);
        void editLine(int i);
    
    public:
        TextEditor(string path);
        ~TextEditor();
        
        void draw();
        
        int control();

};

#endif