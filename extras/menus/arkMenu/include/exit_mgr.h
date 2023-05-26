#ifndef EXIT_MGR_H
#define EXIT_MGR_H

#include "system_entry.h"
#include "optionsmenu.h"

extern "C"{
    void sctrlKernelExitVSH(void*);
    void scePowerRequestColdReset(int);
    void scePowerRequestStandby();
}

static t_options_entry exit_opts[] = {
    {0, "Exit"},
    {1, "Restart"},
    {2, "Shutdown"},
};

class ExitManager : public SystemEntry{

    /* Options Menu instance, will be drawn by the draw thread if it's different from null */
    OptionsMenu* optionsmenu;

    public:
        void draw(){
            if (optionsmenu) optionsmenu->draw();
        };
        void control(Controller* pad){
            if (optionsmenu){
                int ret = optionsmenu->control();
                switch (ret){
                    case 0:
                        sctrlSESetUmdFile("");
          	            sctrlSESetBootConfFileIndex(MODE_UMD);
                        sctrlKernelExitVSH(NULL);
                        break;
                    case 1:
                        scePowerRequestColdReset(0);
                        break;
                    case 2:
                        scePowerRequestStandby();
                        break;
                }
                OptionsMenu* aux = optionsmenu;
                optionsmenu = NULL;
                delete aux;
            }
            else {
                sctrlSESetUmdFile("");
          	    sctrlSESetBootConfFileIndex(MODE_UMD);
                sctrlKernelExitVSH(NULL);
            }
        };
        void pause(){};
        void resume(){
            if (IS_PSP(common::getArkConfig())){
                optionsmenu = new OptionsMenu("", sizeof(exit_opts)/sizeof(t_options_entry), exit_opts);
            }
            else{
                optionsmenu = NULL;
            }
        };
        std::string getInfo(){return "Exit";};
        void setInfo(std::string info){};
        Image* getIcon(){return common::getImage(IMAGE_EXIT);};
        void setName(std::string name){};
        std::string getName(){return "Exit";};
        bool isStillLoading(){ return false; };
};

#endif
