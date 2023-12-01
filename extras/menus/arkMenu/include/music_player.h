#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include <vector>
#include "optionsmenu.h"
#include "mp3.h"

class MusicPlayer : public OptionsMenu{

    private:
        string path;
        TextScroll scroll;
    
    public:
        MusicPlayer(string path);
        MusicPlayer(vector<string>* playlist);
        ~MusicPlayer();
        
        void draw();
        
        int control();

        static void pauseResume();
        static bool isPlaying();
        static void fullStop();

};

#endif