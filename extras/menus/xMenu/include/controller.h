#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <pspsdk.h>
#include <pspctrl.h>

class Controller{

    private:
        SceCtrlData pad;
        
        u32 nowpad, newpad, oldpad;
        
        int n;
    
    public:
    
        Controller();
        ~Controller();
        
        // update controller data
        void update();
        // wait until there's no input in controller
        void flush();
        
        // generic wait for user input, returns true if cross pressed, false if circle is pressed
        bool wait(void* busy_wait=NULL);

        // wrapper for x/o swap
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
};
        

#endif
