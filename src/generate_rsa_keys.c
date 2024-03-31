#include "generate_rsa_keys.h"
#include "defines.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "xstatus.h"
#include "xil_printf.h"

uint32_t *primes;
uint32_t primeCount;

void print64(uint64_t num) {
	xil_printf("%x",(unsigned int)((num & 0xFFFFFFFF00000000) >> 32));
	xil_printf("%x\r\n",(unsigned int)(num & 0x00000000FFFFFFFF));
}

void seedPrimes() {
	bool isPrime[RSA_PRIMES_MAXIMUM + 1];

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

	primes = (uint32_t *)malloc(primeCount * sizeof(uint32_t));

	primeCount = 0;
	for (int i = 0; i <= RSA_PRIMES_MAXIMUM; i++) {
		if (isPrime[i]) {
			primes[primeCount] = i;
			primeCount++;
		}
	}
}

uint32_t generateRandomPrime() {
	bool found = false;
	uint32_t randomPrime = 0;
	while (!found) {
		randomPrime = primes[rand() % primeCount];
		found = randomPrime > RSA_PRIMES_MINIMUM;
	}

	return randomPrime;
}

uint64_t gcd(uint64_t a, uint64_t b) {
	uint64_t temp;
	while (1) {
		temp = a % b;
		if (temp == 0) {
			return b;
		}
		a = b;
		b = temp;
	}
}

void generatePublicKey(uint64_t totient, rsaData *data) {
	bool found = false;
	uint64_t key = 2;
	while (!found) {
		if (gcd(key, totient) == 1) {
			found = true;
			data->publicKey = key;
		}
		key++;
	}
}

void generatePrivateKey(int totient, rsaData *data) {
	bool found = false;
	uint64_t key = 2;
	while (!found) {
		if ((key * data->publicKey) % totient == 1) {
			found = true;
			data->privateKey = key;
		}
		key++;
	}
}

rsaData* generateRSAKeys(uint64_t seed) {
	rsaData data;

	// Generate a list of prime numbers
	seedPrimes();

	// Seed the random number generator
	srand(seed);

	// Generate two random prime numbers
	bool found = false;
	uint32_t p, q;
	while (!found) {
		p = generateRandomPrime();
		q = generateRandomPrime();
		if (p != q) {
			if (p * q < 65536) {
					found = true;
				}
		}
	}
	xil_printf("Primes: %x, %x\n\r", p, q);

	// Free the primes list from memory
	free(primes);

	// Calculate the modulus
	data.modulus = p * q;
	xil_printf("Modulus: %x\n\r", data.modulus);

	// Calculate the totient
	uint32_t totient = (p - 1) * (q - 1);
	xil_printf("Totient: %x\n\r", totient);

	// Generate the public key
	generatePublicKey(totient, &data);
	xil_printf("Public key: ");
	print64(data.publicKey);

	// Generate the private key
	generatePrivateKey(totient, &data);
	xil_printf("Private key: ");
	print64(data.privateKey);

	return &data;
}
