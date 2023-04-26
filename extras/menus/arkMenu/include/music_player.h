#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include "optionsmenu.h"
#include "mp3.h"

class MusicPlayer : public OptionsMenu{

    private:
        string path;
        MP3* sound;
    
    public:
        MusicPlayer(string path);
        ~MusicPlayer();
        
        void draw();
        
        int control();

};

#endif