#include "GFX.h"
#include <string.h>

#ifndef _swap_int16_t
	#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

static GFX_drawOutputPixel Draw_output;
static GFX_drawOutputBlock Draw_output_block;

static int Output_height;
static int Output_width;
static int Cursor_x;
static int Cursor_y;
static bool Wrap_text = false;
static uint16_t Text_color;
static int16_t Text_size = 1;
static const GFXfont* Display_font;

void GFX_init(GFX_drawOutputPixel output, GFX_drawOutputBlock output_block, int width, int height)
{
	Draw_output = output;
	Draw_output_block = output_block;
	Output_width = width;
	Output_height = height;
	Text_color = 1;
	GFX_setFont(&GFX_DEFAULT_FONT);
}

// void GFX_drawImage(Image* img, int16_t x, int16_t y)
// {

// 	//TODO IMAGE WRITER
	
// 	//Display_setAddrWindow(x, y, x + img->witdth - 1, y + img->height - 1);
// 	//SPI_WriteBoolColorData(img->bitmap, img->witdth * img->height, img->imgColor, img->bgColor);
// }

int GFX_getWidth()
{
	return Output_width;
}

int GFX_getHeight()
{
	return Output_height;
}

/*
uint16_t GFX_RGBtoColor565(uint8_t r, uint8_t g, uint8_t b) 
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
*/

void GFX_fillScreen(uint16_t color)
{
	GFX_fillRect(0, 0,  Output_width, Output_height, color);
}

void GFX_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= Output_width) || (y >= Output_height)) 
	{
		return;
	}
	if((x + w - 1) >= Output_width)  
	{
		w = Output_width - x;
	}
	if((y + h - 1) >= Output_height) 
	{
		h = Output_height - y;
	}
	
	(*Draw_output_block)(x, y, w, h, color);
}

void GFX_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
	GFX_drawHLine(x, y, w, color);
	GFX_drawHLine(x, y+h-1, w, color);
	GFX_drawVLine(x, y, h, color);
	GFX_drawVLine(x+w-1, y, h, color);
}

void GFX_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) 
{
	// smarter version
	GFX_drawHLine(x+r  , y    , w-2*r, color); // Top
	GFX_drawHLine(x+r  , y+h-1, w-2*r, color); // Bottom
	GFX_drawVLine(x    , y+r  , h-2*r, color); // Left
	GFX_drawVLine(x+w-1, y+r  , h-2*r, color); // Right
	// draw four corners
	GFX_drawCircleHelper(x+r    , y+r    , r, 1, color);
	GFX_drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
	GFX_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
	GFX_drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

void GFX_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) 
{
	// smarter version
	GFX_fillRect(x+r, y, w-2*r, h, color);

	// draw four corners
	GFX_fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
	GFX_fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

void GFX_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	GFX_drawPixel(x0  , y0+r, color);
	GFX_drawPixel(x0  , y0-r, color);
	GFX_drawPixel(x0+r, y0  , color);
	GFX_drawPixel(x0-r, y0  , color);

	while (x<y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		GFX_drawPixel(x0 + x, y0 + y, color);
		GFX_drawPixel(x0 - x, y0 + y, color);
		GFX_drawPixel(x0 + x, y0 - y, color);
		GFX_drawPixel(x0 - x, y0 - y, color);
		GFX_drawPixel(x0 + y, y0 + x, color);
		GFX_drawPixel(x0 - y, y0 + x, color);
		GFX_drawPixel(x0 + y, y0 - x, color);
		GFX_drawPixel(x0 - y, y0 - x, color);
	}
}

void GFX_drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) 
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;
		if (cornername & 0x4) 
		{
			GFX_drawPixel(x0 + x, y0 + y, color);
			GFX_drawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) 
		{
			GFX_drawPixel(x0 + x, y0 - y, color);
			GFX_drawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) 
		{
			GFX_drawPixel(x0 - y, y0 + x, color);
			GFX_drawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) 
		{
			GFX_drawPixel(x0 - y, y0 - x, color);
			GFX_drawPixel(x0 - x, y0 - y, color);
		}
	}
}

void GFX_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
	GFX_drawVLine(x0, y0-r, 2*r+1, color);
	GFX_fillCircleHelper(x0, y0, r, 3, 0, color);
}

void GFX_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) 
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;

		if (cornername & 0x1) 
		{
			GFX_drawVLine(x0+x, y0-y, 2*y+1+delta, color);
			GFX_drawVLine(x0+y, y0-x, 2*x+1+delta, color);
		}
		if (cornername & 0x2) 
		{
			GFX_drawVLine(x0-x, y0-y, 2*y+1+delta, color);
			GFX_drawVLine(x0-y, y0-x, 2*x+1+delta, color);
		}
	}
}

void GFX_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) 
{
	GFX_drawLine(x0, y0, x1, y1, color);
	GFX_drawLine(x1, y1, x2, y2, color);
	GFX_drawLine(x2, y2, x0, y0, color);
}

void GFX_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1)
	{
		_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
	}
	if (y1 > y2)
	{
		_swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
	}
	if (y0 > y1)
	{
		_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
	}

	if(y0 == y2)
	{ // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if(x1 < a)      a = x1;
		else if(x1 > b) b = x1;
		if(x2 < a)      a = x2;
		else if(x2 > b) b = x2;
		GFX_drawHLine(a, y0, b-a+1, color);
		return;
	}

	int16_t
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1;
	int32_t
	sa   = 0,
	sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if(y1 == y2)
	{
		last = y1;   // Include y1 scanline
	}
	else
	{
		last = y1-1; // Skip it
	}

	for(y=y0; y <= last; y++)
	{
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) 
		{
			_swap_int16_t(a,b);
		}
		GFX_drawHLine(a, y, b-a+1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y<=y2; y++)
	{
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) 
		{
			_swap_int16_t(a,b);
		}
		GFX_drawHLine(a, y, b-a+1, color);
	}
}

void GFX_drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if((x >= Output_width) || (y >= Output_height)) 
	{
		return;
	}
	if((x + w - 1) >= Output_width)  
	{
		w = Output_width - x;
	}
	
	
	(*Draw_output_block)(x, y, w, 1, color);
}

void GFX_drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if((x >= Output_width) || (y >= Output_height)) 
	{
		return;
	}
	if((y + h - 1) >= Output_height) 
	{
		h = Output_height - y;
	}

	(*Draw_output_block)(x, y, 1, h, color);
}

void GFX_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) 
	{
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) 
	{
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
		} else {
		ystep = -1;
	}

	for (; x0<=x1; x0++) 
	{
		if (steep) 
		{
			GFX_drawPixel(y0, x0, color);
		} 
		else 
		{
			GFX_drawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) 
		{
			y0 += ystep;
			err += dx;
		}
	}
}

void GFX_drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if((x < 0) ||(x >= Output_width) || (y < 0) || (y >= Output_height)) 
	{
		return;
	}

	(*Draw_output)(x, y, color);
}

void GFX_setCursor(int16_t x, int16_t y)
{
	Cursor_x = x;
	Cursor_y = y;
}

int16_t GFX_getCursorX()
{
	return Cursor_x;
}

int16_t GFX_getCursorY()
{
	return Cursor_y;
}


void GFX_setTextColor(uint16_t c)
{
	Text_color = c;
}

void GFX_setFont(const GFXfont *f)
{
	Display_font = f;
}

void GFX_setTextWrap(bool wrap)
{
	Wrap_text = wrap;
}

static void Display_drawChar(int16_t x, int16_t y, uint8_t c, uint16_t color, uint8_t size)
{
	// Character is assumed previously filtered by write() to eliminate
	// newlines, returns, non-printable characters, etc.  Calling drawChar()
	// directly with 'bad' characters of font may cause mayhem!

	c -= Display_font->first;
	GFXglyph *glyph  = &((Display_font->glyph)[c]);
	uint8_t  *bitmap = Display_font->bitmap;

	uint16_t bo = glyph->bitmapOffset;
	uint8_t  w  = glyph->width,
	h  = glyph->height;
	int8_t xo = glyph->xOffset,
	yo = glyph->yOffset;
	uint8_t bits = 0;
	uint8_t bit = 0;
	int16_t  xo16 = 0;
	int16_t yo16 = 0;

	if(size > 1) {
		xo16 = xo;
		yo16 = yo;
	}

	// Todo: Add character clipping here

	// NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
	// THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
	// has typically been used with the 'classic' font to overwrite old
	// screen contents with new data.  This ONLY works because the
	// characters are a uniform size; it's not a sensible thing to do with
	// proportionally-spaced fonts with glyphs of varying sizes (and that
	// may overlap).  To replace previously-drawn text when using a custom
	// font, use the getTextBounds() function to determine the smallest
	// rectangle encompassing a string, erase the area with fillRect(),
	// then draw new text.  This WILL infortunately 'blink' the text, but
	// is unavoidable.  Drawing 'background' pixels will NOT fix this,
	// only creates a new set of problems.  Have an idea to work around
	// this (a canvas object type for MCUs that can afford the RAM and
	// displays supporting setAddrWindow() and pushColors()), but haven't
	// implemented this yet.

	for(uint8_t yy = 0; yy < h; yy++) 
	{
		for(uint8_t xx = 0; xx < w; xx++) 
		{
			if(!(bit++ & 7)) 
			{
				bits = bitmap[bo++];
			}
			if(bits & 0x80) 
			{
				if(size == 1) 
				{
					GFX_drawPixel(x+xo+xx, y+yo+yy, color);
				} 
				else 
				{
					GFX_fillRect(x+(xo16+xx)*size, y+(yo16+yy)*size, size, size, color);
				}
			}
			bits <<= 1;
		}
	}

}

void GFX_drawText(char txt[], int length)
{
	for (int i = 0; i < length; i++)
	{
		char c = txt[i];
		if(c == '\n')
		{
			Cursor_x  = 0;
			Cursor_y += Text_size * Display_font->yAdvance;
		}
		else if(c != '\r')
		{
			uint8_t first = Display_font->first;

			if((c >= first) && (c <= Display_font->last))
			{
				uint8_t   c2    = c - Display_font->first;
				GFXglyph *glyph = &((Display_font->glyph)[c2]);
				uint8_t   w     = glyph->width,
				h     = glyph->height;
				if((w > 0) && (h > 0))
				{ // Is there an associated bitmap?
					int16_t xo = glyph->xOffset; // sic
					if(Wrap_text && ((Cursor_x + Text_size * (xo + w)) >= Output_width)) {
						// Drawing character would go off right edge; wrap to new line
						Cursor_x  = 0;
						Cursor_y += Text_size * Display_font->yAdvance;
					}
					Display_drawChar(Cursor_x, Cursor_y, c, Text_color, Text_size);
				}
				Cursor_x += glyph->xAdvance * Text_size;
			}
		}
	}
}

// Pass string and a cursor position, returns UL corner and W,H.
void GFX_getTextBounds(char txt[], int length, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) 
{
	uint8_t c; // Current character

	*x1 = x;
	*y1 = y;
	*w  = *h = 0;

	GFXglyph *glyph;
	uint8_t first = Display_font->first,
	last  = Display_font->last,
	gw, gh, xa;
	int8_t    xo, yo;
	int16_t   minx = Output_width, miny = Output_height, maxx = -1, maxy = -1,
	gx1, gy1, gx2, gy2, ts = (int16_t)Text_size,
	ya = ts * Display_font->yAdvance;

	for(int i = 0; i < length; i++)
	{
		c = txt[i];
		if(c != '\n') 
		{ // Not a newline
			if(c != '\r') 
			{ // Not a carriage return, is normal char
				if((c >= first) && (c <= last)) 
				{ // Char present in current font
					c    -= first;
					glyph = &((Display_font->glyph)[c]);
					gw    = glyph->width;
					gh    = glyph->height;
					xa    = glyph->xAdvance;
					xo    = glyph->xOffset;
					yo    = glyph->yOffset;
					if(Wrap_text && ((x + (((int16_t)xo + gw) * ts)) >= Output_width)) 
					{
						// Line wrap
						x  = 0;  // Reset x to 0
						y += ya; // Advance y by 1 line
					}
					gx1 = x   + xo * ts;
					gy1 = y   + yo * ts;
					gx2 = gx1 + gw * ts - 1;
					gy2 = gy1 + gh * ts - 1;
					if(gx1 < minx) minx = gx1;
					if(gy1 < miny) miny = gy1;
					if(gx2 > maxx) maxx = gx2;
					if(gy2 > maxy) maxy = gy2;
					x += xa * ts;
				}
			} // Carriage return = do nothing
		} 
		else 
		{ // Newline
			x  = 0;  // Reset x
			y += ya; // Advance y by 1 line
		}
	}

	// End of string
	*x1 = minx;
	*y1 = miny;
	if(maxx >= minx) 
	{
		*w  = maxx - minx + 1;
	}
	if(maxy >= miny) 
	{
		*h  = maxy - miny + 1;
	}
}
