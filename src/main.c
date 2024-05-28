#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "xil_printf.h"
#include "xgpio.h"
#include <stdbool.h>
#include <stdio.h>
#include "sleep.h"
#include "libs/GFX/GFX.h"

#include "defines.h"
#include "generate_rsa_keys.h"
#include "rsa_key_transmitter.h"
#include "rsa_decryption.h"
#include "display.h"


#define STATUS_LED_DEVICE_ID    XPAR_CONNECTION_EMBEDDED_STATUS_LED_DEVICE_ID

rsaData RSAData;
XGpio leds;
char code_buffer[CODE_LENGTH] = {DEFAULT_CODE, DEFAULT_CODE, DEFAULT_CODE, DEFAULT_CODE};

void DisplayCode(char* buffer, uint8_t length);
void statusLED();
void displayData(bool decrypting);

void Initialize() {
	// In debug mode, generate non-random RSA keys for testing
	#ifdef DEBUG
		generateRSAKeys(&RSAData, 1);
	#endif

	XGpio_DiscreteWrite(&leds, 1, 0x3);

	XStatus status, init_state = XST_SUCCESS;
	status = initKeyTransmitter(&RSAData);
	if (status != XST_SUCCESS) {
		print("Error initializing key transmitter\n\r");
	}
	init_state |= status;

	status = initDecryption();
	if (status != XST_SUCCESS) {
		print("Error initializing decryption\n\r");
	}
	init_state |= status;

	status = initDisplay();
	if (status != XST_SUCCESS) {
		print("Error initializing display\n\r");
	}
	init_state |= status;

	if (init_state == XST_SUCCESS) {
		XGpio_DiscreteWrite(&leds, 1, 0x2);
		print("Embedded application initialized\n\r");
	} else {
		XGpio_DiscreteWrite(&leds, 1, 0x1);
		usleep(500000);
		Initialize();
	}
}


int main()
{
    init_platform();
	print("Starting embedded application\n\r");
	XGpio_Initialize(&leds, STATUS_LED_DEVICE_ID);
	XGpio_SetDataDirection(&leds, 1, 0x0);

	// Initialize
	Initialize();

	// display code
	DisplayCode(code_buffer, CODE_LENGTH);

	print("Entering main loop\n\r");

    while(1) {
        statusLED();
        runKeyTransmitter(&RSAData);
        bool decrypting = decryption(&RSAData, code_buffer);

        displayData(decrypting);
    }

    cleanup_platform();
    return 0;
}

void displayData(bool decrypting) {
	bool updateDisplay = false;
	// Display 'decrypting' if decryption is in progress, only if the code has changed
	static bool decrypting_old = false;
	if (decrypting != decrypting_old || updateDisplay) {
		if (decrypting) {
			DrawText("Decrypting...", 0, 0, Font_small, Text_start_left, WHITE);
		} else {
			DrawText("Decrypting...", 0, 0, Font_small, Text_start_left, BLACK);
		}
		decrypting_old = decrypting;
		updateDisplay = true;
	}

	// Display code if it has changed
	static uint8_t buffer_old[CODE_LENGTH];
	if (memcmp(code_buffer, buffer_old, CODE_LENGTH) != 0 || updateDisplay) {
		DisplayCode(code_buffer, CODE_LENGTH);
		memcpy(buffer_old, code_buffer, CODE_LENGTH);
		updateDisplay = true;
	}


	if (updateDisplay) {
		WriteDisplay();
	}
}

void DisplayCode(char* buffer, uint8_t length) {
	char codeString[CODE_LENGTH + 1];
	memcpy(codeString, buffer, CODE_LENGTH);
	codeString[CODE_LENGTH] = '\0';

	DrawText(codeString, DISPLAY_WIDTH / 2, (DISPLAY_HEIGHT - 18) / 2, Font_large, Text_start_center, WHITE);
}


void statusLED() {
    XTime tNow;
    static XTime tOld;
	static bool state;
    XTime_GetTime(&tNow);
    if (tOld + NS_TO_TIME(STATUS_BLINK) * 1000 < tNow) {
        // Toggle status LED
        XGpio_DiscreteWrite(&leds, 1, state << 2);
        state = !state;
        tOld = tNow;
    }
}
