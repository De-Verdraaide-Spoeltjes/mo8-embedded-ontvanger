#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "defines.h"
#include "generate_rsa_keys.h"
#include "rsa_key_transmitter.h"
#include "rsa_decryption.h"
#include "display.h"

rsaData RSAData;

int main()
{
    init_platform();
    print("Starting embedded application\n\r");

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

    while(1);

    cleanup_platform();
    return 0;
}
