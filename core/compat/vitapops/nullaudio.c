/*
 * This file is part of PRO CFW.
 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <globals.h>

// Prevents crashing on homebrews/menus that use PSP sound (not available for PSX exploits)

typedef struct SceAudioInputParams SceAudioInputParams;

int sceAudioOutput2Reserve(int samplecount){
    return 0;
}

int sceAudioInputBlocking(int samplecount, int freq, void *buf){
    return -1;
}

int sceAudioOutputBlocking(int channel, int vol, void* buf){
    return 0;
}

int sceAudioOutputPannedBlocking(int channel, int leftvol, int rightvol, void* buf){
    return 0;
}

int sceAudioOutput2OutputBlocking(int vol, void* buf){
    return 0;
}

int sceAudioSRCChReserve(int samplecount, int freq, int channels){
    return -1;
}

int sceAudioOneshotOutput(int chanId, int sampleCount, int fmt, int leftVol, int rightVol, void* buf){
    return -1;
}

int sceAudioOutput2Release(void){
    return 0;
}

int sceAudioSRCChRelease(void){
    return 0;
}

int sceAudioChReserve(int channel, int sampleCount, int format){
    return -1;
}

int sceAudioOutput2ChangeLength(int sampleCount){
    return 0;
}

int sceAudioOutput2GetRestSample(void){
    return 0;
}

int sceAudioInput(int sampleCount, int freq, void* buf){
    return -1;
}

int sceAudioChRelease(u32 channel){
    return 0;
}

int sceAudioInputInit(int arg0, int gain, int amodule_reboot_beforerg2){
    return -1;
}

int sceAudioWaitInputEnd(){
    return -1;
}

int sceAudioOutput(u32 chanId, int vol, void* buf){
    return 0;
}

int sceAudioChangeChannelConfig(u32 chanId, int format){
    return -1;
}

int sceAudioPollInputEnd(){
    return 0;
}

int sceAudioGetInputLength(){
    return 0;
}

int sceAudioGetChannelRestLength(u32 chanId){
    return 0;
}

int sceAudioChangeChannelVolume(u32 chanId, int leftVol, int rightVol){
    return 0;
}

int sceAudioSetChannelDataLen(u32 chanId, int sampleCount){
    return -1;
}

int sceAudioSRCOutputBlocking(int vol, void* buf){
    return 0;
}

int sceAudioOutputPanned(u32 chanId, int leftVol, int rightVol, void* buf){
    return 0;
}

int sceAudioInputInitEx(SceAudioInputParams* param){
    return -1;
}

int sceAudioGetChannelRestLen(u32 chanId){
    return 0;
}

int sceAudio_driver_4A0FE97D(u32 a0){
    return 0;
}

int sceAudio_driver_5182B550(u32 a0){
    return 0;
}

int sceAudio_driver_53A4FE20(){
    return 0;
}

int sceAudio_driver_95AE5A2B(u32 a0, u32 a1){
    return 0;
}

int sceAudio_driver_F86DFDD6(u8 a0){
    return 0;
}

int sceAudio_driver_FF298CE7(u32 a0){
    return 0;
}
