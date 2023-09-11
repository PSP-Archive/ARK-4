#include <vitasdk.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "install.h"
#include "io.h"

static int progress = 0;
static int totalProgress = 0; 

vita2d_pgf *pgf;
vita2d_pvf *pvf;

// calculate the total progress
void countProgress() {
	totalProgress = 6; // Copy EBOOT.PBP, PBOOT.PBP, GAME.RIF
					   // PROMOTE, HASH EBOOT, GEN EBOOT SIGNATURE steps
						   
	totalProgress += GetTotalNeededDirectories(); // Directories required to be created.
	totalProgress += CountTree("app0:save"); // Total number of files / dirs in ARK4 savedata.
}

void uiInit() {
	countProgress();
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	pgf = vita2d_load_default_pgf();
	pvf = vita2d_load_default_pvf();
}

void drawTextCenter(int y, char* text) {
	int width = 0;
	int height = 0;
	
	vita2d_pgf_text_dimensions(pgf, 1.0, text, &width, &height);
	
	int x = ((960 / 2) - (width / 2));
	vita2d_pgf_draw_text(pgf, x, y, RGBA8(255,255,255,255), 1.0f, text);
}

void drawProgress() {	
	int end = 900;
	int start = 60;
	int y = 300;
	int barPx = (int)floor(((float)progress / (float)totalProgress) * (float)(end - start));
	int percent = (int)floor(((float)progress / (float)totalProgress) * 100.0);
	char percentText[0x100];
	
	vita2d_draw_line(start, y, end, y, RGBA8(128,128,128,255));
	vita2d_draw_line(start, y, start + barPx, y, RGBA8(0,255,0,255));
	
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
	vita2d_pgf_draw_text(pgf, 20, 30, RGBA8(255,255,255,255), 1.0f, "FasterARK");
	vita2d_draw_line(0, 64, 960, 64, RGBA8(255,255,255,255));
	vita2d_draw_line(0, 544-64, 960, 544-64, RGBA8(255,255,255,255));
}


void displayMsg(char* title, char* msg) {
	startDraw();
	drawLines();
	
	drawTextCenter(190, title);
	drawTextCenter(230, msg);
	
	endDraw();	
}

void updateUi(char* msg) {
	progress++;
	
	startDraw();
	drawLines();
	
	drawTextCenter(190, "Installing ...");
	drawTextCenter(230, msg);
	drawProgress();
	
	endDraw();
}