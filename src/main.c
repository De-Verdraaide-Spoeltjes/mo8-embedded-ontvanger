#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "xil_printf.h"
#include "xgpio.h"
#include <stdbool.h>

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

void statusLED();

int main()
{
    init_platform();
    print("Starting embedded application\n\r");
    XGpio_Initialize(&leds, STATUS_LED_DEVICE_ID);
    XGpio_Initialize(&buttons, BTNS_DEVICE_ID);
    XGpio_SetDataDirection(&leds, 1, 0x0);
    XGpio_SetDataDirection(&buttons, 1, 0x1);

    XStatus status;

    // In debug mode, generate non-random RSA keys for testing
    #ifdef DEBUG
        generateRSAKeys(&RSAData, 1);
    #endif

    status = initKeyTransmitter(&RSAData);
    if (status != XST_SUCCESS) {
    	print("Error initializing key transmitter\n\r");
    	cleanup_platform();
		return 0;
    }

    status = initDecryption();
    if (status != XST_SUCCESS) {
    	print("Error initializing decryption\n\r");
    	cleanup_platform();
		return 0;
    }

    status = initDisplay();
    if (status != XST_SUCCESS) {
    	print("Error initializing display\n\r");
    	cleanup_platform();
        return 0;
    }

//    TODO: Add status LED

    print("Embedded application initialized\n\r");

    while(1) {
        statusLED();
        runKeyTransmitter(&RSAData);

        static bool prevButtonState = false;

        bool buttonState = XGpio_DiscreteRead(&buttons, 1) & 0x1;

        if (buttonState && !prevButtonState && buttonState == true) {
            xil_printf("Button pressed\n\r");
            initDisplay();
        }

        prevButtonState = buttonState;
    }

    cleanup_platform();
    return 0;
}


void statusLED() {
    XTime tNow;
    static XTime tOld;
	static bool state;
    XTime_GetTime(&tNow);
    if (tOld + NS_TO_TIME(STATUS_BLINK) * 1000 < tNow) {
        // Toggle status LED
        XGpio_DiscreteWrite(&leds, 1, state << 1);
        state = !state;
        tOld = tNow;
    }
}
