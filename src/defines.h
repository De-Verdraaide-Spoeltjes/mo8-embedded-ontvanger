#ifndef SRC_DEFINES_H
#define SRC_DEFINES_H

#include <stdint.h>

#define DEBUG

typedef struct RSAData {
	uint64_t publicKey;
	uint64_t privateKey;
    uint32_t primes_p, primes_q;
	uint32_t modulus;
} rsaData;

#define CODE_LENGTH 4

#define DEFAULT_CODE    '_'
#define CODE_ERROR  	'.'
#define CODE_MAX_DIGIT  '9'
#define CODE_MIN_DIGIT  '0'


#define STATUS_BLINK 200 //ms

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define I2C_SLAVE_DEVICE_ADDRESS 0x3C


#define TIME_TO_NS_DIVIDER 325 //XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2000000
#define TIME_TO_NS(i) (i / TIME_TO_NS_DIVIDER)
#define NS_TO_TIME(i) (i * TIME_TO_NS_DIVIDER)

enum displayAlignment 
{
    Text_start_left, 
    Text_start_center, 
    Text_start_right
};

enum displayFontSelect 
{
    Font_small, 
    Font_medium, 
	Font_medium_large,
    Font_large
};

#endif
