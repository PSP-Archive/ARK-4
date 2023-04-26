#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include "optionsmenu.h"
#include "ya2d++.h"

class ImageViewer : public OptionsMenu{

    private:
        int x, y;
        int color_index;
        int zoom;
        Image* img;
    
    public:
        ImageViewer(string path);
        ~ImageViewer();
        
        void draw();
        
        int control();

};

#endif