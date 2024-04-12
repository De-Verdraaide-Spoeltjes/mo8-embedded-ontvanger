#include "rsa_decryption.h"
#include "xgpio.h"
#include "xstatus.h"
#include <stdlib.h>

XGpio dataInput;

enum rsaDecryptionState {
	Decryption_waiting,
	Decryption_getData,
	Decryption_decrypt
};


uint32_t *prime_values;

bool dataReady = false;

#define READ_DATA_DEVICE_ID          XPAR_CONNECTION_EMBEDDED_BUTTONS_DEVICE_ID


uint64_t decryptCRT(uint64_t encryptedData, uint64_t privateKey, uint32_t modulus);
uint64_t modPow(uint64_t base, uint64_t exp, uint64_t mod);
uint64_t modInverse(uint64_t a, uint64_t m);
void calc_primes();

XStatus initDecryption() {
	//	Do something here
	XGpio_Initialize(&dataInput, READ_DATA_DEVICE_ID);

	XGpio_SetDataDirection(&dataInput, 1, 0x1);

	calc_primes();

	return XST_SUCCESS;
}

// test values
// uint64_t publicKey = 7;
uint64_t privateKey = 11581303;
uint32_t modulus = 27033439;
bool decryption(rsaData *RSAData, char *code_buffer){
	// test values
	static uint32_t encryptedData[CODE_LENGTH];

	// State machine for decryption
	static enum rsaDecryptionState state = Decryption_waiting;
	// Flag to indicate if the decryption process is running
	static bool decrypting = false;

	// Check if data is ready
	static bool prevDataReadyState = false;
	bool dataReadyState = XGpio_DiscreteRead(&dataInput, 1) & 0x1;
	if (dataReadyState && !prevDataReadyState && dataReadyState == true) {
		dataReady = true;
		// reset the code buffer -- temp
		for (int i = 0; i < CODE_LENGTH; i++) {
			code_buffer[i] = '0';
		}
	}
	prevDataReadyState = dataReadyState;
	
	// State machine
	switch (state) {
		case Decryption_waiting:
			if (dataReady) {
				state = Decryption_getData;
				decrypting = true;
				dataReady = false;
			} else {
				decrypting = false;
			}
			break;

		case Decryption_getData:
			encryptedData[0] = 81531;
			encryptedData[1] = 13043192;
			encryptedData[2] = 20083651;
			encryptedData[3] = 9532143;

			state = Decryption_decrypt;
			break;

		case Decryption_decrypt:
			for (int i = 0; i < CODE_LENGTH; i++) {
				code_buffer[i] = decryptCRT(encryptedData[i], privateKey, modulus);
			}

			// Reset the buffer
			for (int i = 0; i < CODE_LENGTH; i++) {
				encryptedData[i] = 0;
			}
			
			state = Decryption_waiting;
			break;

		default:
			state = Decryption_waiting;
			break;
	}
	return decrypting;
}


uint64_t decryptCRT(uint64_t encryptedData, uint64_t privateKey, uint32_t modulus) {
	uint64_t p = 0;
	uint64_t q = 0;
	uint64_t dP = 0;
	uint64_t dQ = 0;
	uint64_t qInv = 0;

	// Calculate p and q
	for (int i = 0; i < RSA_PRIMES_MAXIMUM; i++) {
		if (modulus % prime_values[i] == 0) {
			p = prime_values[i];
			q = modulus / prime_values[i];
			break;
		}
	}

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

uint64_t modInverse(uint64_t a, uint64_t m) {
	a = a % m;
	for (int x = 1; x < m; x++) {
		if ((a * x) % m == 1) {
			return x;
		}
	}
	return 1;
}


void calc_primes() {
	bool isPrime[RSA_PRIMES_MAXIMUM + 1];
	uint32_t primeCount;


	for (int i = 1; i <= RSA_PRIMES_MAXIMUM; i++) {
		isPrime[i] = true;
	}

	isPrime[0] = false;
	isPrime[1] = false;
	isPrime[2] = false;

	for (int i = 2; i <= RSA_PRIMES_MAXIMUM; i++) {
		int j = 2 * (i);
		while (j <= RSA_PRIMES_MAXIMUM) {
			isPrime[j] = false;
			j += i;
		}
	}

	primeCount = 0;
	for (int i = 0; i <= RSA_PRIMES_MAXIMUM; i++) {
		if (isPrime[i]) {
			primeCount++;
		}
	}

	prime_values = (uint32_t *)malloc(primeCount * sizeof(uint32_t));

	primeCount = 0;
	for (int i = 0; i <= RSA_PRIMES_MAXIMUM; i++) {
		if (isPrime[i]) {
			prime_values[primeCount] = i;
			primeCount++;
		}
	}
}
