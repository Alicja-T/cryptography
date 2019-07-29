#include "decrypt.h"


/*
Decrypts KEY_SIZE block using previous block.
*/


static void decrypt_key_size_block(unsigned char key[], unsigned char encrypted[], unsigned char previous[]) { 
	unsigned char result_buffer[KEY_SIZE];
	
	for (int i = 0; i < KEY_SIZE; i++) {
		result_buffer[i] = key[i] ^ encrypted[i];
		result_buffer[i] ^= previous[i];
		previous[i] = encrypted[i];
		encrypted[i] = result_buffer[i];
	}
}

/*
Decrypts BLOCK_SIZE block by first transposing matrices corresponding to a key,
and then decrypting KEY_SIZE blocks. BLOCK_SIZE is assumed to be multiple of KEY_SIZE
*/

static void decrypt_block(unsigned char key[], unsigned char result_block[], unsigned char previous[]) {
	int iter_number = BLOCK_SIZE / KEY_SIZE;
	for (int i = 0; i < iter_number; i++) {
		unsigned char key_bits = key[i];
		for (int j = 0; j < 8; j++) {
			if ((key_bits & 1) == 1) {
				unsigned char key_matrix[8];
				int offset = i * KEY_SIZE + j * 8;
				for (int k = 0; k < 8; k++) {
					key_matrix[k] = result_block[offset + k];
				}
				
				transpose8(key_matrix);

				for (int k = 0; k < 8; k++) {
					result_block[offset + k] = key_matrix[k];
				}
			}
			key_bits >>= 1;
		}
		decrypt_key_size_block(key, result_block + i * KEY_SIZE, previous);
	}
}

/*
Removes padding added in encryption process.
*/

int remove_padding(unsigned char source[]) {
	int padding = source[BLOCK_SIZE - 1];
	for (int i = BLOCK_SIZE - 2; i > BLOCK_SIZE - padding - 1; i--) {
		if (source[i] != padding) {
			return 0;
		}
	}
	return padding;
}

/*
	Operates on a buffer that is presumed to be multiple of BLOCK_SIZE + KEY_SIZE. 
	First random block contains initialization vector.

*/

void decrypt(unsigned char *encrypted, unsigned char key[], int blocks_number) {
	
	int block_count = 0;
	
	unsigned char previous_block[KEY_SIZE];
	for (int i = 0; i < KEY_SIZE; i++) {
		previous_block[i] = encrypted[i];
	}

	unsigned char *encrypted_buffer;
	encrypted_buffer = encrypted + KEY_SIZE;
	
	while (block_count < blocks_number) {
		decrypt_block(key, encrypted_buffer, previous_block);
		
		if (block_count <= blocks_number) {
			encrypted_buffer += BLOCK_SIZE;
		}
		block_count++;
	}
	printf("total block count: %d\n", block_count);
}

/*
	This function is called only if malloc in decrypt_file fails.
	It uses small buffer to decrypt file.
*/
void decrypt_file_with_buffering(char* filepath, unsigned char key[], long int filesize) {
	int blocks_number = filesize / BLOCK_SIZE;
	FILE *source_file;
	FILE *result_file;
	source_file = fopen( filepath, "rb");
	if (source_file == NULL) {
		fprintf(stderr, "Problem with opening source file: %s \n", strerror(errno));
		exit(1);
	}
	char filename_buffer[80];
	int len = strlen(filepath);
	strcpy_s(filename_buffer, len + 1, filepath);
	get_new_filename(filename_buffer, DECRYPTED_FILE);
	
	result_file = fopen(filename_buffer, "wb");
	if (result_file == NULL) {
		fprintf(stderr, "Write error: %s \n", strerror(errno));
		exit(1);
	}

	unsigned char previous_block[KEY_SIZE];
	get_random_block(previous_block, KEY_SIZE);
	int read_result = fread(previous_block, 1, KEY_SIZE, source_file);
	if (read_result != KEY_SIZE) {
		fprintf(stderr, "Problem with decrypting file.");
		exit(1);
	}
	int i = 0;
	int write_result;
	
	while (i < blocks_number - 1) {
		unsigned char buffer[BLOCK_SIZE];
		int read_result = fread(buffer, 1, BLOCK_SIZE, source_file);
		if (read_result == BLOCK_SIZE) {
			decrypt_block(key, buffer, previous_block);
			write_result = fwrite(buffer, 1, BLOCK_SIZE, result_file);
			if (write_result != BLOCK_SIZE) {
				fprintf(stderr, "write error: %s \n", strerror(errno));
				exit(1);
			}
		}
		else {
			fclose(source_file);
			fclose(result_file);
			printf("blocks_number on error: %d \n", i);
			printf("Error reading source file.\n");
			exit(1);
		}
		i++;
	}
	// remove padding from the last block
	if (i == blocks_number - 1) {
		unsigned char buffer[BLOCK_SIZE];
		int read_result = fread(buffer, 1, BLOCK_SIZE, source_file);
		if (read_result == BLOCK_SIZE) {
			decrypt_block(key, buffer, previous_block);
			int padding = remove_padding(buffer);
			write_result = fwrite(buffer, 1, BLOCK_SIZE - padding, result_file);
		}
	}
	fclose(source_file);
	fclose(result_file);
	printf("Decryption successful \n");
}

/*
	Decrypts file using dynamic memory buffer.
	If malloc fails to allocate buffer for the file size,
	decrypt_file_with_buffering is called.
*/

void decrypt_file(char* encrypted_name, char* keypath) {

	FILE *encrypted_file;
	FILE *key_file;
	
	long int filesize = get_file_size(encrypted_name);

	if (filesize == -1) {
		exit(1);
	}
		
	key_file = fopen(keypath, "rb");
	if (key_file == NULL) {
		fprintf(stderr, "Problem with opening key file: %s", strerror(errno));
		exit(1);
	}

	unsigned char key_buffer[KEY_SIZE];
	size_t read_result = fread(key_buffer, 1, KEY_SIZE, key_file);
	if (read_result != KEY_SIZE) {
		printf("Incomplete key\n");
		fclose(key_file);
		exit(1);
	}
	else {
		printf("key read successfully\n");
		fclose(key_file);
		key_file = NULL;
	}
	
	if (filesize != -1) {
		unsigned char *encrypted_buffer;
		int blocks_number = (filesize - KEY_SIZE) / BLOCK_SIZE;
		encrypted_buffer = malloc(filesize);
		if (encrypted_buffer != NULL) {
			encrypted_file = fopen(encrypted_name, "rb");
			if (encrypted_file == NULL) {
				fprintf(stderr, "Problem with opening enrypted file: %s \n", strerror(errno));
				exit(1);
			}
			fread(encrypted_buffer, filesize, 1, encrypted_file);
			fclose(encrypted_file);
			encrypted_file = NULL;

			printf("decrypting file %s\n", encrypted_name);
			decrypt(encrypted_buffer, key_buffer, blocks_number);
			
			int padding = remove_padding(encrypted_buffer + KEY_SIZE + (blocks_number - 1) * BLOCK_SIZE);
			int final_size = blocks_number * BLOCK_SIZE - padding;

			char filename_buffer[260];
			strcpy(filename_buffer, encrypted_name);

			get_new_filename(filename_buffer, DECRYPTED_FILE);
			save_result(filename_buffer, encrypted_buffer + KEY_SIZE, final_size);
			free(encrypted_buffer);
		}
		else {
			decrypt_file_with_buffering(encrypted_name, key_buffer, filesize);
			printf("Successfully decrypted %ld bytes. \n", filesize);
		}
	}
	else {
		exit(1);
	}
}
