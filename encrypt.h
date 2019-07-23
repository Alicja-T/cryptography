#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stdio.h>
#include "utils.h"

void generate_key(unsigned char key[]);

void encrypt(FILE *source, unsigned char key[], long int file_size);

void encrypt_file(char *filepath);


#endif // ENCRYPT_H

