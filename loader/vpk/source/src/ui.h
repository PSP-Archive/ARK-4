#ifndef UI_H
#define UI_H

#include <vita2d.h>

void uiInit(void);
void displayMsg(const char* title, const char* msg);
void updateUi(const char* msg);
void waitCross(void);
vita2d_pgf* uiGetFont(void);

#endif // UI_H
