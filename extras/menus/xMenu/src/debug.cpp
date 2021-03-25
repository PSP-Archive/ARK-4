#include "debug.h"

void debugScreen(const char* text){
	clearScreen(CLEAR_COLOR);
	printTextScreen(0, 0, text, WHITE_COLOR);
	flipScreen();
}


void debugFile(const char* text){
	FILE* fp = fopen("DEBUG.TXT", "a+");
	fwrite(text, 1, strlen(text), fp);
	fclose(fp);
}
