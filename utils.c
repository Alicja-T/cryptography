
#include <time.h>
#include <errno.h>
#include "utils.h"


void get_random_block(unsigned char buffer[], int size) {
	time_t now = time(NULL);
	int seed = rand();
	srand(seed);
	printf("seed returned %ld\n", seed);
	for (int i = 0; i < size; i++) {
		seed = rand();
		srand(seed);
		buffer[i] = rand() % 256;
	}
}

long int get_file_size(char* filepath) {
	struct _stat buffer;
	long int size = -1;
	int result = _stat(filepath, &buffer);
	
	if (result != 0) {
		if (errno == ENOENT) {
			printf(stderr, "File %s not found\n", filepath);
		}
	}
	else {
		size = buffer.st_size;
	}
	return size;
}

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
	}
	puts("");
}

void transpose8(unsigned char * source, unsigned char * result) {
	unsigned long long x = 0;
	int i;
	for (i = 0; i <= 7; i++) {
		x = x << 8 | source[i];
	}

	x = x & 0xAA55AA55AA55AA55LL |
		(x & 0x00AA00AA00AA00AALL) << 7 |
		(x >> 7) & 0x00AA00AA00AA00AALL;
	x = x & 0xCCCC3333CCCC3333LL |
		(x & 0x0000CCCC0000CCCCLL) << 14 |
		(x >> 14) & 0x0000CCCC0000CCCCLL;
	x = x & 0xF0F0F0F0F0F0F0F0LL |
		(x & 0x00000000F0F0F0F0LL) << 28 |
		(x >> 28) & 0x00000000F0F0F0F0LL;

	for (i = 7; i >= 0; i--) {
		result[i] = x;
		x = x >> 8;
	}

}

void save_result(unsigned char* file_path, unsigned char *buffer, long int buffer_size) {
	FILE *result_file;
	errno_t write_err;
	
	write_err = fopen_s(&result_file, file_path, "wb");
	if (write_err != 0) {
		fprintf(stderr, "write error");
		exit(1);
	}
	else {
		write_err = fwrite(buffer, 1, buffer_size, result_file);
		printf("write result %d\n", write_err);
		fclose(result_file);
		result_file = NULL;
	}
}

void get_new_filename(unsigned char* file_name, int mode) {
	
	switch (mode) {

		// adding key file extension
		case KEY_FILE: {
			time_t now = time(0);
			sprintf_s(file_name, 50, "%d.pkey", now);
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