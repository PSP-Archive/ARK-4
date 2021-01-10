#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <psputility.h>
#include <pspdisplay.h>

#include "osk.h"
#include "gfx.h"
#include "common.h"
#include "debug.h"

SceUtilityOskParams* OSK::oskParams = NULL;
unsigned short *OSK::intext = NULL;
unsigned short *OSK::desc = NULL;

SceUtilityOskParams* OSK::initOskEx(int nData, int language)
{
    SceUtilityOskParams* oskParams;

    oskParams = (SceUtilityOskParams*) malloc(sizeof(SceUtilityOskParams));
    if (!oskParams)  return NULL;
    memset(oskParams, 0, sizeof(SceUtilityOskParams));

    oskParams->base.size = sizeof(SceUtilityOskParams);
    if (language >= 0)
       oskParams->base.language = language;
    else
       sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &oskParams->base.language);
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &oskParams->base.buttonSwap);
    oskParams->base.graphicsThread = 17;
    oskParams->base.accessThread = 19;
    oskParams->base.fontThread = 18;
    oskParams->base.soundThread = 16;
    oskParams->base.buttonSwap = !(int)common::getConf()->swap_buttons;
    oskParams->datacount = (1>nData)? 1: nData;
    oskParams->data = (SceUtilityOskData*) malloc(oskParams->datacount * sizeof(SceUtilityOskData));
    if (!oskParams->data) {
       free(oskParams);
       return NULL;
    }

    memset(oskParams->data, 0, oskParams->datacount * sizeof(SceUtilityOskData));
    return oskParams;
}

int OSK::initOskDataEx(SceUtilityOskParams* oskParams, int idx,
                     unsigned short *desc, unsigned short *intext, int textLimit, int linesNumber)
{
    if (!oskParams  ||  idx < 0  ||  idx >= oskParams->datacount)
       return 0;

    unsigned short *outtext = (unsigned short *) malloc((textLimit + 1)*sizeof(unsigned short));
    if (!outtext)
       return 0;

    memset(&oskParams->data[idx], 0, sizeof(SceUtilityOskData));
    oskParams->data[idx].language = PSP_UTILITY_OSK_LANGUAGE_DEFAULT;
    oskParams->data[idx].lines = linesNumber;
    oskParams->data[idx].unk_24 = 1;			    // set to 1
    oskParams->data[idx].desc = desc;
    oskParams->data[idx].intext = intext;
    oskParams->data[idx].outtextlength = textLimit;
    oskParams->data[idx].outtextlimit = textLimit;
    oskParams->data[idx].outtext = outtext;

    return 1;
}

int OSK::activateOskEx(SceUtilityOskParams* oskParams, int waitcycle)
{
    if (!oskParams) return -1;

    if (!OSK::oskParams) {			// There is no active OSK - activate the given one
       OSK::oskParams = oskParams;
       int ret = sceUtilityOskInitStart(oskParams);
       if (ret < 0) OSK::oskParams = NULL;
       return ret;
    }
    else if (OSK::oskParams == oskParams)
       return 0;			// The given OSK is currently active...
    else if (!waitcycle)
       return -2;			// Other OSK is currently active...
    else {					// Wait for other OSK to finish and try again to activate the given one
       while (OSK::oskParams) sceDisplayWaitVblankStart();
       return activateOskEx(oskParams, waitcycle);
    }
}

int OSK::oskIsActiveEx(SceUtilityOskParams* oskParams)
{
    return (oskParams  &&  (oskParams == OSK::oskParams));
}

void OSK::deActivateOskEx(SceUtilityOskParams* oskParams)
{
    if (oskIsActiveEx(oskParams))
       OSK::oskParams = NULL;
}

int OSK::oskGetResultEx(SceUtilityOskParams* oskParams, int idx)
{
    if (!oskParams) return 0;
    else if (idx < 0  ||  idx >= oskParams->datacount)
       return oskParams->base.result;
    else
       return oskParams->data[idx].result;
}

unsigned short* OSK::oskOutTextEx(SceUtilityOskParams* oskParams, int idx)
{
    if (!oskParams  ||  idx < 0  ||  idx >= oskParams->datacount)
       return NULL;
    else
       return oskParams->data[idx].outtext;
}


void OSK::endOskEx(SceUtilityOskParams* oskParams)
{
    if (!oskParams  || !oskIsActiveEx(oskParams))   return;

    deActivateOskEx(oskParams);
    int i;
    for (i = 0; i < oskParams->datacount; i++) {
       free(oskParams->data[i].outtext);
    }
    free(oskParams->data);
    free(oskParams);
}


// class methods


OSK::OSK()
{
}

OSK::~OSK()
{
}

void OSK::init(const char *descStr, const char *initialStr, int textLimit, int linesNumber, int language){
    int i = 0;

    if (intext || desc)   return;			//<-- STAS: Is the static OSK already initialized?

//    intext = (unsigned short *) malloc((textLimit + 1)*sizeof(unsigned short));	//<-- STAS: textLimit ISN'T right!
    intext = (unsigned short *) malloc((strlen(initialStr) + 1)*sizeof(unsigned short));
    if (!intext)
        return;

    desc = (unsigned short *) malloc((strlen(descStr) + 1)*sizeof(unsigned short));
    if (!desc) {
        end();
        return;
    }

    for (i=0; i<=strlen(initialStr); i++)
        intext[i] = (unsigned short)initialStr[i];

    for (i=0; i<=strlen(descStr); i++){
        desc[i] = (unsigned short)descStr[i];
    }

    SceUtilityOskParams* oskParams = initOskEx(1, language);
    if (!oskParams || !initOskDataEx(oskParams, 0, desc, intext, textLimit, linesNumber) || (activateOskEx(oskParams,0)<0)) {
        end();
    }
}

void OSK::loop()
{	
	while (this->isActive()){
		common::clearScreen(CLEAR_COLOR);
		common::getImage(IMAGE_BG)->draw(0, 0);
		this->draw();
		common::flipScreen();
		if (sceUtilityOskGetStatus() == PSP_UTILITY_DIALOG_NONE)
			break;
	}
	common::clearScreen(CLEAR_COLOR);
	common::flipScreen();
	sceDisplayWaitVblankStart();
}

void OSK::draw()
{
    switch(sceUtilityOskGetStatus()){
		case PSP_UTILITY_DIALOG_INIT:			//<-- STAS: sceUtilityOskUpdate should be called only in VISIBLE state!
			sceKernelDelayThread(100000);
			break;
        case PSP_UTILITY_DIALOG_VISIBLE :
        	sceDisplayWaitVblankStart();
        	sceDisplayWaitVblankStart();
        	sceGuFinish();
			sceGuSync(0,0);
            sceUtilityOskUpdate(2);
            break;
        case PSP_UTILITY_DIALOG_QUIT:
            sceUtilityOskShutdownStart();
            break;
        case PSP_UTILITY_DIALOG_FINISHED:		//<-- STAS: nothing to do for the other states
        case PSP_UTILITY_DIALOG_NONE:
        default:
			break;								//<-- STAS END -->
    }
}

int OSK::isActive(){
    return oskIsActiveEx(oskParams);
}

int OSK::getStatus(){
    return sceUtilityOskGetStatus();
}


int OSK::getResult(){
    return oskGetResultEx(oskParams, -1);
}

void OSK::getText(char *text){
    int i, j;
    j = 0;
    if (oskParams)  for(i = 0; oskParams->data[0].outtext[i]; i+=1) {
        if (oskParams->data[0].outtext[i]!='\n' && oskParams->data[0].outtext[i]!='\r'){
            text[j] = (char)(oskParams->data[0].outtext[i]);
            j++;
        }
    }
    text[j] = 0;
}

void OSK::getText(unsigned short *text){
    int i, j;
    j = 0;
    if (oskParams)  for(i = 0; oskParams->data[0].outtext[i]; i++) {
        if (oskParams->data[0].outtext[i]!='\n' && oskParams->data[0].outtext[i]!='\r'){
            text[j] = oskParams->data[0].outtext[i];
            j++;
        }
    }
    text[j] = 0;
}

void OSK::end(){
    if (intext) free(intext);
    if (desc)   free(desc);
    intext  = NULL;
    desc    = NULL;
    endOskEx(oskParams);
    oskParams = NULL;
}
