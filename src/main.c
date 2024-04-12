#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "xil_printf.h"
#include "xgpio.h"
#include <stdbool.h>
#include <stdio.h>
#include "sleep.h"

#include "defines.h"
#include "generate_rsa_keys.h"
#include "rsa_key_transmitter.h"
#include "rsa_decryption.h"
#include "display.h"


#define STATUS_LED_DEVICE_ID    XPAR_CONNECTION_EMBEDDED_STATUS_LED_DEVICE_ID
#define BTNS_DEVICE_ID          XPAR_CONNECTION_EMBEDDED_BUTTONS_DEVICE_ID



rsaData RSAData;
XGpio leds, buttons;
char code_buffer[CODE_LENGTH] = {'0', '0', '0', '0'};

void DisplayCode(char* buffer, uint8_t length);
void statusLED();

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
	XGpio_Initialize(&buttons, BTNS_DEVICE_ID);
	XGpio_SetDataDirection(&leds, 1, 0x0);
	XGpio_SetDataDirection(&buttons, 1, 0x1);

	// Initialize
	Initialize();

	// display code
	DisplayCode(code_buffer, CODE_LENGTH);

	print("Entering main loop\n\r");

    while(1) {
        statusLED();
        runKeyTransmitter(&RSAData);
        decryption(&RSAData, code_buffer);

		// Display code if it has changed
		static uint8_t buffer_old[CODE_LENGTH];
		if (memcmp(code_buffer, buffer_old, CODE_LENGTH) != 0) {
			DisplayCode(code_buffer, CODE_LENGTH);
			memcpy(buffer_old, code_buffer, CODE_LENGTH);
		}

        static bool prevButtonState = false;
        bool buttonState = XGpio_DiscreteRead(&buttons, 1) & 0x1;
        if (buttonState && !prevButtonState && buttonState == true) {
            xil_printf("Button pressed\n\r");

			// increment code
			for (int i = CODE_LENGTH - 1; i >= 0; i--) {
				code_buffer[i]++;
				if (code_buffer[i] > '9') {
					code_buffer[i] = '0';
				} else {
					break;
				}
			}
        }
        prevButtonState = buttonState;
    }

    cleanup_platform();
    return 0;
}

void DisplayCode(char* buffer, uint8_t length) {
	char codeString[CODE_LENGTH + 1];
	memcpy(codeString, buffer, CODE_LENGTH);
	codeString[CODE_LENGTH] = '\0';

	DrawText(codeString, DISPLAY_WIDTH / 2, (DISPLAY_HEIGHT - 18) / 2, Font_large, Text_start_center);
	
	WriteDisplay();
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
