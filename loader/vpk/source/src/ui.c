#include <vitasdk.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <psp2/ctrl.h>
#include "install.h"
#include "io.h"

static int progress = 0;
static int totalProgress = 0;

static vita2d_pgf *pgf = NULL;
static vita2d_pvf *pvf = NULL;

void countProgress() {
    totalProgress = 9; // Copy EBOOT.PBP, PBOOT.PBP, GAME.RIF, PROMOTE, HASH EBOOT, GEN EBOOT SIGNATURE
    totalProgress += GetTotalNeededDirectories(0);
    totalProgress += GetTotalNeededDirectories(1);
    totalProgress += CountTree("app0:save");
    totalProgress += CountTree("app0:psx");
}

void uiInit() {
    countProgress();
    vita2d_init();
    vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
    pgf = vita2d_load_default_pgf();
    pvf = vita2d_load_default_pvf();
}

void drawTextCenter(int y, const char* text) {
    int width = 0, height = 0;
    vita2d_pgf_text_dimensions(pgf, 1.0f, text, &width, &height);
    int x = (960 / 2) - (width / 2);
    vita2d_pgf_draw_text(pgf, x, y, RGBA8(255, 255, 255, 255), 1.0f, text);
}

void drawProgress() {
    int end = 900, start = 60, y = 300;
    int barPx = (int)floor(((float)progress / (float)totalProgress) * (float)(end - start));
    int percent = (int)floor(((float)progress / (float)totalProgress) * 100.0);
    char percentText[0x100];
    vita2d_draw_line(start, y, end, y, RGBA8(128, 128, 128, 255));
    vita2d_draw_line(start, y, start + barPx, y, RGBA8(0, 255, 0, 255));
    snprintf(percentText, sizeof(percentText), "%i%% (%i/%i)", percent, progress, totalProgress);
    drawTextCenter(330, percentText);
}

void endDraw() {
    vita2d_end_drawing();
    vita2d_swap_buffers();
}

void startDraw() {
    vita2d_start_drawing();
    vita2d_clear_screen();
}

void drawLines() {
    vita2d_pgf_draw_text(pgf, 20, 30, RGBA8(255, 255, 255, 255), 1.0f, "FasterARK");
    vita2d_draw_line(0, 64, 960, 64, RGBA8(255, 255, 255, 255));
    vita2d_draw_line(0, 544 - 64, 960, 544 - 64, RGBA8(255, 255, 255, 255));
}

void displayMsg(const char* title, const char* msg) {
    startDraw();
    drawLines();
    drawTextCenter(190, title);
    drawTextCenter(230, msg);
    endDraw();
}

void updateUi(const char* msg) {
    progress++;
    startDraw();
    drawLines();
    drawTextCenter(190, "Installing ...");
    drawTextCenter(230, msg);
    drawProgress();
    endDraw();
}

void waitCross(void) {
    SceCtrlData pad;
    while (1) {
        vita2d_start_drawing();
        vita2d_clear_screen();

        drawTextCenter(240, "Install Complete!");
        drawTextCenter(270, "Press X to continue...");

        endDraw();

        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_CROSS) break;
        sceKernelDelayThread(10000);
    }
}

vita2d_pgf* uiGetFont(void) {
    return pgf;
}
