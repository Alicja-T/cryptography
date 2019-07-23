#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/stat.h>

#define BUFFERSIZE 64
#define KEYSIZE 64

void get_random_buffer(unsigned char buffer[], int size);

long int get_file_size(char* filepath);

#endif // !UTILS_H
