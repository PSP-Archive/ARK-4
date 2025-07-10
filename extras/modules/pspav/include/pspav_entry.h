#ifndef PSPAV_ENTRY
#define PSPAV_ENTRY

typedef enum{
    PAD_USER_CANCEL = -1,
    PAD_NONE = 0,
    PAD_USER_ACCEPT = 1
} PSPAV_PadState;

typedef struct{
    void* priv;
    void* icon1;
    unsigned int size_icon1;
    void* at3data;
    unsigned int size_at3data;
    void (*drawBG)(void*);
    void (*drawIcon)(void*, int, int);
} PSPAVEntry;

typedef struct{
    PSPAV_PadState (*getPadState)();
    void (*clearScreen)(unsigned int);
    void (*flipScreen)();
    void (*flushTexture)(void*);
    void* (*getRawTexture)(void*);
    void* (*createTexture)(int, int);
    void (*freeTexture)(void*);
    void (*drawTexture)(void*, int, int);
    void (*setTextureAlpha)(void*, int);
}PSPAVCallbacks;

#endif