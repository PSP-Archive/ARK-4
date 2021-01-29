#ifndef PEOPS_MENU_H
#define PEOPS_MENU_H

class PeopsMenu {

    private:
        int index;
        
        void processPeopsConf();

    public:
        PeopsMenu();
        ~PeopsMenu();

        void draw();
    
        void control();

};

#endif
