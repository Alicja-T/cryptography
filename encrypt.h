#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stdio.h>
#include "utils.h"


void encrypt(unsigned char *source, unsigned char key[], int blocks_number);

void encrypt_file(char *filepath);


#endif // ENCRYPT_H

