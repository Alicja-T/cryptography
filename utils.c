
#include <sys/timeb.h>
#include <time.h>
#include <errno.h>
#include "utils.h"


/*
	Function get_random_block puts the bytes in the buffer using random numbers generator.
	Random numbers generator is seeded with milliseconds to increase randomness. 
*/

void get_random_block(unsigned char buffer[], int size) {
	
	struct _timeb tstruct;
	_ftime_s(&tstruct);
	unsigned short millisec = tstruct.millitm;
	srand( (int) millisec );
	for (int i = 0; i < size; i++) {
		buffer[i] = rand() % 256;
	}
	int blocks_number = size / 8;
	
	// Next block transposes 8x8 bit matrices corresponding to 1s in millisec number
	// to increase randomness of the block

	for (int i = 0; i < blocks_number; i++) {
		if ( (millisec & 1) == 1) {
			unsigned char key_matrix[8];
			
			for (int j = 0; j < 8; j++) {
				key_matrix[j] = buffer[i * 8 + j];
			}
		
			transpose8(key_matrix);
			
			for (int j = 0; j < 8; j++) {
				buffer[i * 8 + j] = key_matrix[j];
			}
		}
		millisec >>= 1;
	}
}

/*
Uses system call to get file size before opening. 
*/

long int get_file_size(char* filepath) {
	struct _stat buffer;
	long int size = -1;
	int result = _stat(filepath, &buffer);
	if (result != 0) {
		if (errno == ENOENT) {
			fprintf(stderr, strerror(errno));
		}
	}
	else {
		size = buffer.st_size;
	}
	return size;
}

/*
helper function useful for debugging
*/

void printBits(size_t const size, void const * const ptr){
	unsigned char *b = (unsigned char*)ptr;
	unsigned char byte;
	int i, j;

	for (i = size - 1; i >= 0; i--)
	{
		for (j = 7; j >= 0; j--)
		{
			byte = (b[i] >> j) & 1;
			printf("%u", byte);
		}
		printf(" ");
	}
	puts("");
}

/*
Function transpose8 takes input of 8 bytes source matrix, 
transposes it as a 8x8 bit array and stores the result in input array.
Source: "Hacker's Delight" Second Edition p.145
*/


void transpose8(unsigned char input[8]) {
	unsigned long long x = 0;
	int i;
	for (i = 0; i <= 7; i++) {
		x = x << 8 | input[i];
	}

	x = x & 0xAA55AA55AA55AA55LL |
		(x & 0x00AA00AA00AA00AALL) << 7 |
		(x >> 7) & 0x00AA00AA00AA00AALL;
	x = x & 0xCCCC3333CCCC3333LL |
		(x & 0x0000CCCC0000CCCCLL) << 14 |
		(x >> 14) & 0x0000CCCC0000CCCCLL;
	x = x & 0xF0F0F0F00F0F0F0FLL |
		(x & 0x00000000F0F0F0F0LL) << 28 |
		(x >> 28) & 0x00000000F0F0F0F0LL;

	for (i = 7; i >= 0; i--) {
		input[i] = x;
		x = x >> 8;
	}
}

/*
	Saves buffer of buffer_size to a file.
*/

void save_result(char* file_path, unsigned char *buffer, long int buffer_size) {
	
	FILE *result_file;
	long int write_result;
	
	result_file = fopen(file_path, "wb");
	if (result_file == NULL) {
		fprintf(stderr, "write error: %s \n", strerror(errno));
		exit(1);
	}
	else {
		write_result = fwrite(buffer, 1, buffer_size, result_file);
		printf("write result %ld\n", write_result);
		fclose(result_file);
		result_file = NULL;
	}
}

/*
 Helper function to create/reconstruct unique file name.
*/

void get_new_filename(char* file_name, int mode) {
	
	switch (mode) {

		// adding key file extension
		case KEY_FILE: {
			time_t now = time(0);
			sprintf_s(file_name, 50, "%d.pkey", (int)now );
			break;
		}
		// addint new extension
		case ENCRYPTED_FILE: 
			sprintf_s(file_name, 50, "%s.enc", file_name);
			break;
		
		// stripping off extension
		case DECRYPTED_FILE: {
			size_t len = strlen(file_name);
			for (size_t i = len - 1; i > len - 4; i--) {
				file_name[i] = '\0';
			}
			break;
		}
		default: ;
	}
	
}