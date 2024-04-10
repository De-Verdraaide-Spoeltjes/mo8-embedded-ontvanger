#ifndef SRC_DEFINES_H
#define SRC_DEFINES_H

#include <stdint.h>

#define DEBUG

typedef struct RSAData {
	uint64_t publicKey;
	uint64_t privateKey;
	uint32_t modulus;
} rsaData;

// The range for the random prime number generator - bigger primes means bigger RSA keys
#define RSA_PRIMES_MINIMUM 200
#define RSA_PRIMES_MAXIMUM 10000


#define TIME_TO_NS_DIVIDER 325 //XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2000000
#define TIME_TO_NS(i) (i / TIME_TO_NS_DIVIDER)
#define NS_TO_TIME(i) (i * TIME_TO_NS_DIVIDER)

#endif
