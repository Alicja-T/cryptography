#ifndef DECRYPT_H
#define DECRYPT_H

#include <stdio.h>
#include "utils.h"

void decrypt(unsigned char *encrypted, unsigned char keybuffer[], int blocks_number);
void decrypt_file(char* cipherpath, char* keypath);

#endif //DECRYPT_H
