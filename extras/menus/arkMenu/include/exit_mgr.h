#ifndef EXIT_MGR_H
#define EXIT_MGR_H

#include "system_entry.h"
#include "system_mgr.h"
#include "optionsmenu.h"
#include "common.h"
#include <systemctrl.h>


extern "C"{
    int scePowerRequestSuspend(void);
    void scePowerRequestColdReset(int);
    void scePowerRequestStandby();
}

static t_options_entry exit_opts[] = {
    {-1, "Cancel"},
    {0, "Exit"},
    {1, "Suspend"},
    {2, "Restart"},
    {3, "Shutdown"},
};

class ExitManager : public SystemEntry{

    /* Options Menu instance, will be drawn by the draw thread if it's different from null */
    OptionsMenu* optionsmenu;
    bool canceled;

    public:
        //ExitManager(){ optionsmenu = NULL; };
        void draw(){
            if (optionsmenu) optionsmenu->draw();
        };
        void control(Controller* pad){
            if (canceled) return;
            common::saveConf();
            if (optionsmenu){
                int ret = optionsmenu->control();
                switch (ret){
                    case -1:
                        canceled = true;
                        break;
                    case 0:
                        break;
                    case 1:
                        canceled = true;
                        scePowerRequestSuspend();
                        break;
                    case 2:
                        scePowerRequestColdReset(0);
                        break;
                    case 3:
                        scePowerRequestStandby();
                        break;
                }
                OptionsMenu* aux = optionsmenu;
                optionsmenu = NULL;
                delete aux;
            }
            if (canceled) {
                SystemMgr::changeMenuState();
            }
            else {
                sctrlSESetUmdFile("");
                sctrlSESetBootConfFileIndex(MODE_UMD);
                sctrlKernelExitVSH(NULL);
            }
        };
        void pause(){};
        void resume(){
            canceled = false;
            int nopts;
            if (IS_PSP(common::getArkConfig())){
                nopts = sizeof(exit_opts)/sizeof(t_options_entry);
            }
            else{
                nopts = 2;
            }
            optionsmenu = new OptionsMenu("", nopts, exit_opts);
        };
        std::string getInfo(){return "Exit";};
        void setInfo(std::string info){};
        void setFooter(std::string footer){};
        std::string getFooter(){return "";};
        Image* getIcon(){return common::getImage(IMAGE_EXIT);};
        void setName(std::string name){};
        std::string getName(){return "Exit";};
        bool isStillLoading(){ return false; };
};

#endif
