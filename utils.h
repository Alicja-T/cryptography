#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*
utils.h contains functions that are used by both encryption and decryption functions.
*/

enum file_type { KEY_FILE, ENCRYPTED_FILE, DECRYPTED_FILE };

enum buffer_size {KEY_SIZE = 64, BLOCK_SIZE = 256};

void get_random_block(unsigned char buffer[], int size);

long int get_file_size(char* filepath);

void printBits(size_t const size, void const * const ptr);

void transpose8(unsigned char input[8]);

void save_result(char* file_path, unsigned char *buffer, long int buffer_size);

void get_new_filename(char* file_name, int mode);


#endif // !UTILS_H
