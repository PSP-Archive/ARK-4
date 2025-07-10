#ifndef PSPAV_H
#define PSPAV_H

#ifdef __cplusplus
extern "C" {
#endif

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
    void (*drawTextureBlend)(void*, int, int);
    void (*setTextureAlpha)(void*, int);
}PSPAVCallbacks;

// AT3

void pspavSetAt3Data(char* data, int size, int* abortVar, int delay);
void pspavResetAt3Data();
int pspavPlayAT3(unsigned int argc, void* argv);

// PMF
unsigned char pspavPlayGamePMF(PSPAVEntry* e, PSPAVCallbacks* callbacks, int x, int y);
void pspavPlayVideoFile(const char* path, PSPAVCallbacks* callbacks);

#ifdef __cplusplus
}
#endif

#endif