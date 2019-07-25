#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/stat.h>

#define BUFFERSIZE 64
#define KEYSIZE 64

void get_random_block(unsigned char buffer[], int size);

long int get_file_size(char* filepath);

void printBits(size_t const size, void const * const ptr);

void transpose8(unsigned char *, unsigned char *);


#endif // !UTILS_H
