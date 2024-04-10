#ifndef SRC_DISPLAY_H
#define SRC_DISPLAY_H

#include "xstatus.h"

XStatus initDisplay();
void WriteDisplay();
void DrawText(const char* text, uint16_t xpos, uint16_t ypos, uint8_t fontSize, uint8_t nullAlignment);

#endif
