#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "utils.h"

/*
Function encrypt encrypts a buffer "source" with a provided key. Buffer source is
assumed to consist of number of blocks size BLOCK_SIZE defined in utils.h.
*/


void encrypt_buffer(unsigned char *source, unsigned char key[], int blocks_number);

/*
Function encrypt_file opens file and loads it into a buffer using malloc. 
*/

void encrypt_file(char *filepath);


#endif // ENCRYPT_H

