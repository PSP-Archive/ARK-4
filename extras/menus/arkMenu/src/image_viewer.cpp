#include "image_viewer.h"
#include "controller.h"
#include "system_mgr.h"
#include "common.h"

ImageViewer::ImageViewer(string path){
    img = new Image(path);
    img->swizzle();
    int w = img->getWidth();
    int h = img->getHeight();
    if (w < 480){
        x = (480-w)/2;
    }
    else{
        x = 0;
    }
    if (h < 272){
        y = (272-h)/2;
    }
    else{
        y = 0;
    }
}

ImageViewer::~ImageViewer(){
    delete img;
}
        
void ImageViewer::draw(){
    ya2d_clear_screen(WHITE_COLOR);
    img->draw(x, y);
}
        
int ImageViewer::control(){
    Controller pad;
    pad.flush();
    
    bool running = true;

    SystemMgr::enterFullScreen();

    while (running){
        pad.update();

        if (pad.decline()){
            running = false;
        }
    }

    pad.flush();
    
    SystemMgr::exitFullScreen();

    return 0;
}
