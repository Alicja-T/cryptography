
#include "decrypt.h"

static void decrypt_buffer(unsigned char key[], unsigned char result[], unsigned char cipher[], unsigned char previous[]) { //simple decryption using CBC to be upgraded
	for (int i = 0; i < BLOCK_SIZE; i++) {
		result[i] = key[i] ^ cipher[i];
		result[i] ^= previous[i];
		previous[i] = cipher[i];
		cipher[i] = result[i];
	}
}

int remove_padding(unsigned char source[]) {
	int padding = source[BLOCK_SIZE - 1];
	for (int i = BLOCK_SIZE - 2; i > BLOCK_SIZE - padding - 1; i--) {
		if (source[i] != padding) {
			return 0;
		}
	}
	return padding;
}

void decrypt(unsigned char *encrypted, unsigned char key[], int blocks_number) {
	
	printf("decrypting file... \n");
		
	int block_count = 0;
	
	unsigned char previous_block[KEY_SIZE];
	for (int i = 0; i < KEY_SIZE; i++) {
		previous_block[i] = encrypted[i];
	}

	unsigned char *encrypted_buffer;
	encrypted_buffer = encrypted + KEY_SIZE;
	
	while (block_count < blocks_number) {
				
		unsigned char result_buffer[BLOCK_SIZE];
		decrypt_buffer(key, result_buffer, encrypted_buffer, previous_block);
		
		if (block_count <= blocks_number) {
			encrypted_buffer += BLOCK_SIZE;
		}
		block_count++;
	}
	printf("total block count: %d\n", block_count);
	
}

void decrypt_file(char* encrypted_name, char* keypath) {

	FILE *encrypted_file;
	FILE *keyfile;
	errno_t encrypted_error;
	errno_t key_error;
	long int filesize = get_file_size(encrypted_name);

	printf("File %s size: %d\n", encrypted_name, filesize);

	encrypted_error = fopen_s(&encrypted_file, encrypted_name, "rb");
	if (encrypted_error != 0) {
		fprintf(stderr, "Problem with opening enrypted file");
		exit(1);
	}

	key_error = fopen_s(&keyfile, keypath, "rb");
	if (key_error != 0) {
		fprintf(stderr, "Problem with opening key file");
		exit(1);
	}

	unsigned char key_buffer[KEY_SIZE];
	size_t read_result = fread(key_buffer, 1, KEY_SIZE, keyfile);
	if (read_result != KEY_SIZE) {
		printf("Incomplete key\n");
		fclose(encrypted_file);
		fclose(keyfile);
		exit(1);
	}
	else {
		printf("key read successfully\n");
	}
	
	if (filesize != -1) {
		unsigned char *encrypted_buffer;
		int blocks_number = (filesize - KEY_SIZE) / BLOCK_SIZE;
		encrypted_buffer = malloc(filesize);
		if (encrypted_buffer != NULL) {
			fread(encrypted_buffer, filesize, 1, encrypted_file);
			fclose(encrypted_file);
			encrypted_file = NULL;

			printf("decrypting file %s\n", encrypted_name);
			decrypt(encrypted_buffer, key_buffer, blocks_number);
			
			int padding = remove_padding(encrypted_buffer + KEY_SIZE + (blocks_number - 1) * BLOCK_SIZE);
			int final_size = blocks_number * BLOCK_SIZE - padding;

			char filename_buffer[80];
			memset(filename_buffer, '\0', 80);
			int len = strlen(encrypted_name);
			strcpy(filename_buffer, encrypted_name, len);

			get_new_filename(filename_buffer, DECRYPTED_FILE);
			save_result(filename_buffer, encrypted_buffer + KEY_SIZE, final_size);
			free(encrypted_buffer);

			fclose(keyfile);
			keyfile = NULL;
		}
	}
	else {
		exit(1);
	}

}
