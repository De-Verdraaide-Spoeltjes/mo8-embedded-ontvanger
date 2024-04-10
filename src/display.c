#include "display.h"

#include "defines.h"
#include "xstatus.h"
#include "sleep.h"
#include "xiicps.h"
#include "xtime_l.h"

#define HEIGHT 64
#define WIDTH 128

#define IIC_DEVICE_ID XPAR_PS7_I2C_0_DEVICE_ID
#define IIC_CLOCK_SPEED 400000
#define IIC_DELAY 100

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define I2C_SLAVE_DEVICE_ADDRESS 0x3C
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

static unsigned char array[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8]; // Display buffer

static void initialize_OLED();                        // Initialize OLED
static void Flush();                                  // Send data
static void writeMulti(uint8_t *src, uint16_t count); // Write multiple bytes

void line(int x1, int x2, int y1, int y2);
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void draw_pixel(int x, int y)
{
    if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT))
    { // Check for boundaries
        return;
    }
    else
    {
        array[x + (y / 8) * WIDTH] |= (1 << (y % 8)); // Store pixel in array
    }
}

void draw_vertical_line(int x, int y1, int y2)
{
    if ((x < 0) || (x >= WIDTH) || (y1 >= HEIGHT) || (y2 < 0))
    { // Check for boundaries
        return;
    }
    else
    {
        if (y1 < 0)
            y1 = 0;
        if (y2 >= HEIGHT)
            y2 = HEIGHT - 1;

        for (int y = y1; y <= y2; y++)
        {
            array[x + (y / 8) * WIDTH] |= (1 << (y % 8)); // Store pixel in array
        }
    }
}

XStatus initDisplay()
{
    // --- init i2c ---
    // Initialize Comms
    XIicPs_Config *config;
    int status;

    config = XIicPs_LookupConfig(IIC_DEVICE_ID);
    if (config == NULL)
    {
        return XST_FAILURE;
    }

    status = XIicPs_CfgInitialize(&iic, config, config->BaseAddress);
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    status = XIicPs_SelfTest(&iic);
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    XIicPs_SetSClk(&iic, IIC_CLOCK_SPEED);

    initialize_OLED();               // Initialize screen
    memset(array, 0, sizeof(array)); // Initialize array with 0s
    draw_pixel(1, 1);                // Store pixel at (x,y) location
    draw_pixel(2, 1);                // Store pixel at (x,y) location
    draw_pixel(4, 1);                // Store pixel at (x,y) location
    draw_pixel(1, 62);               // Store pixel at (x,y) location
    draw_pixel(1, 63);               // Store pixel at (x,y) location
    line(0, 127, 0, 63);             // Draw line

    draw_vertical_line(0, 0, 63); // Draw vertical line

    Flush();                         // Send data

    return XST_SUCCESS;
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

/*Bresenham's line drawing algorithm*/

void line(int x0, int x1, int y0, int y1)
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

    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (; x0 <= x1; x0++)
    {
        if (steep)
        {
            draw_pixel(y0, x0);
        }
        else
        {
            draw_pixel(x0, y0);
        }
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

static void Flush()
{
    // Wire.beginTransmission(I2C_SLAVE_DEVICE_ADDRESS); // Start communication with slave
    // Wire.write(0x00);             // Command stream
    // Wire.write(0x00);             // Set lower column start address for page addressing mode
    // Wire.write(0x10);             // Set higher column start address for page addressing mode
    // Wire.write(0x40);             // Set display start line
    // Wire.endTransmission();       // End communication with slave
    const uint8_t flush_commands[] = {
        COMMAND_MODE,
        SH1106_SETLOWCOLUMN | 0x0,
        SH1106_SETHIGHCOLUMN | 0x0,
        SH1106_SETSTARTLINE | 0x0,
    };

    writeMulti((uint8_t *)flush_commands, sizeof(flush_commands) / sizeof(uint8_t));

    uint8_t height = HEIGHT;
	uint8_t width = WIDTH + 4; 
	uint8_t m_row = 0;
	uint8_t m_col = 2;
	
	height >>= 3;
	width >>= 3;

    uint16_t pos = 0;

    for (uint8_t page = 0; page < height; page++)
    {
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
        for (uint8_t i = 0; i < 8; i++)
        {
            // create buffer
            uint8_t buffer[width + 1];
            memcpy(buffer, data_header, sizeof(data_header));
            memcpy(buffer + 1, array + pos, width);

            // increment position
            pos += width;

            // write buffer
            writeMulti(buffer, width + 1);
        }
    }
}

// starting at the given register
static void writeMulti(uint8_t *src, uint16_t count)
{
    int status;
    status = XIicPs_MasterSendPolled(&iic, src, count, I2C_SLAVE_DEVICE_ADDRESS);
    if (status != XST_SUCCESS)
    {
#ifdef DEBUG
        xil_printf("error writing: \t");
        for (uint8_t i = 0; i < count; i++)
        {
            xil_printf(" %x", src[i]);
        }
        xil_printf("\r\n\r\n");
#endif
        return;
    }

    usleep(IIC_DELAY);
}
