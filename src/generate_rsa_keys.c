#include "generate_rsa_keys.h"
#include "defines.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "xstatus.h"
#include "xil_printf.h"

void print64(uint64_t num) {
	xil_printf("%x",(unsigned int)((num & 0xFFFFFFFF00000000) >> 32));
	xil_printf("%x\r\n",(unsigned int)(num & 0x00000000FFFFFFFF));
}

void extendedEuclid(uint64_t a, uint64_t b, int64_t *g, int64_t *y, int64_t *x) {
	if (a == 0) {
		*g = b;
		*y = 0;
		*x = 1;
		return;
	}	

	int64_t g2, y2, x2;
	extendedEuclid(b % a, a, &g2, &y2, &x2);
	
	*g = g2;
 	*y = x2 - (b / a) * y2;
	*x = y2;
}

uint32_t modInv(uint32_t a, uint32_t m) {
	int64_t g, y, x;
	extendedEuclid(a, m, &g, &y, &x);
	return (y + m) % m;
}

bool isPrime(uint32_t num) {
	double root = sqrt(num);

	for (int i = 2; i <= root; i++) {
		if (num % i == 0) {
			return false;
		}
	}

	return true;
}

uint32_t generateRandomPrime() {
	uint32_t lower = 1000;
	uint32_t upper = UINT16_MAX;
	bool found = false;

	uint64_t random;
	while (!found) {
		random = (rand() % (upper - lower + 1)) + lower;
		found = isPrime(random);
	}
	return random;
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

void generatePrivateKey(uint32_t totient, rsaData *data) {
	data->privateKey = modInv(data->publicKey, totient);
}

void generateRSAKeys(rsaData *data, uint64_t seed) {
	// Seed the random number generator
	srand(seed);

	// Generate two random prime numbers
	bool found = false;
	uint32_t p, q;
	while (!found) {
		p = generateRandomPrime();
		q = generateRandomPrime();
		if (p != q) {
			found = true;
		}
	}
	data->primes_p = p;
	data->primes_q = q;

	#ifdef DEBUG
		xil_printf("Primes: %x, %x\n\r", p, q);
	#endif

	// Calculate the modulus
	data->modulus = p * q;
	#ifdef DEBUG
		xil_printf("Modulus: %x\n\r", data->modulus);
	#endif

	// Calculate the totient
	uint32_t totient = (p - 1) * (q - 1);
	#ifdef DEBUG
		xil_printf("Totient: %x\n\r", totient);
	#endif

	// Generate the public key
	generatePublicKey(totient, data);
	#ifdef DEBUG
		xil_printf("Public key: ");
		print64(data->publicKey);
	#endif

	// Generate the private key
	generatePrivateKey(totient, data);
	#ifdef DEBUG
		xil_printf("Private key: ");
		print64(data->privateKey);
	#endif
}
