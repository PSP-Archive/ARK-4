#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <pspsdk.h>
#include <pspctrl.h>

class Controller{

    private:
        SceCtrlData pad;
        
        u32 nowpad, newpad, oldpad;
        int n;

        void *_ksceCtrlReadBufferPositive;

        void clCtrlReadBufferPositive();
    
    public:
    
        Controller();
        ~Controller();
        
        // update controller data
        void update(int ignore=3);
        // wait until there's no input in controller
        void flush();
        
        // generic wait for user input, returns true if cross pressed, false if circle is pressed
        bool wait(void* busy_wait=NULL);

        // Return the current pad data
        u32 get_buttons();
        
        // check for accept or decline from user (depending on the button configuration)
        bool any();
        bool accept();
        bool decline();
        
        // check if the corresponding button has been pressed
        bool up();
        bool down();
        bool left();
        bool right();
        bool cross();
        bool circle();
        bool square();
        bool triangle();
        bool RT();
        bool LT();
        bool start();
        bool select();
        bool home();
        bool volume();
        bool mute();
};
        

#endif
