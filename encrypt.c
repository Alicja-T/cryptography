#include "encrypt.h"


/*
	Generates a random key of KEY_SIZE and saves it to a file.
*/

static void generate_key(unsigned char* key_buffer) {
	get_random_block(key_buffer, KEY_SIZE);
	char filename_buffer[80];
	get_new_filename(filename_buffer, KEY_FILE);
	save_result(filename_buffer, key_buffer, KEY_SIZE);
}

/*
	Adds padding to the last block to keep all blocks the same size.
*/

static void add_padding(unsigned char *buffer, long int filesize) {
	
	int padding = BLOCK_SIZE - filesize % BLOCK_SIZE;
	int i = filesize % BLOCK_SIZE;
	for (int j = i; j < i + padding; j++) {
		buffer[j] = padding;
	}
}


/*
	Encrypts KEY_SIZE block using Cipher Block Chain 
	(next block is encrypted using previous block).
*/

static void encrypt_key_size_block(unsigned char key[], unsigned char source[], unsigned char previous[]) { 
	unsigned char result_buffer[KEY_SIZE];
	for (int i = 0; i < KEY_SIZE; i++) {
		result_buffer[i] = source[i] ^ previous[i];
		result_buffer[i] ^= key[i];
		previous[i] = result_buffer[i];
		source[i] = result_buffer[i];
	}
}

/*
	Encrypts BLOCK_SIZE block by encrypting KEY_SIZE blocks 
	and transposing matrices corresponding to a key to increase noise. 
	BLOCK_SIZE is assumed to be multiple of KEY_SIZE
*/

static void encrypt_block(unsigned char key[], unsigned char source_block[], unsigned char previous[]) {
	int iter_number = BLOCK_SIZE / KEY_SIZE;
	for (int i = 0; i < iter_number; i++) {
		encrypt_key_size_block(key, source_block + i * KEY_SIZE, previous);
		unsigned char key_bits = key[i];
			for (int j = 0; j < 8; j++) {
				if ((key_bits & 1) == 1) {
					unsigned char key_matrix[8];
					int offset = i * KEY_SIZE + j * 8;
					for (int k = 0; k < 8; k++) {
						key_matrix[k] = source_block[offset + k];
					}
						transpose8(key_matrix);
					
					for (int k = 0; k < 8; k++) {
						source_block[offset + k] = key_matrix[k];
					}
				}
				key_bits >>= 1;
			}
	}
}

/*
	Operates on a buffer that is presumed to be multiple of BLOCK_SIZE.
	Prepends random data before encrypted data to generate different encrypted file for the same plaintext every time.
	Explicit Initialization Vector is used.	This random data is subsequently used as
	an Initialization Vector for Cipher Block Chaining.
*/

void encrypt_buffer(unsigned char *source, unsigned char key[], int blocks_number) {
	
	unsigned char previous_block[KEY_SIZE];
	get_random_block(previous_block, KEY_SIZE);

	for (int i = 0; i < KEY_SIZE; i++) {
			source[i] = previous_block[i];
	}
	unsigned char *source_buffer;
	source_buffer = source + KEY_SIZE;

	int block_count = 0;
		
	while (block_count < blocks_number) {
			
		encrypt_block(key, source_buffer, previous_block);
		if (block_count <= blocks_number) {
			source_buffer = source_buffer + BLOCK_SIZE;
		}
		block_count++;
	}
}

/*
	This function is called only if malloc in encrypt_file function fails. 
	It uses small buffer to encrypt file sequentially.
*/

void encrypt_file_with_buffering(char* filepath, long int filesize) {
	
	int blocks_number = filesize / BLOCK_SIZE + 1;

	unsigned char key[KEY_SIZE];
	generate_key(key);

	FILE *source_file;
	FILE *result_file;

	source_file = fopen(filepath, "rb");
	if (source_file == NULL) {
		fprintf(stderr, "Problem with opening source file: %s \n", strerror(errno));
		exit(1);
	}

	char filename_buffer[260];
	strcpy(filename_buffer, filepath);
	get_new_filename(filename_buffer, ENCRYPTED_FILE);

	result_file = fopen(filename_buffer, "wb");
	if (result_file == NULL) {
		fprintf(stderr, "write error: %s \n", strerror(errno));
		exit(1);
	}

	unsigned char previous_block[KEY_SIZE];
	get_random_block(previous_block, KEY_SIZE);
	int write_result = fwrite(previous_block, 1, KEY_SIZE, result_file);
	if (write_result != KEY_SIZE) {
		fprintf(stderr, "write error: %s \n", strerror(errno));
		exit(1);
	}
	int i = 0;

	while ( i < blocks_number - 1){
		unsigned char buffer[BLOCK_SIZE];
		int read_result = fread(buffer, 1, BLOCK_SIZE, source_file);
		if (read_result == BLOCK_SIZE) {
			encrypt_block(key, buffer, previous_block);
			write_result = fwrite(buffer, 1, BLOCK_SIZE, result_file);
			if (write_result != BLOCK_SIZE) {
				fprintf(stderr, "write error: %s \n", strerror(errno) );
				exit(1);
			}
		}
		else {
			fclose(source_file);
			fclose(result_file);
			fprintf(stderr, "Error reading file: %s \n", strerror(errno) );
			exit(1);
		}
		i++;
	}
	
	//add padding to the last block
	if (i == blocks_number - 1) {
		int remainder = filesize % BLOCK_SIZE;
		unsigned char buffer[BLOCK_SIZE];
		int read_result = fread(buffer, 1, remainder, source_file);
		if (read_result == remainder) {
			add_padding(buffer, filesize);
			encrypt_block(key, buffer, previous_block);
			write_result = fwrite(buffer, 1, BLOCK_SIZE, result_file);
		}
	}
	fclose(source_file);
	fclose(result_file);
	printf("Encryption successful. Total blocks number: %d \n", blocks_number);
}

/*
	Encrypts file allocating dynamic memory buffer.
	If malloc fails to allocate memory for entire file, encrypt_file_with_buffering is called.
*/


void encrypt_file(char *filepath) {
	
	FILE *source_file;
	long int filesize = get_file_size(filepath);
			
	unsigned char *file_buffer;
	int blocks_number = filesize / BLOCK_SIZE + 1;
	file_buffer = malloc(blocks_number*BLOCK_SIZE + KEY_SIZE);

	printf("filesize: %ld blocks number %d\n", filesize, blocks_number);
	
	if (file_buffer != NULL) {
		source_file = fopen(filepath, "rb");
		if (source_file == NULL) {
			fprintf(stderr, "Problem with opening source file: %s \n", strerror(errno));
			exit(1);
		}
		long int read_result = fread(file_buffer + KEY_SIZE, 1, filesize, source_file);
		if (read_result == filesize) {
			
			unsigned char key_buffer[KEY_SIZE];
			generate_key(key_buffer);

			/*adding padding to the last block if its size is less than BLOCK_SIZE*/
			int offset = (blocks_number - 1) * BLOCK_SIZE + KEY_SIZE;
			add_padding(file_buffer + offset, filesize);

			encrypt_buffer(file_buffer, key_buffer, blocks_number);
			
			char filename_buffer[80];
			strcpy(filename_buffer, filepath);
			get_new_filename(filename_buffer, ENCRYPTED_FILE);
			save_result(filename_buffer, file_buffer, blocks_number*BLOCK_SIZE + KEY_SIZE);
		}
		else {
			fprintf(stderr, "Error reading file: %s \n", strerror(errno));
		}
		fclose(source_file);
		source_file = NULL;
		free(file_buffer);
	}
	else {
		encrypt_file_with_buffering(filepath, filesize);
	}
}