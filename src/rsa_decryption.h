#ifndef SRC_RSA_DECRYPTION_H
#define SRC_RSA_DECRYPTION_H

#include "xstatus.h"
#include <stdbool.h>
#include "defines.h"

XStatus initDecryption();
bool decryption(rsaData *RSAData, char *code_buffer);

#endif
