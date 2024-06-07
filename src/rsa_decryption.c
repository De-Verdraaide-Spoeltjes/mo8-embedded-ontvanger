#include "rsa_decryption.h"
#include "xgpio.h"
#include "xstatus.h"
#include <stdlib.h>
#include "xparameters.h"

XGpio dataCom;

enum rsaDecryptionState {
	Decryption_waiting,
	Decryption_getData,
	Decryption_decrypt
};


uint32_t *prime_values;

#define COM_DATA_DEVICE_ID            	XPAR_CONNECTION_EMBEDDED_RSA_COMMUNICATIE_DEVICE_ID
#define COM_DATA_BASE_ADDR           	XPAR_CONNECTION_EMBEDDED_RSA_COMMUNICATIE_BASEADDR
#define ENCRYPTED_DATA_BASE_ADDR     	XPAR_CONNECTION_EMBEDDED_RSA_VERSLEUTELD_KARAKTER_BASEADDR


uint64_t decryptCRT(uint64_t encryptedData, uint64_t privateKey, uint32_t modulus, uint64_t p, uint64_t q);
uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod);
uint64_t modInverse(uint64_t a, uint64_t m);

XStatus initDecryption() {
	//	Do something here
	XGpio_Initialize(&dataCom, COM_DATA_DEVICE_ID);

	XGpio_SetDataDirection(&dataCom, 1, 0xFFFFFFFF);
	XGpio_SetDataDirection(&dataCom, 1, 0x0);

	return XST_SUCCESS;
}

bool decryption(rsaData *RSAData, char *code_buffer){
	// Store the encrypted data
	static uint32_t encryptedData[CODE_LENGTH];

	// State machine for decryption
	static enum rsaDecryptionState state = Decryption_waiting;
	
	// Flag to indicate if the decryption process is running
	static bool decrypting = false;
	
	// Flag to indicate if data is ready
	static bool dataReady = false;

	// Check if the data is ready
	bool dataReadyState = XGpio_DiscreteRead(&dataCom, 1) & 0x1;
	// Check for a rising edge
	// if (dataReadyState && !prevDataReadyState && dataReadyState == true) {
	if (dataReadyState && !dataReady) {
		// Set the data ready flag
		dataReady = true;
		
		// reset the code buffer -- temp
		for (int i = 0; i < CODE_LENGTH; i++) {
			code_buffer[i] = DEFAULT_CODE;
		}
	}
	
	// State machine
	switch (state) {
		// Wait for data to be ready, the set decrypting flag
		case Decryption_waiting:	
			if (dataReady) {
				state = Decryption_getData;
				decrypting = true;
			} else {
				decrypting = false;
			}
			break;

		// Get the data from the digital blocks
		case Decryption_getData:
			for (int i = 0; i < CODE_LENGTH; i++) {
				encryptedData[i] = Xil_In32(ENCRYPTED_DATA_BASE_ADDR);
				Xil_Out32(COM_DATA_BASE_ADDR + XGPIO_DATA2_OFFSET, 0x1);
				Xil_Out32(COM_DATA_BASE_ADDR + XGPIO_DATA2_OFFSET, 0x0);

				#ifdef DEBUG
					xil_printf("Encrypted data %d: %d\n\r", i, encryptedData[i]);
				#endif
			}

			state = Decryption_decrypt;
			break;

		// Decrypt the data
		case Decryption_decrypt:
			// Decrypt the data
			for (int i = 0; i < CODE_LENGTH; i++) {
				code_buffer[i] = decryptCRT(encryptedData[i], RSAData->privateKey, RSAData->modulus, RSAData->primes_p, RSAData->primes_q);

				if (code_buffer[i] > CODE_MAX_DIGIT || code_buffer[i] < CODE_MIN_DIGIT) {
					code_buffer[i] = CODE_ERROR;
				}
			}

			// Reset the buffer
			for (int i = 0; i < CODE_LENGTH; i++) {
				encryptedData[i] = 0;
			}

			#ifdef DEBUG
				xil_printf("Decrypted data: ");
				for (int i = 0; i < CODE_LENGTH; i++) {
					xil_printf("%d ", code_buffer[i] - '0');
				}
				xil_printf("\n\r\n\r");
			#endif
			
			state = Decryption_waiting;
			dataReady = false;
			break;

		default:
			state = Decryption_waiting;
			break;
	}
	return decrypting;
}


/** Decrypts the given encrypted data using the Chinese Remainder Theorem (CRT) method.
 *
 * @param encryptedData The data to be decrypted.
 * @param privateKey The private key used for decryption.
 * @param modulus The modulus used for decryption.
 * @param p The first prime number.
 * @param q The second prime number.
 * @return The decrypted data.
 */
uint64_t decryptCRT(uint64_t encryptedData, uint64_t privateKey, uint32_t modulus, uint64_t p, uint64_t q) {
	uint64_t dP = 0;
	uint64_t dQ = 0;
	uint64_t qInv = 0;

	// Calculate dP, dQ and qInv
	dP = privateKey % (p - 1);
	dQ = privateKey % (q - 1);
	qInv = modInverse(q, p);

	// Calculate m1 and m2
	uint64_t m1 = modPow(encryptedData, dP, p);
	uint64_t m2 = modPow(encryptedData, dQ, q);

	// Calculate h
	uint64_t h = (qInv * (m1 - m2)) % p;

	// Calculate m
	uint64_t m = m2 + h * q;

	return m;
}

/** Calculates the modular exponentiation of the given base, exponent and modulus.
 *
 * @param base The base value.
 * @param exp The exponent value.
 * @param mod The modulus value.
 * @return The result of the modular exponentiation.
 */
uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod) {
	uint64_t result = 1;
	base = base % mod;
	while (exp > 0) {
		if (exp % 2 == 1) {
			result = (result * base) % mod;
		}
		exp = exp >> 1;
		base = (base * base) % mod;
	}
	return result;
}

/** Calculates the modular inverse of the given values.
 *
 * @param a The first value.
 * @param m The second value.
 * @return The modular inverse of the given values.
 */
uint64_t modInverse(uint64_t a, uint64_t m) {
	a = a % m;
	for (int x = 1; x < m; x++) {
		if ((a * x) % m == 1) {
			return x;
		}
	}
	return 1;
}
