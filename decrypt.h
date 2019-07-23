#ifndef DECRYPT_H
#define DECRYPT_H

#include <stdio.h>
#include "utils.h"

void decrypt(FILE *encrypted, unsigned char keybuffer[], long int file_size);
void decrypt_file(char* cipherpath, char* keypath);

#endif //DECRYPT_H
