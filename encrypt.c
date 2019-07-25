#include "encrypt.h"


static void encrypt_buffer(unsigned char key[], unsigned char source[], unsigned char cipher[], unsigned char previous[]) { //simple encryption using CBC to be upgraded
	for (int i = 0; i < BLOCK_SIZE; i++) {
		cipher[i] = source[i] ^ previous[i];
		cipher[i] ^= key[i];
		previous[i] = cipher[i];
		source[i] = cipher[i];
	}
}

void encrypt(unsigned char *source, unsigned char key[], int blocks_number) {
	
	/*
	Prepend random data before encrypted data to generate different ciphers for the same plaintext every time.
	Explicit Initialization Vector is used.	This random data is subsequently used as an initialization vector for Cipher Block Chaining.
	*/
	
	unsigned char previous_block[KEY_SIZE];
	get_random_block(previous_block, KEY_SIZE);

	for (int i = 0; i < KEY_SIZE; i++) {
			source[i] = previous_block[i];
	}
	unsigned char *source_buffer;
	source_buffer = source + KEY_SIZE;

	int block_count = 0;

	printf("block count: %d, max_block_count: %d\n", block_count, blocks_number);
	
	while (block_count < blocks_number) {
		unsigned char cipher_buffer[BLOCK_SIZE];
		//for (int i = 0; i < 8; i++) {
		//	printf("first 8 chars of source before encryption: %d: %c\n", i , source_buffer[i]);
		//}

		encrypt_buffer(key, source_buffer, cipher_buffer, previous_block);
		//for (int i = 0; i < 8; i++) {
		//	printf("first 8 chars of source after encryption: %d: %c\n", i, source_buffer[i]);
		//}
		printf("block count: %d \n", block_count);
		if (block_count <= blocks_number) {
			source_buffer = source_buffer + BLOCK_SIZE;
		}
		block_count++;
	}
}

void encrypt_file(char *filepath) {
	
	FILE *sourcefile;
	errno_t source_error;
	long int filesize = get_file_size(filepath);
	
	char filename_buffer[80];
	int len = strlen(filepath);
	printf("length of filepath %d \n", len);
	strcpy_s(filename_buffer, len + 1, filepath);
			
	source_error = fopen_s(&sourcefile, filepath, "rb");
	if (source_error != 0) {
		fprintf(stderr, "Problem with opening source file");
		exit(1);
	}
	
	unsigned char *file_buffer;
	int blocks_number = filesize / BLOCK_SIZE + (filesize % BLOCK_SIZE != 0 ? 1 : 0);
	file_buffer = malloc(blocks_number*BLOCK_SIZE + KEY_SIZE);
	
	if (file_buffer != NULL) {
		fread(file_buffer + KEY_SIZE, filesize, 1, sourcefile);
		fclose(sourcefile);
		sourcefile = NULL;
		printf("blocks number %d\n", blocks_number);
		unsigned char key_buffer[KEY_SIZE];
		get_random_block(key_buffer, KEY_SIZE);

		/*adding padding to the last block if its size is less than BLOCK_SIZE*/

		if (filesize % BLOCK_SIZE != 0) {
			int padding = BLOCK_SIZE - filesize % BLOCK_SIZE;
			int i = KEY_SIZE + (blocks_number - 1) * BLOCK_SIZE + filesize % BLOCK_SIZE;
			for (; i < BLOCK_SIZE * blocks_number + KEY_SIZE; i++) {
				file_buffer[i] = padding;
			}
		}

		printf("encrypting file %s\n", filepath);
		encrypt(file_buffer, key_buffer, blocks_number);
		get_new_filename(filename_buffer, ENCRYPTED_FILE);
		save_result(filename_buffer, file_buffer, blocks_number*BLOCK_SIZE + KEY_SIZE);
			
		get_new_filename(filename_buffer, KEY_FILE);
		save_result(filename_buffer, key_buffer, KEY_SIZE);
		free(file_buffer);
	}
}