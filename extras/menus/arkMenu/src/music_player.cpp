#include "music_player.h"
#include "common.h"
#include "controller.h"
#include "system_mgr.h"

MusicPlayer::MusicPlayer(string path){
    this->path = path;
}

MusicPlayer::~MusicPlayer(){
    delete sound;
}
        
void MusicPlayer::draw(){
    string info = (sound->isPaused()? string("||"):string(">"));
    common::getImage(IMAGE_DIALOG)->draw_scale(0, 0, 480, 20);
    common::printText(5, 13, this->path.c_str(), LITEGRAY, SIZE_MEDIUM, 1, 0);
    common::printText(15, 13, this->path.c_str(), LITEGRAY, SIZE_MEDIUM, 1, 1);
}
        
int MusicPlayer::control(){
    Controller pad;
    pad.flush();
    
    bool running = true;

    SystemMgr::enterFullScreen();

    sound = new MP3((char*)path.c_str(), false);
    sound->play();

    while (running && sound->isPlaying()){
        pad.update();

        if (pad.accept()){
            sound->pauseResume();
        }
        else if (pad.decline()){
            sound->stop();
            running = false;
        }
    }

    pad.flush();
    
    SystemMgr::exitFullScreen();

    return 0;
}