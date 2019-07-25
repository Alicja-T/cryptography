#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

/*
utils.h contains functions that are used by both encryption and decryption functions.
*/


#define BLOCK_SIZE 64
#define KEY_SIZE 64
#define KEY_FILE 1
#define ENCRYPTED_FILE 2
#define DECRYPTED_FILE 3

void get_random_block(unsigned char buffer[], int size);

long int get_file_size(char* filepath);

void printBits(size_t const size, void const * const ptr);

void transpose8(unsigned char *, unsigned char *);

void save_result(unsigned char* file_path, unsigned char *buffer, long int buffer_size);

void get_new_filename(unsigned char* file_name, int mode);


#endif // !UTILS_H
