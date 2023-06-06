#include "image_viewer.h"
#include "controller.h"
#include "system_mgr.h"
#include "common.h"

#define MAX_COLORS 5

static unsigned int colors[MAX_COLORS] = {
    WHITE,
    LITEGRAY,
    GRAY,
    DARKGRAY,
    BLACK
};

ImageViewer::ImageViewer(string path){
    color_index = 0;
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
    ya2d_clear_screen(colors[color_index]);
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

    bool is_moving = false;
    int scroll_speed = 1;

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
        else if (pad.LT()){
            if (color_index > 0) color_index--;
            continue;
        }
        else if (pad.RT()){
            if (color_index < MAX_COLORS-1) color_index++;
            continue;
        }

        is_moving = false;

        if (pad.down()){
            if (y+h > 0) y-=scroll_speed;
            if (scroll_speed<10) scroll_speed++;
            is_moving = true;
        }
        if (pad.up()){
            if (y < 272) y+=scroll_speed;
            if (scroll_speed<10) scroll_speed++;
            is_moving = true;
        }
        if (pad.right()){
            if (x+w > 0) x-=scroll_speed;
            if (scroll_speed<10) scroll_speed++;
            is_moving = true;
        }
        if (pad.left()){
            if (x < 480) x+=scroll_speed;
            if (scroll_speed<10) scroll_speed++;
            is_moving = true;
        }

        if (!is_moving) scroll_speed = 1;
    }

    pad.flush();
    
    SystemMgr::exitFullScreen();

    return 0;
}
