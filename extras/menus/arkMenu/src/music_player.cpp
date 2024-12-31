#include "music_player.h"
#include "common.h"
#include "controller.h"
#include "system_mgr.h"

static MP3* current_song = NULL;
static vector<string> playlist = vector<string>();
static int cur_play = 0;

static void mp3_cleanup(MP3* music){
    printf("cleaning up mp3\n");
    if (music == current_song){
       if (cur_play+1 < playlist.size()){
            cur_play++;
            current_song = new MP3((char*)playlist[cur_play].c_str());
            current_song->on_music_end = mp3_cleanup;
            current_song->play();
        }
        else{
            current_song = NULL;
            playlist.clear();
            cur_play = 0;
        }
        
        delete music;
    }
}

static void add_playlist(string path){
    for (int i=0; i<playlist.size(); i++){
        if (playlist[i] == path) return;
    }
    playlist.push_back(path);
}

MusicPlayer::MusicPlayer(string path){
    this->path = path;
    scroll.w = 400;
    if (playlist.size()){
        add_playlist(path);
    }
}

MusicPlayer::MusicPlayer(vector<string>* pl){
    this->path = pl->at(0);
    scroll.w = 400;
    for (int i=0; i<pl->size(); i++){
        add_playlist(pl->at(i));
    }
}

MusicPlayer::~MusicPlayer(){
}
        
void MusicPlayer::draw(){
    string info = (MP3::isPaused()? string("||"):string(">"));
    common::getImage(IMAGE_DIALOG)->draw_scale(0, 0, 480, 20);
    common::printText(5, 13, info.c_str(), LITEGRAY, SIZE_MEDIUM, 1, 0);
    common::printText(15, 13, (current_song)? current_song->getFilename() : this->path.c_str(), LITEGRAY, SIZE_MEDIUM, 1, &scroll);

    if (playlist.size()){
        common::getImage(IMAGE_DIALOG)->draw_scale(20, 30, 450, 235);
        int y = 70;
        
        for (int i=0; i<playlist.size(); i++){
            if (i == cur_play) common::printText(30, y, ">");
            common::printText(40, y, playlist[i].c_str(), LITEGRAY, SIZE_TINY);
            y+=20;
        }
    }
}
        
int MusicPlayer::control(){
    Controller pad;
    pad.flush();
    
    bool running = true;

    while (MP3::isPlaying() && playlist.size() == 0){
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

    MP3::fullStop();

    if (current_song != NULL && this->path != current_song->getFilename() && playlist.size() == 0){
        delete current_song;
        current_song = NULL;
    }

    if(current_song == NULL) {
        current_song = new MP3((char*)path.c_str(), false);
        current_song->on_music_end = mp3_cleanup;
        current_song->play();
    }

    while (running && MP3::isPlaying()){
        pad.update();

        if (pad.accept()){
            current_song->pauseResume();
        }
        else if (pad.decline()){
            playlist.clear();
            cur_play = 0;
            current_song->stop();
            running = false;
            while (MP3::isPlaying()){
                sceKernelDelayThread(1000);
            }
        }
        else if (pad.triangle() && !MP3::isPaused()){
            running = false;
        }
        else if (pad.LT()){
            if (cur_play > 0 && playlist.size()){
                cur_play-=2;
                current_song->stop();
            }
        }
        else if (pad.RT()){
            if (cur_play+1 < playlist.size()){
                current_song->stop();
            }
        }
        if(current_song == NULL && playlist.size() != 0) {
            current_song->stop();
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

void MusicPlayer::fullStop(){
    if (current_song) current_song->on_music_end = NULL;
    while (MP3::isPlaying()){
        MP3::fullStop();
        sceKernelDelayThread(1000);
    }
}