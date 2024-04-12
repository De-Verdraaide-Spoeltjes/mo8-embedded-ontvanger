#include "display.h"
#include "libs/GFX/GFX.h"
#include "defines.h"
#include "xstatus.h"
#include "sleep.h"
#include "xiicps.h"
#include "xtime_l.h"

#include "libs/GFX/fonts/Terminal5x8.h"
#include "libs/GFX/fonts/FreeSans9pt7b.h"
#include "libs/GFX/fonts/FreeSans12pt7b.h"
#include "libs/GFX/fonts/FreeSansBold18pt7b.h"

#define IIC_DEVICE_ID XPAR_PS7_I2C_0_DEVICE_ID
#define IIC_CLOCK_SPEED 400000
#define IIC_DELAY 100


#define DATA_MODE 0x40
#define COMMAND_MODE 0x00

#define SH1106_SETCONTRAST 0x81
#define SH1106_DISPLAYALLON_RESUME 0xA4
#define SH1106_DISPLAYALLON 0xA5
#define SH1106_NORMALDISPLAY 0xA6
#define SH1106_INVERTDISPLAY 0xA7
#define SH1106_DISPLAYOFF 0xAE
#define SH1106_DISPLAYON 0xAF

#define SH1106_SETDISPLAYOFFSET 0xD3
#define SH1106_SETCOMPINS 0xDA

#define SH1106_SETVCOMDETECT 0xDB

#define SH1106_SETDISPLAYCLOCKDIV 0xD5
#define SH1106_SETPRECHARGE 0xD9

#define SH1106_SETMULTIPLEX 0xA8

#define SH1106_SETLOWCOLUMN 0x00
#define SH1106_SETHIGHCOLUMN 0x10

#define SH1106_SETSTARTLINE 0x40

#define SH1106_MEMORYMODE 0x20
#define SH1106_COLUMNADDR 0x21
#define SH1106_PAGEADDR   0x22

#define SH1106_COMSCANINC 0xC0
#define SH1106_COMSCANDEC 0xC8

#define SH1106_SEGREMAP 0xA0

#define SH1106_CHARGEPUMP 0x8D

#define SH1106_EXTERNALVCC 0x1
#define SH1106_SWITCHCAPVCC 0x2

XIicPs iic;

static unsigned char displayData[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8]; // Display buffer

static void initialize_OLED();                        // Initialize OLED
static void writeMulti(uint8_t *src, uint16_t count); // Write multiple bytes

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

static void display_drawPixel(uint16_t x, uint16_t y, enum displayPixelColor pixelColor) {
	// uint32_t pixel = (((uint32_t)y) * DISPLAY_WIDTH) + x;

	//    uint8_t bitmask = 0x80 >> (pixel % 8);
	//    displayData[pixel / 8] |= bitmask;

    uint8_t color = 0;
	switch(pixelColor)
	{
		case BLACK:
			color = 0;
			break;
		case WHITE:
			color = 1;
			break;
		default:
			color = 0;
			break;
	}

    if ((x >= DISPLAY_WIDTH) || (y >= DISPLAY_HEIGHT))
    {
        //out of bounds
        return;
    }

    if (color)
    {
        displayData[x + (y / 8) * DISPLAY_WIDTH] |= (1 << (y & 7));
    }
    else
    {
        displayData[x + (y / 8) * DISPLAY_WIDTH] &= ~(1 << (y & 7));
    }
}

// draw a block to the memory
static void display_drawBlock(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t pixelColor) {
	//if (((x + width) > currentWindow_width) || ((y + height) > currentWindow_height))
	//{
	//	//out of bounds
	//	return;
	//}

    for(uint16_t xp = 0; xp < width; xp++)
    {
        for(uint16_t yp = 0; yp < height; yp++)
        {
        	display_drawPixel(x + xp, y + yp, pixelColor);
        }      
    }
}

void setFontSize(enum displayFontSelect fontSelect) {
    switch(fontSelect)
    {
        case Font_small:
            GFX_setFont(&terminal5x8);	
            break;
            
        case Font_medium:
            GFX_setFont(&FreeSans9pt7b);
            break;

        case Font_medium_large:
        	GFX_setFont(&FreeSans12pt7b);
            break;
            
        case Font_large:
            GFX_setFont(&FreeSansBold18pt7b);
            break;
    }
}

XStatus initDisplay() {
    // --- init i2c ---
    // Initialize Comms
    XIicPs_Config *config;
    int status;

    config = XIicPs_LookupConfig(IIC_DEVICE_ID);
    if (config == NULL)
    {
        #ifdef DEBUG
        xil_printf("Error: XIicPs_LookupConfig\n");
        #endif
        return XST_FAILURE;
    }

    status = XIicPs_CfgInitialize(&iic, config, config->BaseAddress);
    if (status != XST_SUCCESS)
    {
        #ifdef DEBUG
        xil_printf("Error: XIicPs_CfgInitialize\n");
        #endif
        return XST_FAILURE;
    }

    status = XIicPs_SelfTest(&iic);
    if (status != XST_SUCCESS)
    {
        #ifdef DEBUG
        xil_printf("Error: XIicPs_SelfTest\n");
        #endif
        return XST_FAILURE;
    }

    XIicPs_SetSClk(&iic, IIC_CLOCK_SPEED);

    GFX_init(&display_drawPixel, &display_drawBlock, DISPLAY_WIDTH, DISPLAY_HEIGHT);

	GFX_setTextWrap(false);
	GFX_setTextColor(WHITE);

    initialize_OLED();               // Initialize screen
    memset(displayData, 0, sizeof(displayData)); // Initialize array with 0s

    WriteDisplay();                  // Send data

    return XST_SUCCESS;
}

// draw text
void DrawText(const char* text, uint16_t xpos, uint16_t ypos, uint8_t fontSize, uint8_t nullAlignment) {
    setFontSize(fontSize);

	int16_t xtop = 0;
	int16_t ytop = 0;
	uint16_t width = 0;
	uint16_t height = 0;

	int lengthText = strlen(text);
	GFX_getTextBounds((char*)text, lengthText, 0, 0, &xtop, &ytop, &width, &height);
    GFX_setCursor(xpos - xtop - width*nullAlignment/2, ypos - ytop);
    GFX_drawText((char*)text, lengthText);
}

static void initialize_OLED()
{
    const uint8_t init_commmands[] = {
        COMMAND_MODE,
        // Init sequence for 128x64 OLED module
        SH1106_DISPLAYOFF,         // 0xAE
        SH1106_SETDISPLAYCLOCKDIV, // 0xD5
        0x80,                      // the suggested ratio 0x80
        SH1106_SETMULTIPLEX,       // 0xA8
        0x3F,
        SH1106_SETDISPLAYOFFSET, // 0xD3
        0x00,                    // no offset

        SH1106_SETSTARTLINE | 0x0, // line #0 0x40
        SH1106_CHARGEPUMP,         // 0x8D
        0x14,
        SH1106_MEMORYMODE, // 0x20
        0x00,              // 0x0 act like ks0108
        SH1106_SEGREMAP | 0x1,
        SH1106_COMSCANDEC,
        SH1106_SETCOMPINS, // 0xDA
        0x12,
        SH1106_SETCONTRAST, // 0x81
        0xCF,
        SH1106_SETPRECHARGE, // 0xd9
        0xF1,
        SH1106_SETVCOMDETECT, // 0xDB
        0x40,
        SH1106_DISPLAYALLON_RESUME, // 0xA4
        SH1106_NORMALDISPLAY,       // 0xA6

        SH1106_DISPLAYON, //--turn on oled panel
    };

    writeMulti((uint8_t *)init_commmands, sizeof(init_commmands) / sizeof(uint8_t));
}

void WriteDisplay() {
    const uint8_t flush_commands[] = {
        COMMAND_MODE,
        SH1106_SETLOWCOLUMN | 0x0,
        SH1106_SETHIGHCOLUMN | 0x0,
        SH1106_SETSTARTLINE | 0x0,
    };

    writeMulti((uint8_t *)flush_commands, sizeof(flush_commands) / sizeof(uint8_t));

    uint8_t height = DISPLAY_HEIGHT;
	uint8_t width = DISPLAY_WIDTH + 4;
	uint8_t m_row = 0;
	uint8_t m_col = 2;
	
	height >>= 3;
	width >>= 3;

    uint16_t pos = 0;

    for (uint8_t page = 0; page < height; page++) {
        // set page address
        const uint8_t page_commands[] = {
            COMMAND_MODE,
            0xB0 + page + m_row,      //set page address
            0x10 | (m_col >> 4),   //set higher column address
            m_col & 0xf,           //set lower column address
        };
        writeMulti((uint8_t *)page_commands, sizeof(page_commands) / sizeof(uint8_t));

        // write data
        const uint8_t data_header[] = {
            DATA_MODE
        };

        // write rows of page
        for (uint8_t i = 0; i < 8; i++) {
            // create buffer
            uint8_t buffer[width + 1];
            memcpy(buffer, data_header, sizeof(data_header) / sizeof(uint8_t));
            memcpy(buffer + 1, displayData + pos, width);

            // increment position
            pos += width;

            // write buffer
            writeMulti(buffer, sizeof(buffer) / sizeof(uint8_t));
        }
    }

    // reset memory
    memset(displayData, 0, sizeof(displayData));
}

// starting at the given register
static void writeMulti(uint8_t *src, uint16_t count) {
    int status;
    status = XIicPs_MasterSendPolled(&iic, src, count, I2C_SLAVE_DEVICE_ADDRESS);
    if (status != XST_SUCCESS) {
#ifdef DEBUG
        xil_printf("error writing: \t");
        for (uint8_t i = 0; i < count; i++) {
            xil_printf(" %x", src[i]);
        }
        xil_printf("\r\n\r\n");
#endif
        return;
    }

    usleep(IIC_DELAY);
}
