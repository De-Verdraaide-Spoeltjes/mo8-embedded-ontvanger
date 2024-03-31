#ifndef SRC_DEFINES_H
#define SRC_DEFINES_H

#include <stdint.h>

typedef struct RSAData {
	uint64_t publicKey;
	uint64_t privateKey;
	uint32_t modulus;
} rsaData;

// The range for the random prime number generator - bigger primes means bigger RSA keys
#define RSA_PRIMES_MINIMUM 200
#define RSA_PRIMES_MAXIMUM 10000

#endif
