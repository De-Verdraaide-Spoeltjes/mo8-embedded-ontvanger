#ifndef GFX_H_
#define GFX_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

// #include "images/images.h"
#include "fonts/gfxfont.h"
#include "fonts/Org_01.h"

#define GFX_DEFAULT_FONT Org_01

enum displayPixelColor{BLACK, WHITE, GRAY};

typedef void (*GFX_drawOutputPixel)(uint16_t x, uint16_t y, enum displayPixelColor);
typedef void (*GFX_drawOutputBlock)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

void GFX_init(GFX_drawOutputPixel output, GFX_drawOutputBlock output_block, int width, int height);

int GFX_getWidth(void);
int GFX_getHeight(void);
//uint16_t GFX_RGBtoColor565(uint8_t r, uint8_t g, uint8_t b);
//void Display_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
//void Display_setRotation(uint8_t m);
//uint8_t Display_getRotation(void);


void GFX_fillScreen(uint16_t color);
void GFX_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GFX_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void GFX_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void GFX_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void GFX_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void GFX_drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void GFX_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void GFX_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void GFX_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void GFX_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void GFX_drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void GFX_drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void GFX_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void GFX_drawPixel(int16_t x, int16_t y, uint16_t color);
// void GFX_drawImage(Image* img, int16_t x, int16_t y);
void GFX_drawText(char txt[], int length);
void GFX_setTextWrap(bool wrap);
void GFX_setCursor(int16_t x, int16_t y);
void GFX_setFont(const GFXfont *f);
int16_t GFX_getCursorX(void);
int16_t GFX_getCursorY(void);
void GFX_setTextColor(uint16_t c);
void GFX_getTextBounds(char txt[], int length, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);

#endif /* GFX_H_ */