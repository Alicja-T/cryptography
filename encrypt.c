#include "encrypt.h"

static void generate_key(unsigned char key[]) {
	FILE *newkeyfile;
	errno_t err;
	time_t now = time(0);
	char new_file_name[80];
	sprintf_s(new_file_name, 80, "key%d.pkey\0", (int)now);
	printf("new file name: %s\n", new_file_name);

	err = fopen_s(&newkeyfile, new_file_name, "wb");
	if (err != 0) {
		fprintf(stderr, "problem with generating key file");
	}
	else {
		get_random_block(key, BUFFERSIZE);
		fwrite(key, sizeof(char), BUFFERSIZE, newkeyfile);
		fclose(newkeyfile);
		printf("generated new key\n");
	}
}



static void encrypt_buffer(unsigned char key[], unsigned char source[], unsigned char cipher[], unsigned char previous[]) { //simple encryption using CBC to be upgraded
	for (int i = 0; i < BUFFERSIZE; i++) {
		cipher[i] = source[i] ^ previous[i];
		cipher[i] ^= key[i];
		previous[i] = cipher[i];
		source[i] = cipher[i];
	}
}

static void save_encrypted(unsigned char *file_path, unsigned char* encrypted_buffer, long int file_size) {
	FILE *encrypted_file;
	errno_t write_err;
	char new_file_name[80];
	sprintf_s(new_file_name, 80, "%s.enc\0", file_path);
	printf("new file name: %s\n", new_file_name);
	write_err = fopen_s(&encrypted_file, new_file_name, "wb");
	if (write_err != 0) {
		fprintf(stderr, "write error");
		exit(1);
	}
	write_err = fwrite(encrypted_buffer, 1, file_size, encrypted_file);
	printf("write result %d\n", write_err);
}

void encrypt(unsigned char *source, unsigned char key[], int blocks_number) {
	
	/*
	Prepend random data before encrypted data to generate different ciphers for the same plaintext every time.
	Explicit Initialization Vector is used.	This random data is subsequently used as an initialization vector for Cipher Block Chaining.
	*/
	
	unsigned char previous_block[KEYSIZE];
	get_random_block(previous_block, KEYSIZE);

	for (int i = 0; i < KEYSIZE; i++) {
			source[i] = previous_block[i];
	}
	unsigned char *source_buffer;
	source_buffer = source + KEYSIZE;

	int block_count = 0;

	printf("block count: %d, max_block_count: %d\n", block_count, blocks_number);
	
	while (block_count < blocks_number) {
		unsigned char cipher_buffer[BUFFERSIZE];
		for (int i = 0; i < 8; i++) {
			printf("first 8 chars of source before encryption: %d: %c\n", i , source_buffer[i]);
		}

		encrypt_buffer(key, source_buffer, cipher_buffer, previous_block);
		for (int i = 0; i < 8; i++) {
			printf("first 8 chars of source after encryption: %d: %c\n", i, source_buffer[i]);
		}
		printf("block count: %d \n", block_count);
		if (block_count <= blocks_number) {
			source_buffer = source_buffer + BUFFERSIZE;
		}
		block_count++;
	}
}

void encrypt_file(char *filepath) {
	
	FILE *sourcefile;
	errno_t source_error;
	long int filesize = get_file_size(filepath);

	printf("File %s size: %d\n", filepath, filesize);
		
	source_error = fopen_s(&sourcefile, filepath, "rb");
	if (source_error != 0) {
		fprintf(stderr, "Problem with opening source file");
		exit(1);
	}
	
	unsigned char *file_buffer;
	int blocks_number = filesize / BUFFERSIZE + (filesize % BUFFERSIZE != 0 ? 1 : 0);
	file_buffer = malloc(blocks_number*BUFFERSIZE + KEYSIZE);
	if (file_buffer != NULL) {
		fread(file_buffer + KEYSIZE, filesize, 1, sourcefile);
		fclose(sourcefile);
		sourcefile = NULL;
		printf("blocks number %d\n", blocks_number);
		unsigned char key_buffer[BUFFERSIZE];
		generate_key(key_buffer);
		
		if (filesize % BUFFERSIZE != 0) {
			int padding = BUFFERSIZE - filesize % BUFFERSIZE;
			printf("padding number %d\n", padding);
			int i = KEYSIZE + (blocks_number - 1) * BUFFERSIZE + filesize % BUFFERSIZE;
			for (; i < BUFFERSIZE * blocks_number + KEYSIZE; i++) {
				file_buffer[i] = padding;
				printf("index number %d\n", i);
			}
		}

		printf("encrypting file %s\n", filepath);
		encrypt(file_buffer, key_buffer, blocks_number);
		save_encrypted(filepath, file_buffer, blocks_number*BUFFERSIZE + KEYSIZE);
		free(file_buffer);
	}
}