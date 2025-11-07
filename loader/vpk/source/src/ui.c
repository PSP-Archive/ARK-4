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
    vita2d_set_clear_color(RGBA8(0x10, 0x10, 0x20, 0xFF)); // Dark blue background
    pgf = vita2d_load_default_pgf();
    pvf = vita2d_load_default_pvf();
}

void drawTextCenter(int y, const char* text) {
    int width = 0, height = 0;
    vita2d_pgf_text_dimensions(pgf, 1.0f, text, &width, &height);
    int x = (960 / 2) - (width / 2);
    // Draw shadow
    vita2d_pgf_draw_text(pgf, x + 1, y + 1, RGBA8(0, 0, 0, 128), 1.0f, text);
    // Draw main text
    vita2d_pgf_draw_text(pgf, x, y, RGBA8(255, 255, 255, 255), 1.0f, text);
}

void drawTextCenterColored(int y, const char* text, uint8_t r, uint8_t g, uint8_t b) {
    int width = 0, height = 0;
    vita2d_pgf_text_dimensions(pgf, 1.0f, text, &width, &height);
    int x = (960 / 2) - (width / 2);
    // Draw shadow
    vita2d_pgf_draw_text(pgf, x + 1, y + 1, RGBA8(0, 0, 0, 128), 1.0f, text);
    // Draw main text
    vita2d_pgf_draw_text(pgf, x, y, RGBA8(r, g, b, 255), 1.0f, text);
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
    for (int y = 0; y < 64; y++) {
        uint8_t alpha = (uint8_t)(255 * (1.0f - (float)y / 64.0f * 0.7f));
        vita2d_draw_line(0, y, 960, y, RGBA8(0x20, 0x40, 0x80, alpha));
    }

    for (int y = 544 - 64; y < 544; y++) {
        uint8_t alpha = (uint8_t)(255 * (1.0f - (float)(544 - y) / 64.0f * 0.7f));
        vita2d_draw_line(0, y, 960, y, RGBA8(0x20, 0x40, 0x80, alpha));
    }

    vita2d_pgf_draw_text(pgf, 21, 31, RGBA8(0, 0, 0, 128), 1.2f, "FasterARK");
    vita2d_pgf_draw_text(pgf, 20, 30, RGBA8(255, 255, 255, 255), 1.2f, "FasterARK");

    vita2d_draw_line(0, 64, 960, 64, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(0, 544 - 64, 960, 544 - 64, RGBA8(0x40, 0x80, 0xFF, 255));

    vita2d_draw_line(0, 0, 20, 0, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(0, 0, 0, 20, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(940, 0, 960, 0, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(960, 0, 960, 20, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(0, 524, 20, 524, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(0, 524, 0, 544, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(940, 524, 960, 524, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(960, 524, 960, 544, RGBA8(0x60, 0xA0, 0xFF, 255));
}

void displayMsg(const char* title, const char* msg) {
    startDraw();
    drawLines();

    vita2d_draw_rectangle(100, 150, 760, 120, RGBA8(0x20, 0x20, 0x40, 180));
    vita2d_draw_line(100, 150, 860, 150, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(100, 270, 860, 270, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(100, 150, 100, 270, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(860, 150, 860, 270, RGBA8(0x40, 0x80, 0xFF, 255));

    drawTextCenterColored(190, title, 0x40, 0x80, 0xFF);
    drawTextCenter(230, msg);
    endDraw();
}

void updateUi(const char* msg) {
    progress++;
    startDraw();
    drawLines();

    vita2d_draw_rectangle(80, 160, 800, 100, RGBA8(0x15, 0x15, 0x30, 200));
    vita2d_draw_line(80, 160, 880, 160, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(80, 260, 880, 260, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(80, 160, 80, 260, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(880, 160, 880, 260, RGBA8(0x40, 0x80, 0xFF, 255));

    drawTextCenterColored(190, "Installing ...", 0x60, 0xA0, 0xFF);
    drawTextCenter(230, msg);
    drawProgress();
    endDraw();
}

void waitCross(void) {
    SceCtrlData pad;
    while (1) {
        startDraw();
        drawLines();

        vita2d_draw_rectangle(120, 180, 720, 140, RGBA8(0x10, 0x30, 0x10, 220));
        vita2d_draw_line(120, 180, 840, 180, RGBA8(0x00, 0x80, 0x00, 255));
        vita2d_draw_line(120, 320, 840, 320, RGBA8(0x00, 0x80, 0x00, 255));
        vita2d_draw_line(120, 180, 120, 320, RGBA8(0x00, 0x80, 0x00, 255));
        vita2d_draw_line(840, 180, 840, 320, RGBA8(0x00, 0x80, 0x00, 255));

        vita2d_draw_line(150, 240, 170, 260, RGBA8(0x00, 0xFF, 0x00, 255));
        vita2d_draw_line(170, 260, 190, 230, RGBA8(0x00, 0xFF, 0x00, 255));

        drawTextCenterColored(220, "Install Complete!", 0x00, 0xFF, 0x00);
        drawTextCenterColored(260, "Press X to continue...", 0x80, 0xFF, 0x80);

        endDraw();

        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_CROSS) break;
        sceKernelDelayThread(10000);
    }
}

vita2d_pgf* uiGetFont(void) {
    return pgf;
}
