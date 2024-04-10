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

#define STATUS_BLINK 200 //ms
#define STATUS_LED_DEVICE_ID    XPAR_CONNECTION_EMBEDDED_STATUS_LED_DEVICE_ID
#define BTNS_DEVICE_ID          XPAR_CONNECTION_EMBEDDED_BUTTONS_DEVICE_ID



rsaData RSAData;
XGpio leds, buttons;

void demo(uint8_t);
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

	// display demo
    demo(0);

    while(1) {
        statusLED();
        runKeyTransmitter(&RSAData);

        static bool prevButtonState = false;
        static uint8_t counter = 0;

        bool buttonState = XGpio_DiscreteRead(&buttons, 1) & 0x1;

        if (buttonState && !prevButtonState && buttonState == true) {
            xil_printf("Button pressed\n\r");
            counter++;
            demo(counter);
        }
        prevButtonState = buttonState;
    }

    cleanup_platform();
    return 0;
}

void demo(uint8_t value) {
	char demoValue[15];
	sprintf(demoValue, "Small demo %d", (int)value);

	DrawText(demoValue, 0, 0, Font_small, Text_start_left);
	DrawText("Medium", 128, 0, Font_medium, Text_start_right);
	DrawText("Medium large", 128/2, 11, Font_medium_large, Text_start_center);
	DrawText("Large", 128/2, 30, Font_large, Text_start_center);

	WriteDisplay();                  // Send data
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
