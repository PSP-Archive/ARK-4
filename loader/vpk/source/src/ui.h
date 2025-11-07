#ifndef UI_H
#define UI_H

#include <vita2d.h>

void uiInit(void);
void displayMsg(const char* title, const char* msg);
void updateUi(const char* msg);
void waitCross(void);
vita2d_pgf* uiGetFont(void);

void startDraw(void);
void endDraw(void);
void drawLines(void);
void drawTextCenterColored(int y, const char* text, uint8_t r, uint8_t g, uint8_t b);

#endif // UI_H
