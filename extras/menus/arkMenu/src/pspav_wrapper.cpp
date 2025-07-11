#include <pspgu.h>
#include "pspav_wrapper.h"
#include "controller.h"
#include "common.h"
#include "entry.h"

static SceUID pspavmod = -1;

static PSPAV_PadState getPadState(){
    Controller pad; pad.update(1);
    if (pad.accept()) return PAD_USER_ACCEPT;
    else if (pad.decline()) return PAD_USER_CANCEL;
    return PAD_NONE;
}

static void* getRawTexture(void* tex){
    ya2d_texture* texture = (ya2d_texture*)tex;
    return texture->data;
}

static void* createTexture(int w, int h){
    return ya2d_create_texture(w, h, GU_PSM_8888, YA2D_PLACE_VRAM);
}

static void setTextureAlpha(void* tex, int has_alpha){
    ya2d_texture* texture = (ya2d_texture*)tex;
    texture->has_alpha = has_alpha;
}

static void PSPAVEntry_drawBG(void* e){
    PSPAVEntry* ave = (PSPAVEntry*)e;
    Entry* entry = (Entry*)ave->priv;
    entry->drawBG();
}

static void PSPAVEntry_drawIcon(void* e, int x, int y){
    PSPAVEntry* ave = (PSPAVEntry*)e;
    Entry* entry = (Entry*)ave->priv;
    entry->getIcon()->draw(x, y);
}

PSPAVCallbacks av_callbacks = {
    &getPadState,
    &common::clearScreen,
    &common::flipScreen,
    (void (*)(void*))&ya2d_flush_texture,
    &getRawTexture,
    &createTexture,
    (void (*)(void*))&ya2d_free_texture,
    (void (*)(void*, int, int))&ya2d_draw_texture,
    &setTextureAlpha,
};

PSPAVEntry convertEntry(Entry* e){
    PSPAVEntry ave;
    ave.priv = e;
    ave.icon1 = e->getIcon1();
    ave.size_icon1 = e->getIcon1Size();
    ave.at3data = e->getSnd();
    ave.size_at3data = e->getSndSize();
    ave.drawBG = PSPAVEntry_drawBG;
    ave.drawIcon = PSPAVEntry_drawIcon;
    return ave;
}
