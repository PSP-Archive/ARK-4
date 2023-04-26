#include "image_viewer.h"
#include "controller.h"
#include "system_mgr.h"
#include "common.h"

ImageViewer::ImageViewer(string path){
    zoom = 1;
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
    int w = img->getWidth() * zoom;
    int h = img->getHeight() * zoom;
    img->draw_scale(x, y, w, h);
}
        
int ImageViewer::control(){
    Controller pad;
    pad.flush();
    
    bool running = true;

    SystemMgr::enterFullScreen();

    int w = img->getWidth();
    int h = img->getHeight();

    while (running){
        pad.update();

        if (pad.decline()){
            running = false;
            continue;
        }
        else if (pad.square()){
            if (zoom < 8){
                zoom *= 2;
                w = img->getWidth() * zoom;
                h = img->getHeight() * zoom;
                if (y<0) y = 0;
                else if (y>=272) y = 272;
                if (x<0) x = 0;
                else if (x>=480) x = 480;
            }
            continue;
        }
        else if (pad.triangle()){
            if (zoom > 1){
                zoom /= 2;
                w = img->getWidth() * zoom;
                h = img->getHeight() * zoom;
                if (y<0) y = 0;
                else if (y>=272) y = 272;
                if (x<0) x = 0;
                else if (x>=480) x = 480;
            }
            continue;
        }
        if (pad.up()){
            if (y+h > 0) y--;
        }
        if (pad.down()){
            if (y < 272) y++;
        }
        if (pad.left()){
            if (x+w > 0) x--;
        }
        if (pad.right()){
            if (x < 480) x++;
        }
    }

    pad.flush();
    
    SystemMgr::exitFullScreen();

    return 0;
}
