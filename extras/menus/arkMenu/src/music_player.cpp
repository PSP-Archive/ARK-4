#include "music_player.h"
#include "common.h"
#include "controller.h"
#include "system_mgr.h"

static MP3* current_song = NULL;

MusicPlayer::MusicPlayer(string path){
    this->path = path;
}

MusicPlayer::~MusicPlayer(){
}
        
void MusicPlayer::draw(){
    string info = (MP3::isPaused()? string("||"):string(">"));
    common::getImage(IMAGE_DIALOG)->draw_scale(0, 0, 480, 20);
    common::printText(5, 13, info.c_str(), LITEGRAY, SIZE_MEDIUM, 1, 0);
    common::printText(15, 13, (current_song)? current_song->getFilename() : this->path.c_str(), LITEGRAY, SIZE_MEDIUM, 1, 1);
}
        
int MusicPlayer::control(){
    Controller pad;
    pad.flush();
    
    bool running = true;

    while (MP3::isPlaying()){
        if (current_song != NULL){
            if (this->path != current_song->getFilename()){
                current_song->stop();
            }
            else{
                break;
            }
        }
        sceKernelDelayThread(1000);
    }

    SystemMgr::enterFullScreen();

    if (current_song != NULL && this->path != current_song->getFilename()){
        delete current_song;
        current_song = NULL;
    }

    if (current_song == NULL){
        current_song = new MP3((char*)path.c_str(), false);
        current_song->play();
    }

    while (running && MP3::isPlaying()){
        pad.update();

        if (pad.accept()){
            current_song->pauseResume();
        }
        else if (pad.decline()){
            current_song->stop();
            running = false;
            while (MP3::isPlaying()){
                sceKernelDelayThread(1000);
            }
            delete current_song;
            current_song = NULL;
        }
        else if (pad.triangle() && !MP3::isPaused()){
            running = false;
        }
    }

    pad.flush();
    
    SystemMgr::exitFullScreen();

    return 0;
}

void MusicPlayer::pauseResume(){
    if (current_song != NULL) current_song->pauseResume();
}

bool MusicPlayer::isPlaying(){
    return (current_song != NULL && current_song->isPlaying() && !current_song->isPaused());
}